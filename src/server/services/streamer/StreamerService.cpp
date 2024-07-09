#include "StreamerService.h"
// #include <boost/filesystem.hpp>
#include "config/ServerConfig.h"
// #include "db/generic/SingleDbInstance.h"
// #include "msg-bus/consumer.h"
#include "common/Logger.h"
//#include "url-copy/LegacyReporter.h"

// namespace fs = boost::filesystem;
using fts3::config::ServerConfig;


namespace fts3 {
namespace server {

void deserializeStreamerMessage(std::string msg,
                                     std::string& eventType,
                                     std::string& src, std::string& dst,
                                     std::string& jobId, uint64_t& fileId,
                                     uint64_t& timestamp, uint64_t& transferred
                                     ) {
    std::istringstream iss(msg);
    std::string token;
    std::getline(iss, eventType, '\t');
    std::getline(iss, src, '\t');
    std::getline(iss, dst, '\t');
    std::getline(iss, token, '\t');
    size_t colonPos = token.find(':');
    jobId = token.substr(0, colonPos);
    fileId = std::stoull(token.substr(colonPos + 1));
    std::getline(iss, token, '\t');
    timestamp = std::stoull(token);
    std::getline(iss, token, '\t');
    transferred = std::stoull(token);
}

StreamerService::StreamerService(StreamerDataSource* s): BaseService("StreamerService"), zmqContext(1), 
                                        zmqAggSocket(zmqContext, ZMQ_PULL), data(s)
{
    std::string bind_address = "tcp://" + host_name + ":" + std::to_string(portnumber);
    FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << bind_address << fts3::common::commit;
    
    zmqAggSocket.bind(bind_address);
    FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "bind successful" << fts3::common::commit;
}


void StreamerService::runService()
{
    int counter = 0;

    // std::string msgDir = ServerConfig::instance().get<std::string>("MessagingDirectory");
    // int purgeMsgDirs = ServerConfig::instance().get<int>("PurgeMessagingDirectoryInterval");
    // int checkSanityState = ServerConfig::instance().get<int>("CheckSanityStateInterval");
    // int multihopSanitySate = ServerConfig::instance().get<int>("MultihopSanityStateInterval");

    while (!boost::this_thread::interruption_requested())
    {
        ++counter;

        try
        {
            // FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "Blocking until a message is received..." << fts3::common::commit;
            zmq::message_t message;
            std::optional<unsigned long> rc = zmqAggSocket.recv(&message, ZMQ_NOBLOCK); // zmqAggSocket.recv(message, zmq::recv_flags::none);
            if (rc == -1) {
                FTS3_COMMON_LOGGER_NEWLOG(ERR) << "DEV: Error receiving message" << fts3::common::commit;
            }
            // else if (rc == 0) {
            //     FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "DEV: No message received" << fts3::common::commit;
            // }
            if (rc > 0) {
                std::string msg_str(static_cast<char*>(message.data()), message.size());
                std::string eventType, src, dst, jobId;
                uint64_t fileId, timestamp, transferred;
                deserializeStreamerMessage(msg_str, eventType, src, dst, 
                                                        jobId, fileId, timestamp, transferred);
                data->numPM++;
                // FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "DEV: message " << data.numPM << " " << msg_str << fts3::common::commit;  
                FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "XXX: message #" << counter << " contains:\n" << msg_str << fts3::common::commit; 
                FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "\t\tparsed timestamp: " << timestamp << fts3::common::commit; 
            }

        }
        catch(...)
        {
            FTS3_COMMON_LOGGER_NEWLOG(ERR) << "Aggregator Exception:" << fts3::common::commit;
        }

        boost::this_thread::sleep(boost::posix_time::seconds(1));
    }
}


} // end namespace server
} // end namespace fts3