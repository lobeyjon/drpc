#pragma once

#include <cstdio>
#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <queue>
#include <string>
#include <time.h>
#include <memory>
#include <unordered_map>
#include "conf.h"
#include "event.h"
#include "rpc_channel.h"
#include "rpc_controller.h"


namespace drpc {
class Host {
public:
    Host(time_t _timeout=NET_HOST_DEFAULT_TIMEOUT);
    virtual ~Host();

    void generateID(unsigned int& hid, int& pos);
    int startup(int port=10000);
    int shutdown();
    IOEvent* getEvent();
    int getConnector(unsigned int hid, Connector* &connector);
    int sendConnector(unsigned int hid, char* data);
    int closeConnector(unsigned int hid);
    int connectorNoDelay(unsigned int hid, int nodelay=0);
    int newConnector(time_t current);
    Connector* getConnectorByFd(int connect_fd);
    void updateConnector(time_t current, Connector* connector);
    int process();

public:
    int host;
    int state;
    int index;
    int socket_fd;
    int epoll_fd;
    int port;
    int timeout;
    typedef std::shared_ptr<Connector> SP_Connector;
    std::vector<SP_Connector> connectors;
    std::queue<IOEvent*> io_event_queue;
    std::unordered_map<int, Connector*> fdConnectorMap;
    

private:
    template<typename T>
    void clearQueue(std::queue<T> q);
};
}