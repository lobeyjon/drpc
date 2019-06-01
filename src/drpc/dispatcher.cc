#include "dispatcher.h"

namespace drpc {

DoneParams::DoneParams(
    google::protobuf::Message* _request, 
    google::protobuf::Message* _response, 
    unsigned int _hid): request(_request), response(_response), hid(_hid) {
}

DoneParams::~DoneParams() {
}

Dispatcher::Dispatcher() {
    
}


void Dispatcher::registerService(google::protobuf::Service* service) {
    services[service->GetDescriptor()->name()]=service;
}

void Dispatcher::dispatch(unsigned int hid, const std::string& data) {
    std::string rpc_header_wsize=data.substr(0, NET_RPC_HEADER_SIZE);
    unsigned int rpc_header_size;
    Utils::unpackUINT32(rpc_header_wsize, rpc_header_size);
    // std::cout<<"Header Length is "<<rpc_header_size<<std::endl;
    std::string rpc_header=data.substr(NET_RPC_HEADER_SIZE, rpc_header_size);
    RPCHeader header;
    header.ParseFromString(rpc_header);
    std::string service_name=header.service_name();
    unsigned int method_idx=header.method_idx();
    // std::cout<<"Service Name is "<<service_name<<" method_idx is "<<method_idx<<std::endl;
    // Get Service by service_idx
    if(services.find(service_name)!=services.end()) {
        google::protobuf::Service* service=services[service_name];
        const google::protobuf::ServiceDescriptor* sdp = service->GetDescriptor();
        // Get Method by method_idx
        const google::protobuf::MethodDescriptor* method = sdp->method(method_idx);
        google::protobuf::Message* request = service->GetRequestPrototype(method).New();
        google::protobuf::Message* response = service->GetResponsePrototype(method).New();
        request->ParseFromString(data.substr(NET_RPC_HEADER_SIZE+rpc_header_size, data.size()-NET_RPC_HEADER_SIZE-rpc_header_size));
        DRpcController controller;

        auto done = google::protobuf::NewCallback(
            this,
            &Dispatcher::onRespMsg,
            DoneParams(request, response, hid));
        service->CallMethod(method, &controller, request, response, done);
    }
}

void Dispatcher::onRespMsg(DoneParams params) {
    std::string data;
    if(params.response!=nullptr) {
        params.response->SerializeToString(&data);
        WriteMsgQueue::getInstance()->push(new IOEvent(-1, params.hid, data));
    }
}

Dispatcher* Dispatcher::getInstance() {
    return m_instance;
}

Dispatcher* Dispatcher::m_instance=new Dispatcher;

}