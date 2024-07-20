#include "StreamerReporter.h"
#include <zmq.hpp>
#include "common/Logger.h"
#include "monitoring/msg-ifce.h"
#include "heuristics.h"

// namespace events = fts3::events;
using fts3::common::commit;

StreamerReporter::StreamerReporter(const UrlCopyOpts &opts): LegacyReporter(opts),
                                                    zmqStreamerSocket(zmqContext, ZMQ_PUSH)
{
    // std::string hostName = "127.0.0.1"; // formerly aggregator-test
    // std::string port = "5555";
    // std::string streamerAddress = std::string("tcp://") + hostName + ":" + port;
}

// TODO: Need to query a mapping controller to get the destination.
void StreamerReporter::setStreamDestination(const Transfer &transfer) {
    // Call the controller using transfer.source and transfer.destination
    // to get the streamer address. Hardcoded for now:
    // TODO: If zmqStreamerSocket is already set, and has the same reporter destination, don't set it again.

    /*
    Example Setting:
    Transfer1 (src1, dst1)
    Transfer2 (src1, dst1)
    Transfer3 (src1, dst2)

    newAddr = call controller to get streamerAddress
    if (streamerAddress != newAddr)
        close zmqStreamerSocket
        connect(newAddr)
    */

    std::string hostName = "127.0.0.1"; // formerly aggregator-test
    std::string port = "5555";
    std::string streamerAddress = std::string("tcp://") + hostName + ":" + port;    

    FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "DEV: Connecting to aggregator at " << streamerAddress << commit;
    zmqStreamerSocket.connect(streamerAddress.c_str());
    FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "DEV: Connected to aggregator" << commit;
}

void StreamerReporter::sendStreamMessage(const Transfer &transfer, std::string strType) {
    std::string msg = serializeStreamerMessage(strType, transfer.source.host, transfer.destination.host,
                                                transfer.jobId, transfer.fileId,
                                                transfer.stats.process.start, transfer.transferredBytes, 
                                                transfer.userFileSize, transfer.instantaneousThroughput);
    /*
        TODO: 
        pseudo code
        shardZmpSockwt = map[src, dst>]
        send to shardZmqSock
    */

    zmq::message_t message(msg.c_str(), msg.size());
    zmqStreamerSocket.send(message);
}

void StreamerReporter::sendStreamMessage(const std::string &msg) {

    zmq::message_t message(msg.c_str(), msg.size());
    zmqStreamerSocket.send(message);
    // TODO: free message
}


void StreamerReporter::sendTransferStart(const Transfer &transfer, Gfal2TransferParams& gfal2Params)
{
    setStreamDestination(transfer);

    LegacyReporter::sendTransferStart(transfer, gfal2Params);

    // Send the transfer start message to the aggregator
    FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "XXD: Sending streaming message to the Streamer Service" << commit;
    sendStreamMessage(transfer, "TRANSFER_START");
}




void StreamerReporter::sendTransferCompleted(const Transfer &transfer, Gfal2TransferParams &params)
{
    LegacyReporter::sendTransferCompleted(transfer, params);
    // std::string msg = serializeStreamerMessage("TRANSFER_COMPLETE", transfer.source.host, transfer.destination.host,
    //                                             transfer.jobId, transfer.fileId,
    //                                             transfer.stats.process.end, transfer.userFileSize);
    sendStreamMessage(transfer, "TRANSFER_COMPLETE");

    // TODO: Close the socket.
    // TODO: If zmqStreamerSocket is not empty, close.
}

std::string StreamerReporter::serializeStreamerMessage(std::string eventType,
                                     std::string src, std::string dst,
                                     std::string jobId, uint64_t fileId,
                                     uint64_t timestamp, uint64_t transferred,
                                     uint64_t userFileSize,
                                     double instantaneousThroughput
                                     ) {
    return eventType + '\t'
         + src + '\t' + dst + '\t'
         + jobId + ':' + std::to_string(fileId) + '\t'
         + std::to_string(timestamp) + '\t' + std::to_string(transferred) + '\t' + std::to_string(userFileSize)
         + '\t' + std::to_string(instantaneousThroughput);
}