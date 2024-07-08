#ifndef StreamerService_H_
#define StreamerService_H_

#include <string>
#include <zmq.hpp>
#include "../BaseService.h"
#include "StreamerDataSource.h"

namespace fts3 {
namespace server {

class StreamerService: public BaseService
{
private:
    zmq::context_t zmqContext;
    zmq::socket_t zmqAggSocket;
    const std::string host_name = "*";
    const int portnumber = 5555;

    StreamerDataSource *data;
public:
    StreamerService(StreamerDataSource* s); // need to give it a dataSource
    virtual void runService();
    StreamerDataSource *getStreamerDataSource() {return data;};
};

} // end namespace server
} // end namespace fts3

#endif // StreamerService_H_