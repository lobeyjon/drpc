#include "rpc_channel.h"

namespace drpc {

Channel::Channel(const char* ip, int port) {
    connector = new Connector(ip, port);
    controller=new DRpcController();
}

Channel::~Channel() {
    delete connector;
    connector=nullptr;
    delete controller;
    controller=nullptr;
}

void Channel::CallMethod(const google::protobuf::MethodDescriptor* method,
                            google::protobuf::RpcController* controller,
                            const google::protobuf::Message* request,
                            google::protobuf::Message* response,
                            google::protobuf::Closure* done) {
                                std::string rpc_header_wsize, rpc_header_packet, data;
                                // std::cout<<"Call Server Service: "<<method->service()->name()<<" Method: "<<method->name()<<std::endl;
                                std::string service_name=method->service()->name();
                                unsigned int method_idx=method->index();
                                RPCHeader rpc_header;
                                rpc_header.set_service_name(service_name);
                                rpc_header.set_method_idx(method_idx);
                                rpc_header.SerializeToString(&rpc_header_packet);
                                // std::cout<<"header length is"<<rpc_header_packet.size()<<std::endl;
                                Utils::packUINT32(rpc_header_packet.size(), rpc_header_wsize);
                                request->SerializeToString(&data);
                                connector->sendData(rpc_header_wsize+rpc_header_packet+data);
                                while(1) {
                                    sleep(0.02);
                                    data=connector->recvData();
                                    if(data!="") {
                                        response->ParseFromString(data);
                                        break;
                                    }
                                }
                            }

}