#include "drpcChannel.h"

namespace drpc {

DRpcChannel::DRpcChannel(ClientConnection* _client, google::protobuf::Service* _service, google::protobuf::RpcController* _controller) {
    client=_client;
    service=_service;
    controller=_controller;
}

DRpcChannel::~DRpcChannel() {

}

void DRpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                            google::protobuf::RpcController* controller,
                            const google::protobuf::Message* request,
                            google::protobuf::Message* response,
                            google::protobuf::Closure* done) {
                                std::string type, data;
                                std::cout<<"Call Remote Method: "<<method->name()<<std::endl;
                                unsigned int method_idx=(unsigned int)method->index();
                                Utils::packUINT32(method_idx, type);
                                request->SerializeToString(&data);
                                client->sendData(type+data);
                            }

void DRpcChannel::fromRequest(const std::string& data) {
    std::string type=data.substr(0, NET_RPC_METHOD_SIZE);
    unsigned int method_idx;
    Utils::unpackUINT32(type, method_idx);
    const google::protobuf::ServiceDescriptor* sdp = service->GetDescriptor();
    const google::protobuf::MethodDescriptor* method = sdp->method(method_idx);
    google::protobuf::Message* request = service->GetRequestPrototype(method).New();
    request->ParseFromString(data.substr(NET_RPC_METHOD_SIZE, data.size()-NET_RPC_METHOD_SIZE));
    service->CallMethod(method, controller, request, nullptr, nullptr);
}

}