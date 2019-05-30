#pragma once


#include "defs.pb.h"


namespace drpc {

class ServerServiceImpl : public ServerService {
public:
    ServerServiceImpl();
    ~ServerServiceImpl();

  virtual void HelloServer(::google::protobuf::RpcController* controller,
                       const ::drpc::HelloServerRequest* request,
                       ::drpc::HelloServerResponse* response,
                       ::google::protobuf::Closure* done);
};


}