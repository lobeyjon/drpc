#pragma once

#include <unordered_map>
#include <string>
#include "conf.h"
#include "utils.h"
#include "services.h"
#include "defs.pb.h"
#include "rpc_controller.h"
#include "rpc_connector.h"

namespace drpc {

class DoneParams {
public:
    DoneParams(google::protobuf::Message* _request, google::protobuf::Message* _response, Connector* _connector);
    ~DoneParams();
    google::protobuf::Message* request;
    google::protobuf::Message* response;
    Connector* connector;
};

class Dispatcher {
public:
    Dispatcher();
    ~Dispatcher();

    void registerService(google::protobuf::Service* service);
    void dispatch(Connector* connector, const std::string& data);
    void onRespMsg(DoneParams params);

    std::unordered_map<std::string, google::protobuf::Service*> services;
    
};

}