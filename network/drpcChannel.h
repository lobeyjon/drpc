#pragma once


#include "servermethod.pb.h"
#include "clientConnection.h"
#include "drpcController.h"

namespace drpc {

class DRpcChannel : public google::protobuf::RpcChannel {
public:
    DRpcChannel(drpc::ClientConnection* _client, google::protobuf::Service* _service, google::protobuf::RpcController* _controller);
    virtual ~DRpcChannel();

    virtual void CallMethod(const google::protobuf::MethodDescriptor* method,
                            google::protobuf::RpcController* controller,
                            const google::protobuf::Message* request,
                            google::protobuf::Message* response,
                            google::protobuf::Closure* done);

    void fromRequest(const std::string& data);

    ClientConnection* client;

private:
    google::protobuf::Service* service;
    google::protobuf::RpcController* controller;
};

}