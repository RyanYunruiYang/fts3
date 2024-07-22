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
                                     uint64_t& timestamp, uint64_t& transferred, 
                                     double& instantaneousThroughput
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
    std::getline(iss, token, '\t');
    instantaneousThroughput = std::stod(token);
}

// This function parses a string message and extracts information from the perf marker.
// The parsed information includes:
// - eventType: The type of event (TRANSFER_START, TRANSFER_CALLBACK_PM, TRANSFER_COMPLETE, or UNDEFINED)
// - src: The source of the transfer
// - dst: The destination of the transfer
// - jobId: The ID of the job
// - fileId: The ID of the file
// - timestamp: The timestamp of the event
// - transferred: The amount of data transferred
void deserializeStreamerMessage(std::string msg, StreamerPerfMarker& pm) {
    
    std::istringstream iss(msg);
    std::string token, s_eventType;
    std::getline(iss, s_eventType, '\t');
    if (s_eventType == "TRANSFER_START")
        pm.eventType = TRANSFER_START;
    else if (s_eventType == "TRANSFER_CALLBACK_PM")
        pm.eventType = TRANSFER_CALLBACK_PM;
    else if (s_eventType == "TRANSFER_COMPLETE")
        pm.eventType = TRANSFER_COMPLETE;
    else {
        pm.eventType = UNDEFINED;
        // Code for other event types
    }

    std::getline(iss, pm.src, '\t');
    std::getline(iss, pm.dst, '\t');
    std::getline(iss, token, '\t');
    size_t colonPos = token.find(':');
    pm.jobId = token.substr(0, colonPos);
    pm.fileId = std::stoull(token.substr(colonPos + 1));
    std::getline(iss, token, '\t');
    pm.timestamp = std::stoull(token);
    std::getline(iss, token, '\t');
    pm.transferred = std::stoull(token);
    std::getline(iss, token, '\t');
    pm.userFileSize = std::stoull(token);
    std::getline(iss, token, '\t');
    pm.instantaneousThroughput = std::stod(token);
}


StreamerService::StreamerService(StreamerDataSource* s): BaseService("StreamerService"), zmqContext(1), 
                                        zmqStreamerSocket(zmqContext, ZMQ_PULL), data(s)
{
    std::string bind_address = "tcp://" + host_name + ":" + std::to_string(portnumber);
    FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << bind_address << fts3::common::commit;
    
    zmqStreamerSocket.bind(bind_address);
    FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "bind successful" << fts3::common::commit;
}


void processTransferStart(const StreamerPerfMarker& pm, StreamerDataSource* data,
                          const Pair& pair)
{
    // Update m_sd
    --data->m_sd[pair].submittedCount;
    ++data->m_sd[pair].activeCount;
    data->s_activePairs.insert(pair);
    FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "AAA:" << pair << " start active count " 
        << data->m_sd[pair].activeCount << fts3::common::commit; 

    // Code for handling values with units involving Bytes
    // Update m_sdf
    data->m_sdf[pair][pm.fileId].lastTransferredBytes = pm.transferred;
    data->m_sdf[pair][pm.fileId].lastTimestamp = pm.timestamp;
    
    data->m_sds[pair].processNewTransfer(pm.timestamp, pm.userFileSize);   
}

void processTransferCallback(const StreamerPerfMarker& pm, StreamerDataSource* data,
                             const Pair& pair)
{
    auto previousTransferred = data->m_sdf[pair][pm.fileId].lastTransferredBytes;
    auto prevTimestamp = data->m_sdf[pair][pm.fileId].lastTimestamp;

    uint64_t transferDelta = pm.transferred - previousTransferred;
    uint64_t timeDelta = pm.timestamp - prevTimestamp;

    // Code for handling values with units involving Bytes
    data->pairToCyclicBuffer[pair].updateTransferred(prevTimestamp, pm.timestamp, transferDelta);


    // Update m_sdf
    data->m_sdf[pair][pm.fileId].lastTransferredBytes = pm.transferred;
    data->m_sdf[pair][pm.fileId].lastTimestamp = pm.timestamp;

    FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "AAE: transferred: " << pm.transferred
                                     << ", transferDelta: " << transferDelta
                                     << ", out of " << pm.userFileSize 
                                     << ", in " << timeDelta
                                     << ", inst tput: " << pm.instantaneousThroughput << fts3::common::commit;
}

void processTransferComplete(const StreamerPerfMarker& pm, StreamerDataSource* data,
                             const Pair& pair)
{
    if (--data->m_sd[pair].activeCount == 0) { // remove pair from activePairs
        data->s_activePairs.erase(pair);
    }
    ++data->m_sd[pair].finishedCount;

    // Code for handling values with units involving Bytes
    data->pairToCyclicBuffer[pair].updateTransferred(data->m_sdf[pair][pm.fileId].lastTimestamp, pm.timestamp, pm.transferred - data->m_sdf[pair][pm.fileId].lastTransferredBytes);

    // Clean up m_sdf
    data->m_sdf[pair].erase(pm.fileId);
                             
    FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "AAA:" << pair << " complete active count " 
        << data->m_sd[pair].activeCount << fts3::common::commit; 
}

void StreamerService::runService()
{
    int counter = 0;
    // std::string msgDir = ServerConfig::instance().get<std::string>("MessagingDirectory");
    while (!boost::this_thread::interruption_requested())
    {
        ++counter;
        try
        {
            // FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "Blocking until a message is received..." << fts3::common::commit;
            zmq::message_t message;
            int rc = zmqStreamerSocket.recv(&message);
            if (rc == -1) {
                FTS3_COMMON_LOGGER_NEWLOG(ERR) << "DEV: Error receiving message" << fts3::common::commit;
            }
            if (rc > 0) {
                std::string msg_str(static_cast<char*>(message.data()), message.size());
                StreamerPerfMarker pm;
                deserializeStreamerMessage(msg_str, pm); // pm contains: eventType, src, dst, jobId, fileId, timestamp, transferred);
                data->numPM++;
                FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "XXX: message #" << counter << " contains:\n" << msg_str << fts3::common::commit; 
                FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "AAB: parsed timestamp: " << pm.timestamp << fts3::common::commit; 

                Pair pair = Pair(pm.src, pm.dst);
                // StreamerFileState* fileState = data->m_sdf[pair][pm.fileId];
                // StreamerPairState* curEpochState = data->pairToCyclicBuffer[pair].getStreamerState(pm.timestamp);

                uint64_t transferDelta, timeDelta;
                switch (pm.eventType) {
                    case TRANSFER_START:
                        processTransferStart(pm, data, pair);
                        break;
                    case TRANSFER_CALLBACK_PM:
                        processTransferCallback(pm, data, pair);
                        break;
                    case TRANSFER_COMPLETE:
                        processTransferComplete(pm, data, pair);
                        break;
                    case UNDEFINED:
                        FTS3_COMMON_LOGGER_NEWLOG(ERR) << "AAA: undefined eventType" << fts3::common::commit; 
                        break;
                    default:
                        FTS3_COMMON_LOGGER_NEWLOG(ERR) << "AAA: wrong eventType" << fts3::common::commit;
                        break;
                }
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