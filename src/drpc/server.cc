#include "server.h"

namespace drpc {
    
Server::Server() {
    host=new Host();
    dispatcher=new Dispatcher();
}

Server::~Server() {
    delete host;
    host=nullptr;
    delete dispatcher;
    dispatcher=nullptr;
}

void Server::run() {
    host->startup();
    while(1) {
        tick();
    }
}

void Server::registerService(google::protobuf::Service* service) {
    dispatcher->registerService(service);
}

void Server::tick() {
    host->process();
    while(1) {
        IOEvent* event=host->getEvent();
        if(event==nullptr) {
            break;
        } 
        else if(event->connectionState==NET_CONNECTION_NEW) {
            //TODO
        }
        else if(event->connectionState==NET_CONNECTION_DATA) {
            // std::cout<<"Dispatcher services"<<std::endl;
            Connector* connector;
            host->getConnector(event->hid, connector);
            dispatcher->dispatch(connector, event->data);
        }
        else if(event->connectionState==NET_CONNECTION_LEAVE) {
            //TODO
        }
        delete event;
        event=nullptr;
    }
}

}