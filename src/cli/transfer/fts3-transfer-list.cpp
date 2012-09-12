/*
 *	Copyright notice:
 *	Copyright © Members of the EMI Collaboration, 2010.
 *
 *	See www.eu-emi.eu for details on the copyright holders
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 * fts3-transfer-list.cpp
 *
 *  Created on: Mar 1, 2012
 *      Author: Michal Simon
 */


#include "GSoapContextAdapter.h"
#include "ui/ListTransferCli.h"

#include "common/JobStatusHandler.h"

#include <memory>

using namespace std;
using namespace fts3::cli;
using namespace fts3::common;


/**
 * This is the entry point for the fts3-transfer-list command line tool.
 */
int main(int ac, char* av[]) {

	try {
		// create and initialize the command line utility
		auto_ptr<ListTransferCli> cli (
				getCli<ListTransferCli>(ac, av)
			);

		// validate command line options, and return respective gsoap context
		optional<GSoapContextAdapter&> opt = cli->validate();
		if (!opt.is_initialized()) return 0;
		GSoapContextAdapter& ctx = opt.get();

		if (!cli->checkIfFeaturesSupported()) {
			return 0;
		}

		vector<string> array = cli->getStatusArray();

		vector<fts3::cli::JobStatus> statuses;

		if (ctx.isUserVoRestrictListingSupported()) {

			statuses = ctx.listRequests2(array, cli->getUserDn(), cli->getVoName());

		} else {

			statuses = ctx.listRequests(array);
		}

		vector<fts3::cli::JobStatus>::iterator it;
		for (it = statuses.begin(); it < statuses.end(); it++) {
			if (cli->isVerbose()) {

				JobStatusHandler::printJobStatus(*it);

			} else {
				cout << it->jobId << "\t" << it->jobStatus << endl;
			}
		}

    } catch(std::exception& e) {
        cerr << "error: " << e.what() << "\n";
        return 1;
    } catch(string& ex) {
    	cout << ex << endl;
    	return 1;
    } catch(...) {
        cerr << "Exception of unknown type!\n";
        return 1;
    }

	return 0;
}
