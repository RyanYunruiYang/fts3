/*
 * Copyright (c) CERN 2013-2016
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

#include "MySqlOptimizerDataSource.h"
#include "db/generic/SingleDbInstance.h"
#include "IntegratedOptimizerDataSource.h"

OptimizerDataSource *MySqlAPI::getOptimizerDataSource()
{
    return new MySqlOptimizerDataSource(connectionPool);
}

// MySqlOptimizerDataSource *MySqlAPI::getMySqlOptimizerDataSource()
// {
//     return new MySqlOptimizerDataSource(connectionPool);
// }


// // Overloaded Version of getOptimizerDataSource
// static OptimizerDataSource *IntegratedOptimizerDataSource::getOptimizerDataSource(StreamerDataSource *dataSource) {
//     OptimizerDataSource *mySqlODS = db::DBSingleton::instance().getDBObjectInstance()->getOptimizerDataSource();
//     return new IntegratedOptimizerDataSource(mySqlODS, dataSource);
// }