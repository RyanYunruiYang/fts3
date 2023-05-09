#!/usr/bin/env python3
#
#   Copyright notice:
#   Copyright CERN, 2016.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

import os
import sys
import logging
import subprocess

from distutils.util import strtobool
from optparse import OptionParser
from pkg_resources import parse_version
from sqlalchemy import create_engine
from sqlalchemy.exc import ProgrammingError
from tempfile import NamedTemporaryFile
from urllib.parse import urlparse

from fts3.util.config import fts3_config_load

log = logging.getLogger(__name__)
ASSUME_YES = False


def infer_sql_location(config):
    """
    Depending on the database configuration, guess where the sql scripts are
    located
    :param config: FTS3 config file parsed
    :return: The inferred SQL script location
    """
    base = '/usr/share'
    if config['fts3.DbType'] not in ['mysql']:
        raise NotImplementedError('Database type %s not supported', config['fts3.DbType'])
    return os.path.join(base, 'fts-%s' % config['fts3.DbType'])


def connect_database(config):
    """
    Connect to the database
    :param config: FTS3 config file parsed
    :return: A database connection
    """
    log.debug('Connecting to the database')
    engine = create_engine(config['sqlalchemy.url'])
    conn = engine.connect()
    log.debug('Connected')
    return conn


def get_schema_version(conn):
    """
    Get the schema version
    :param conn: Database connection
    :return: Schema version, None if not found
    """
    try:
        result = conn.execute(
            'SELECT major, minor, patch FROM t_schema_vers ORDER BY major DESC, minor DESC, patch DESC')
        row = result.fetchone()
        if row is None:
            raise Exception('t_schema_vers exits but is empty!')
        return parse_version("%(major)d.%(minor)d.%(patch)d" % row)
    except ProgrammingError:
        return None


def get_running_services(conn):
    """
    Return a list of tuples (host, service) of services that are still alive
    :param conn: Database connection
    :return: List of tuples
    """
    result = conn.execute(
        'SELECT hostname, service_name FROM t_hosts WHERE beat >= UTC_TIMESTAMP() - INTERVAL 2 MINUTE'
    )
    services = []
    for row in result.fetchall():
        services.append((row['hostname'], row['service_name']))
    return services


def ask_confirmation(question):
    """
    Ask a yes/no question
    :param question: The question
    :return: True if accepted, False if not
    """
    if ASSUME_YES:
        return True
    print("%s [Y/N]" % question)
    return strtobool(input().lower())


def get_full_schema_path(sql_location):
    """
    Find the full schema with the higher version under sql_location
    :param sql_location: Location of the SQL scripts
    :return: The path to the full schema script with the highest version
    """
    full_schemas = []
    for sql in os.listdir(sql_location):
        if sql.endswith('.sql') and sql.startswith('fts-schema'):
            path = os.path.join(sql_location, sql)
            version = parse_version(sql[:-4].split('-')[2])
            log.debug('Found schema %s with version %s', path, version)
            full_schemas.append((version, path))
    if len(full_schemas) == 0:
        raise RuntimeError('Could not find a valid schema')
    highest = sorted(full_schemas, reverse=True)[0]
    log.debug('Highest schema: %s' % highest[1])
    return highest[1]


def get_upgrade_scripts(current_version, sql_location):
    """
    Look for upgrade scripts
    :param current_version: Version running now
    :param sql_location: Where to look for the SQL scripts
    :return: A list with upgrade scripts that must run
    """
    upgrades = []
    for sql in os.listdir(sql_location):
        if sql.endswith('.sql') and sql.startswith('fts-diff'):
            path = os.path.join(sql_location, sql)
            version = parse_version(sql[:-4].split('-')[2])
            log.debug('Found schema %s with version %s', path, version)
            if version > current_version:
                log.debug('Mark to upgrade')
                upgrades.append((version, path))
    return [path for (version, path) in sorted(upgrades)]


