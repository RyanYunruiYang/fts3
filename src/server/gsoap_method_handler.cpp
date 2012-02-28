/* Copyright @ Members of the EMI Collaboration, 2010.
See www.eu-emi.eu for details on the copyright holders.

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 

    http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
See the License for the specific language governing permissions and 
limitations under the License. */

#include "common/logger.h"
#include "common/error.h"
#include "gsoap_method_handler.h"
#include "ftsFileTransferSoapBindingService.h"
 
#ifdef FTS3_COMPILE_WITH_UNITTEST
    #include "unittest/testsuite.h"
#endif // FTS3_COMPILE_WITH_UNITTESTS

FTS3_SERVER_NAMESPACE_START

GSoapMethodHandler::GSoapMethodHandler(Pointer<FileTransferSoapBindingService>::Shared service)
    : _service(service)
{}

void GSoapMethodHandler::handle()
{
    assert (_service.get());
    FTS3_COMMON_LOGGER_NEWLOG (INFO) << "Serving request started... " << commit;
    _service->serve();
    FTS3_COMMON_LOGGER_NEWLOG (INFO) << "Serving request finished... " << commit;
}

#ifdef FTS3_COMPILE_WITH_UNITTEST

#endif // FTS3_COMPILE_WITH_UNITTESTS

FTS3_SERVER_NAMESPACE_END

