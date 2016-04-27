-- Set of commands to run when a user different than the owner of the schema
-- is used to access the database.
--
-- SET HEA OFF
-- SET FEED OFF
-- SET NEWP NONE

----------------------
-- GRANT PERMISIONS --
----------------------
-- As the schema owner:
-- SELECT 'GRANT SELECT, INSERT, DELETE, UPDATE ON ' || table_name || ' TO fts3_devel_w;'  FROM user_tables;
-- SELECT 'GRANT SELECT ON ' || sequence_name || ' TO fts3_devel_w;' FROM user_sequences;
--
GRANT SELECT, INSERT, DELETE, UPDATE ON T_SERVER_SANITY TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_SERVER_CONFIG TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_OPTIMIZE_MODE TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_OPTIMIZE TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_OPTIMIZER_EVOLUTION TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_CONFIG_AUDIT TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_DEBUG TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_CREDENTIAL_CACHE TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_CREDENTIAL TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_CREDENTIAL_VERS TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_SE TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_SE_ACL TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_GROUP_MEMBERS TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_LINK_CONFIG TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_SHARE_CONFIG TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_ACTIVITY_SHARE_CONFIG TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_BAD_SES TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_BAD_DNS TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_SE_PAIR_ACL TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_VO_ACL TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_JOB TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_FILE TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_FILE_RETRY_ERRORS TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_FILE_SHARE_CONFIG TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_STAGE_REQ TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_HOSTS TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_OPTIMIZE_ACTIVE TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_SCHEMA_VERS TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_FILE_BACKUP TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_JOB_BACKUP TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_DM TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_DM_BACKUP TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_PROFILING_INFO TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_PROFILING_SNAPSHOT TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_TURL TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_OPTIMIZE_STREAMS TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_CLOUDSTORAGE TO fts3_devel_w;
GRANT SELECT, INSERT, DELETE, UPDATE ON T_CLOUDSTORAGEUSER TO fts3_devel_w;

GRANT SELECT ON FILE_FILE_ID_SEQ TO fts3_devel_w;
GRANT SELECT ON SE_ID_INFO_SEQ TO fts3_devel_w;
GRANT SELECT ON T_OPTIMIZE_AUTO_NUMBER_SEQ TO fts3_devel_w;

---------------------
-- CREATE SYNONYMS --
---------------------
-- As the schema owner:
-- SELECT 'CREATE SYNONYM ' || table_name || ' FOR fts3_devel.' || table_name || ';'  FROM user_tables;
-- SELECT 'CREATE SYNONYM ' || sequence_name || ' FOR fts3_devel.' || sequence_name || ';' FROM user_sequences;
--
-- As the fts3 user:
--
CREATE SYNONYM T_DM FOR fts3_devel.T_DM;
CREATE SYNONYM T_DM_BACKUP FOR fts3_devel.T_DM_BACKUP;
CREATE SYNONYM T_SERVER_SANITY FOR fts3_devel.T_SERVER_SANITY;
CREATE SYNONYM T_SERVER_CONFIG FOR fts3_devel.T_SERVER_CONFIG;
CREATE SYNONYM T_OPTIMIZE_MODE FOR fts3_devel.T_OPTIMIZE_MODE;
CREATE SYNONYM T_OPTIMIZE FOR fts3_devel.T_OPTIMIZE;
CREATE SYNONYM T_OPTIMIZER_EVOLUTION FOR fts3_devel.T_OPTIMIZER_EVOLUTION;
CREATE SYNONYM T_CONFIG_AUDIT FOR fts3_devel.T_CONFIG_AUDIT;
CREATE SYNONYM T_DEBUG FOR fts3_devel.T_DEBUG;
CREATE SYNONYM T_CREDENTIAL_CACHE FOR fts3_devel.T_CREDENTIAL_CACHE;
CREATE SYNONYM T_CREDENTIAL FOR fts3_devel.T_CREDENTIAL;
CREATE SYNONYM T_CREDENTIAL_VERS FOR fts3_devel.T_CREDENTIAL_VERS;
CREATE SYNONYM T_SE FOR fts3_devel.T_SE;
CREATE SYNONYM T_SE_ACL FOR fts3_devel.T_SE_ACL;
CREATE SYNONYM T_GROUP_MEMBERS FOR fts3_devel.T_GROUP_MEMBERS;
CREATE SYNONYM T_LINK_CONFIG FOR fts3_devel.T_LINK_CONFIG;
CREATE SYNONYM T_SHARE_CONFIG FOR fts3_devel.T_SHARE_CONFIG;
CREATE SYNONYM T_ACTIVITY_SHARE_CONFIG FOR fts3_devel.T_ACTIVITY_SHARE_CONFIG;
CREATE SYNONYM T_BAD_SES FOR fts3_devel.T_BAD_SES;
CREATE SYNONYM T_BAD_DNS FOR fts3_devel.T_BAD_DNS;
CREATE SYNONYM T_SE_PAIR_ACL FOR fts3_devel.T_SE_PAIR_ACL;
CREATE SYNONYM T_VO_ACL FOR fts3_devel.T_VO_ACL;
CREATE SYNONYM T_JOB FOR fts3_devel.T_JOB;
CREATE SYNONYM T_FILE FOR fts3_devel.T_FILE;
CREATE SYNONYM T_FILE_RETRY_ERRORS FOR fts3_devel.T_FILE_RETRY_ERRORS;
CREATE SYNONYM T_FILE_SHARE_CONFIG FOR fts3_devel.T_FILE_SHARE_CONFIG;
CREATE SYNONYM T_STAGE_REQ FOR fts3_devel.T_STAGE_REQ;
CREATE SYNONYM T_HOSTS FOR fts3_devel.T_HOSTS;
CREATE SYNONYM T_OPTIMIZE_ACTIVE FOR fts3_devel.T_OPTIMIZE_ACTIVE;
CREATE SYNONYM T_SCHEMA_VERS FOR fts3_devel.T_SCHEMA_VERS;
CREATE SYNONYM T_FILE_BACKUP FOR fts3_devel.T_FILE_BACKUP;
CREATE SYNONYM T_JOB_BACKUP FOR fts3_devel.T_JOB_BACKUP;
CREATE SYNONYM T_PROFILING_INFO FOR fts3_devel.T_PROFILING_INFO;
CREATE SYNONYM T_PROFILING_SNAPSHOT FOR fts3_devel.T_PROFILING_SNAPSHOT;
CREATE SYNONYM T_TURL FOR fts3_devel.T_TURL;
CREATE SYNONYM T_OPTIMIZE_STREAMS FOR fts3_devel.T_OPTIMIZE_STREAMS;
CREATE SYNONYM T_CLOUDSTORAGE FOR fts3_devel.T_CLOUDSTORAGE;
CREATE SYNONYM T_CLOUDSTORAGEUSER FOR fts3_devel.T_CLOUDSTORAGEUSER;

CREATE SYNONYM DM_FILE_ID_SEQ FOR fts3_devel.DM_FILE_ID_SEQ;
CREATE SYNONYM FILE_FILE_ID_SEQ FOR fts3_devel.FILE_FILE_ID_SEQ;
CREATE SYNONYM SE_ID_INFO_SEQ FOR fts3_devel.SE_ID_INFO_SEQ;
CREATE SYNONYM T_OPTIMIZE_AUTO_NUMBER_SEQ FOR fts3_devel.T_OPTIMIZE_AUTO_NUMBER_SEQ;
