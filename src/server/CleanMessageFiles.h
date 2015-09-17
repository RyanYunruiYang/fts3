/*
 * Copyright (c) CERN 2013-2015
 *
 * Copyright (c) Members of the EMI Collaboration. 2010-2013
 *  See  http://www.eu-emi.eu/partners for details on the copyright
 *  holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#ifndef CLEANSTATELOG_H_
#define CLEANSTATELOG_H_

#include <ctime>

#include <iostream>
#include <utility>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

#include "common/Logger.h"
#include "db/generic/SingleDbInstance.h"

extern bool stopThreads;

namespace fts3 {
namespace server {

class CleanMessageFiles
{
private:
    int counter;

    void removeOldFiles(const std::string& path)
    {
        namespace fs = boost::filesystem;

        fs::recursive_directory_iterator end;

        for (fs::recursive_directory_iterator dir(path); dir != end; ++dir)
        {
            if(!fs::is_directory(*dir))
            {
                std::time_t t = fs::last_write_time( *dir ) ;
                std::time_t now = time(NULL);

                double x =  difftime (now, t);
                //clean files 15 days old
                if(x > 1296000)
                {
                    FTS3_COMMON_LOGGER_NEWLOG(INFO) << " Deleting file " << *dir
                            << " because it was created " << std::ctime( &t )
                            <<  fts3::common::commit;
                    boost::filesystem::remove(*dir);
                }
            }
        }
    }

public:

    void operator () ()
    {
        while (!stopThreads)
        {
            ++counter;

            try
            {
                // Once a day remove left over spool files
                if(counter == 86400)
                {
                    removeOldFiles("/var/lib/fts3/monitoring/");
                    removeOldFiles("/var/lib/fts3/stalled/");
                    removeOldFiles("/var/lib/fts3/status/");
                    removeOldFiles("/var/lib/fts3/logs/");

                    // Reset
                    counter = 0;
                }

                // Every hour
                if (counter % 3600 == 0)
                {
                    db::DBSingleton::instance().getDBObjectInstance()->checkSanityState();
                }
            }
            catch(std::exception& e)
            {
                FTS3_COMMON_LOGGER_NEWLOG(ERR) << "Cannot delete old files " << e.what() <<  fts3::common::commit;
            }
            catch(...)
            {
                FTS3_COMMON_LOGGER_NEWLOG(ERR) << "Cannot delete old files" <<  fts3::common::commit;
            }

            sleep(1);
        }
    }
};

} // end namespace server
} // end namespace fts3

#endif // CLEANSTATELOG_H_