#pragma once

#include <string>
#include "conf.h"
#include "utils.h"
#include "defs.pb.h"
#include "rpc_controller.h"
#include "rpc_connector.h"

namespace drpc {

class Channel : public google::protobuf::RpcChannel {
public:
    Channel(const char* ip, int port);
    virtual ~Channel();

    virtual void CallMethod(const google::protobuf::MethodDescriptor* method,
                            google::protobuf::RpcController* controller,
                            const google::protobuf::Message* request,
                            google::protobuf::Message* response,
                            google::protobuf::Closure* done);

    void dispatch(const std::string& data);

    Connector *connector;
    DRpcController* controller;
};

}