def run_sql_script_mysql(config, sql):
    """
    Run a SQL script
    :param config: FTS3 config file parsed
    :param sql: The SQL script
    """
    parsed = urlparse('mysql://' + config['fts3.DbConnectString'])

    creds = NamedTemporaryFile(delete=True, mode='w')
    print("""
[client]
user=%(fts3.DbUserName)s
password=%(fts3.DbPassword)s
""" % config, file=creds)
    creds.flush()

    cmd = [
        'mysql',
        '--defaults-extra-file=%s' % creds.name,
        '-h', parsed.hostname,
        '-v'
    ]
    if parsed.port:
        cmd.extend(['-P', str(parsed.port)])
    cmd.append(parsed.path[1:])

    log.debug('Running %s < %s', ' '.join(cmd), sql)

    sql_fd = open(sql)
    ret = subprocess.call(cmd, stdout=sys.stdout, stderr=sys.stderr, stdin=sql_fd)
    if ret != 0:
        raise Exception('Failed to run mysql (%d)' % ret)


def run_sql_script(config, sql):
    """
    Run a SQL script
    :param config: FTS3 config file parsed
    :param sql: The SQL script
    """
    log.info('Running %s' % sql)
    run_sql_script_mysql(config, sql)


def populate_schema(config, sql_location):
    """
    Create the schema from scratch
    :param config: FTS3 config file parsed
    :param sql_location: Location of the SQL scripts
    """
    log.warning('The schema does not seem to be created')
    if not ask_confirmation('Do you want to create the schema now?'):
        log.error('Abort')
        return

    schema = get_full_schema_path(sql_location)
    run_sql_script(config, schema)

    current_version = get_schema_version(connect_database(config))
    log.info('Running upgrade scripts')
    upgrade_schema(connect_database(config), config, current_version, sql_location)


def upgrade_schema(conn, config, current_version, sql_location):
    """
    Run the upgrade scripts if needed
    :param conn: Database connection
    :param config: FTS3 config file parsed
    :param current_version: Version present in the database
    :param sql_location: Location of the SQL scripts
    """
    log.info('Current schema version is %s', current_version)

    running_services = get_running_services(conn)
    if len(running_services) > 0:
        log.warning('There are services still running')
        for service in running_services:
            log.warning("%s: %s" % service)
        if not ask_confirmation('Do you want to continue anyway?'):
            log.error('Abort')
            return

    upgrade_scripts = get_upgrade_scripts(current_version, sql_location)
    if len(upgrade_scripts) == 0:
        log.info('No upgrades pending')
        return

    log.warning('%d upgrade scripts found' % len(upgrade_scripts))
    for sql in upgrade_scripts:
        log.warning(sql)

    if not ask_confirmation('Do you want to run the upgrade scripts?'):
        log.error('Abort')
        return

    for sql in upgrade_scripts:
        run_sql_script(config, sql)


def prepare_schema(config, sql_location):
    """
    Run the upgrade scripts
    :param config: FTS3 config file parsed
    :param sql_location: SQL scripts location
    :return:
    """
    db_type = config['fts3.DbType']
    log.info('Database type: %s' % db_type)
    log.info('SQL Scripts location: %s' % sql_location)
    log.info('Database: %s' % config['fts3.DbConnectString'])
    log.info('User: %s' % config['fts3.DbUserName'])

    conn = connect_database(config)

    current_version = get_schema_version(conn)
    if current_version is None:
        populate_schema(config, sql_location)
    else:
        upgrade_schema(conn, config, current_version, sql_location)


if __name__ == '__main__':
    optparser = OptionParser(description='Run database upgrade scripts')
    optparser.add_option('-f', '--config-file', default='/etc/fts3/fts3config', help='Configuration file')
    optparser.add_option('-v', '--verbose', default=False, action='store_true', help='Verbose mode')
    optparser.add_option('-d', '--sql-location', default=None, help='SQL scripts location')
    optparser.add_option('-y', '--assume-yes', default=False, action='store_true', help='Automatically answer yes to questions.')
    opts, args = optparser.parse_args()
    if len(args) > 0:
        optparser.error('No arguments are expected')
    if opts.assume_yes:
        global ASSUME_YES
        ASSUME_YES = True

    log_handler = logging.StreamHandler(sys.stderr)
    log_handler.setFormatter(logging.Formatter('[%(levelname)7s] %(message)s'))
    if opts.verbose:
        log_handler.setLevel(logging.DEBUG)
    else:
        log_handler.setLevel(logging.INFO)
    log.addHandler(log_handler)
    log.setLevel(logging.DEBUG)

    config = fts3_config_load(opts.config_file)

    if opts.sql_location is None:
        opts.sql_location = infer_sql_location(config)

    prepare_schema(config, opts.sql_location)
