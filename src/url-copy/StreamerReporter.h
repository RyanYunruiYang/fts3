/*
 * Copyright (c) CERN 2016
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

#ifndef FTS3_STREAMERREPORTER_H
#define FTS3_STREAMERREPORTER_H

#include "LegacyReporter.h"
#include <zmq.hpp>

/// 
class StreamerReporter: public LegacyReporter {
private:
    // Producer producer;
    // UrlCopyOpts opts;
    // zmq::context_t zmqContext;
    // zmq::socket_t zmqPingSocket;
    zmq::socket_t zmqStreamerSocket;
    std::string streamerAddress;


public:
    StreamerReporter(const UrlCopyOpts &opts);

    // TODO: Modify to obtain destination by querying controller.
    void setStreamDestination(const Transfer &transfer);

    virtual void sendStreamMessage(const std::string &msg);
    virtual void sendStreamMessage(const Transfer &transfer, std::string strType);

    virtual std::string serializeStreamerMessage(std::string eventType,
                                     std::string src, std::string dst,
                                     std::string jobId, uint64_t fileId,
                                     uint64_t timestamp, uint64_t transferred, 
                                     uint64_t filesize, double throughput
                                     );
    static void deserializeStreamerMessage(std::string msg,
                                     std::string& eventType,
                                     std::string& src, std::string& dst,
                                     std::string& jobId, uint64_t& fileId,
                                     uint64_t& timestamp, uint64_t& transferred
                                     );                                

    virtual void sendTransferStart(const Transfer&, Gfal2TransferParams&);

    // virtual void sendProtocol(const Transfer&, Gfal2TransferParams&);

    virtual void sendTransferCompleted(const Transfer&, Gfal2TransferParams&);

    // virtual void sendPing(Transfer&);
};

#endif // FTS3_STREAMERREPORTER_H