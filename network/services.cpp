#include "services.h"

namespace drpc {

ServerServiceImpl::ServerServiceImpl() {

}

ServerServiceImpl::~ServerServiceImpl() {

}

void ServerServiceImpl::Hello(::google::protobuf::RpcController* controller,
                       const ::drpc::HelloRequest* request,
                       ::drpc::HelloResponse* response,
                       ::google::protobuf::Closure* done) {
                            std::cout<<"RPC from client method:Hello request:"<<request->text()<<std::endl;
                       }

void ServerServiceImpl::Bye(::google::protobuf::RpcController* controller,
                       const ::drpc::ByeRequest* request,
                       ::drpc::ByeResponse* response,
                       ::google::protobuf::Closure* done) {
                           std::cout<<"RPC from client method: Bye request:"<<request->text()<<std::endl;
                       }

}