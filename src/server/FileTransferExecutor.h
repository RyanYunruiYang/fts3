/*
 * TransferHandler.h
 *
 *  Created on: Aug 9, 2013
 *      Author: simonm
 */

#ifndef TRANSFERHANDLER_H_
#define TRANSFERHANDLER_H_

#include <boost/thread.hpp>
#include <boost/any.hpp>

#include "db/generic/SingleDbInstance.h"

#include "TransferFileHandler.h"
#include "site_name.h"

#include <set>
#include <string>

#include <boost/scoped_ptr.hpp>

namespace fts3
{

namespace server
{

using namespace db;
using namespace std;
using namespace boost;
using namespace fts3::common;

/**
 * FileTransferExecutor is the worker class that executes
 * FileTransfers one by one in a separate thread.
 *
 * The FileTransfers are added to a thread-safe queue, which is processed by the worker.
 * In order to let the worker thread know that all the date were pushed into the queue the
 * 'noMoreData()' method should be used, in order to wait for the thread to finish processing
 * FileTransfers the 'join()' method should be used (before calling join, noMoreData should be called)
 */
class FileTransferExecutor
{

public:

    /**
     * Constructor.
     *
     * @param tfh - the TransferFileHandler reference
     * @param optimize - flag stating if optimization is ON
     * @param monitoringMsg - is true if monitoring messages are in use
     * @param infosys - information system host
     * @param ftsHostName - hostname of the machine hosting FTS3
     */
    FileTransferExecutor(TransferFiles& tf, TransferFileHandler& tfh, bool monitoringMsg, string infosys, string ftsHostName, string proxy, std::string logDir);

    /**
     * Destructor.
     */
    virtual ~FileTransferExecutor();

    /**
     * spawns a url_copy
     *
     * @return 0 if the file was not scheduled, 1 otherwise
     */
    virtual void run(boost::any &);

private:

    /// pairs that were already checked and were not scheduled
    set< pair<string, string> > notScheduled;

    /// variables from process_service_handler
    TransferFiles tf;
    TransferFileHandler const & tfh;
    bool monitoringMsg;
    string infosys;
    string ftsHostName;
    SiteName siteResolver;
    string proxy;
    std::string logsDir;

    // DB interface
    GenericDbIfce* db;
};

} /* namespace server */
} /* namespace fts3 */
#endif /* TRANSFERHANDLER_H_ */
