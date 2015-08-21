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

#include <iostream>
#include <ctime>

class Se
{

public:
    Se() {}
    ~Se() {}

    std::string ENDPOINT;
    std::string SE_TYPE;
    std::string SITE;
    std::string NAME;
    std::string STATE;
    std::string VERSION;
    std::string HOST;
    std::string SE_TRANSFER_TYPE;
    std::string SE_TRANSFER_PROTOCOL;
    std::string SE_CONTROL_PROTOCOL;
    std::string GOCDB_ID;
};
