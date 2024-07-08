#include "StreamerService.h"
// #include <boost/filesystem.hpp>
#include "config/ServerConfig.h"
// #include "db/generic/SingleDbInstance.h"
// #include "msg-bus/consumer.h"
#include "common/Logger.h"

// namespace fs = boost::filesystem;
using fts3::config::ServerConfig;


namespace fts3 {
namespace server {


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
                // data.count++;
                // FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "DEV: message " << data.count << " " << msg_str << fts3::common::commit;  
                FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "DEV: message #" << counter << " contains: " << msg_str << fts3::common::commit; 
            }

            // message.get()
            /*
            if msg is a performance marker
            
            */      
            // // Every hour
            // if (checkSanityState >0 && counter % checkSanityState == 0) {
            //     db::DBSingleton::instance().getDBObjectInstance()->checkSanityState();
            // }

            // // Every 10 minutes
            // if (purgeMsgDirs > 0 && counter % purgeMsgDirs == 0) {
            //     Consumer consumer(msgDir);
            //     consumer.purgeAll();
            // }

            // //Every 10 minutes (default)
            // if (multihopSanitySate >0 && counter % multihopSanitySate == 0) {
            //     db::DBSingleton::instance().getDBObjectInstance()->multihopSanitySate();
            // }
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