#pragma once


#include "servermethod.pb.h"


namespace drpc {

class ServerServiceImpl : public ServerService {
public:
    ServerServiceImpl();
    ~ServerServiceImpl();

  virtual void Hello(::google::protobuf::RpcController* controller,
                       const ::drpc::HelloRequest* request,
                       ::drpc::HelloResponse* response,
                       ::google::protobuf::Closure* done);

  virtual void Bye(::google::protobuf::RpcController* controller,
                       const ::drpc::ByeRequest* request,
                       ::drpc::ByeResponse* response,
                       ::google::protobuf::Closure* done);
};


}