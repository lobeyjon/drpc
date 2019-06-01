#include "server.h"

namespace drpc {
    
Server::Server() {
    host=new Host();
    work_thread_pool=new WorkThreadPool();
}

Server::~Server() {
    delete host;
    host=nullptr;
    delete work_thread_pool;
    work_thread_pool=nullptr;
}

void Server::run() {
    host->startup();
    while(1) {
        tick();
    }
}

void Server::registerService(google::protobuf::Service* service) {
    Dispatcher::getInstance()->registerService(service);
}

void Server::tick() {
    host->process();
}

}