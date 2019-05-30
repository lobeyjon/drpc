#include "services.h"

namespace drpc {

ServerServiceImpl::ServerServiceImpl() {

}

ServerServiceImpl::~ServerServiceImpl() {

}

void ServerServiceImpl::HelloServer(::google::protobuf::RpcController* controller,
                       const ::drpc::HelloServerRequest* request,
                       ::drpc::HelloServerResponse* response,
                       ::google::protobuf::Closure* done) {
                            std::cout<<"RPC from client, HelloServer request parameters are:"<<request->text()<<std::endl;
                            response->set_text("RPC returns from server, HelloServer response");
                            done->Run();
                       }

}