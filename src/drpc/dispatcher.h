#pragma once

#include <unordered_map>
#include <string>
#include "conf.h"
#include "utils.h"
#include "defs.pb.h"
#include "rpc_controller.h"
#include "rpc_connector.h"
#include "msg_queue.h"

namespace drpc {

class DoneParams {
public:
    DoneParams(google::protobuf::Message* _request, google::protobuf::Message* _response, unsigned int _hid);
    ~DoneParams();
    google::protobuf::Message* request;
    google::protobuf::Message* response;
    unsigned int hid;
};

class Dispatcher {
protected:
    Dispatcher();
public:
    static Dispatcher* getInstance();
    void registerService(google::protobuf::Service* service);
    void dispatch(unsigned int hid, const std::string& data);
    void onRespMsg(DoneParams params);
private:
    static Dispatcher* m_instance;
    std::unordered_map<std::string, google::protobuf::Service*> services;
};

}