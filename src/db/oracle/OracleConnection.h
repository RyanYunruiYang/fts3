/********************************************//**
 * Copyright @ Members of the EMI Collaboration, 2010.
 * See www.eu-emi.eu for details on the copyright holders.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License. 
 * You may obtain a copy of the License at 
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0 
 * 
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 ***********************************************/

/**
 * @file OracleConnection.h
 * @brief hanlde oracle connection
 * @author Michail Salichos
 * @date 09/02/2012
 * 
 **/

#pragma once

#include <iostream>
#include <occi.h>

class OracleConnection {
public:

    OracleConnection(std::string username, std::string password, std::string connectString);

    OracleConnection() {
    }
    ~OracleConnection();

    oracle::occi::ResultSet* createResultset(oracle::occi::Statement* s);
    oracle::occi::Statement* createStatement(std::string sql);

    void destroyResultset(oracle::occi::Statement* s, oracle::occi::ResultSet* r);
    void destroyStatement(oracle::occi::Statement* s);
    void commit();
    void rollback();
    
    
    oracle::occi::Environment* getEnv(){
    	return env;
	}
    
    

private:
    oracle::occi::Environment* env;
    oracle::occi::Connection* conn;
    oracle::occi::ResultSet* r;
    oracle::occi::Statement* s;

};
