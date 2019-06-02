#include "host.h"

namespace drpc {

Host::Host(time_t _timeout) {
    host=0;
    state=NET_STATE_STOP;
    index=1;
    socket_fd=0;
    epoll_fd=0;
    port=0;
    timeout=_timeout;
    connectors.reserve(MAX_HOST_CLIENTS_INDEX+5);
}

Host::~Host() {
    // Delete events in event queue
    clearQueue(ReadMsgQueue::getInstance());
}

template<typename T>
void Host::clearQueue(T* q) {
    while(!q->empty()) {
        q->pop();
    }
}

// Generate a ID for every client connections
void Host::generateID(unsigned int& hid, int& pos) {
    int i=0;
    for(;i<connectors.size() && connectors[i]!=nullptr;++i);
    pos=i;
    if(i==connectors.size()) connectors.push_back(nullptr);
    hid=(pos & MAX_HOST_CLIENTS_INDEX) | (index << MAX_HOST_CLIENTS_BYTES);
    index+=1;
    if(index>=MAX_HOST_CLIENTS_INDEX) index=1;
}

// Startup Server
int Host::startup(int port) {
    Host::shutdown();
    struct sockaddr_in servaddr;
    // Create Socket
    if((socket_fd=socket(AF_INET, SOCK_STREAM, 0))==-1) {
        printf("Create Socket Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    // Reuse address
    int optval=1;
    if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int))==-1) {
        close(socket_fd);
        printf("Set Socket Option Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    // Bind address
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(port);
    if(bind(socket_fd, (struct sockaddr*)&(servaddr), sizeof(servaddr))==-1) {
        close(socket_fd);
        printf("Bind Socket Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    // Listen connections
    if(listen(socket_fd, MAX_HOST_CLIENTS_INDEX)==-1) {
        close(socket_fd);
        printf("Listen Socket Connection Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    // Set Non-Blocking Socket
    int blocking;
    if(blocking=fcntl(socket_fd, F_GETFL, 0)<0) {
        close(socket_fd);
        printf("Get Socket Blocking State Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    if(fcntl(socket_fd, F_SETFL, blocking|O_NONBLOCK)<0) {
        close(socket_fd);
        printf("Set Non-Blocking Socket Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    // Create epoll
    if((epoll_fd=epoll_create(MAX_HOST_CLIENTS_INDEX+1))==-1) {
        close(epoll_fd);
        printf("Create Epoll Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    // Add epoll control
    struct epoll_event ev;
    struct epoll_event events[MAX_HOST_CLIENTS_INDEX+1];
    ev.events=EPOLLIN|EPOLLET;
    ev.data.fd=socket_fd;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev)==-1) {
        printf("Add Epoll Control Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    // Get sock name
    memset(&servaddr, 0, sizeof(servaddr));
    socklen_t sockAddrLen=sizeof(servaddr);
    if(getsockname(socket_fd, (struct sockaddr*)&(servaddr), &sockAddrLen)<0) {
        close(epoll_fd);
        close(socket_fd);
        printf("Get Socket Name Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    port=ntohs(servaddr.sin_port);
    state=NET_STATE_ESTABLISHED;
    printf("Startup Server Success on port:%d\n", port);
    return 0;
}

int Host::shutdown() {
    if(epoll_fd) {
        if(close(epoll_fd)<0) {
            printf("Close Epoll Error: %s(errno: %d)\n", strerror(errno), errno);
        }
    }
    if(socket_fd) {
        if(close(socket_fd)<0) {
            printf("Close Socket Error: %s(errno: %d)\n", strerror(errno), errno);
        }
    }
    epoll_fd=0;
    socket_fd=0;
    index=1;
    for(int i=0;i<connectors.size();++i)
        if(connectors[i]!=nullptr) connectors[i]->closeConnector();
    connectors.clear();
    clearQueue(ReadMsgQueue::getInstance());
    state=NET_STATE_STOP;
    printf("Shutdown Server Success\n");
    return 0;
}

int Host::getConnector(unsigned int hid, Connector* &connector) {
    int pos=(hid & MAX_HOST_CLIENTS_INDEX);
    if((pos<0) || (pos>=connectors.size())) return -1;
    if(connectors[pos]==nullptr) return -2;
    if(connectors[pos]->hid!=hid) return -3;
    connector=connectors[pos].get();
    return 0;
}

int Host::closeConnector(unsigned int hid) {
    Connector* connector;
    int code=getConnector(hid, connector);
    if(code<0) return code;
    connector->closeConnector();
    return 0;
}

int Host::sendConnector(unsigned int hid, char* data) {
    Connector* connector;
    int code=getConnector(hid, connector);
    if(code<0) return code;
    connector->sendData(data);
    connector->process();
    return 0;
}

int Host::connectorNoDelay(unsigned int hid, int nodelay) {
    Connector* connector;
    int code=getConnector(hid, connector);
    return connector->nodelay(nodelay);
}

int Host::newConnector(time_t current) {
    int connect_fd=0;
    // Accept Client Connection
    if((connect_fd=accept(socket_fd, (struct sockaddr*)nullptr, nullptr))==-1) {
        printf("Accept Connection Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    if(connectors.size()>MAX_HOST_CLIENTS_INDEX) {
        if(close(connect_fd)<0) {
            printf("Close Client Connection Error: %s(errno: %d)\n", strerror(errno), errno);
        }
        connect_fd=0;
    }
    if(!connect_fd) {
        printf("Client Connections exceed %d\n", 0xffff);
        return -1;
    }
    unsigned int hid;
    int pos;
    generateID(hid, pos);
    // printf("New Client connected: %d %d\n", hid, pos);
    Connector *connector=new Connector();
    connector->assign(connect_fd, epoll_fd);
    connector->hid=hid;
    connector->active=current;
    struct sockaddr_in connaddr;
    socklen_t sockAddrLen=sizeof(connaddr);
    if(getsockname(socket_fd, (struct sockaddr*)&(connaddr), &sockAddrLen)<0) {
        printf("Get Connection Name Error: %s(errno: %d)\n", strerror(errno), errno);
    }
    connector->connaddr=connaddr;
    SP_Connector p(connector);
    connectors[pos]=p;
    fdConnectorMap[connect_fd]=connector;
    ReadMsgQueue::getInstance()->push(new IOEvent(NET_CONNECTION_NEW, hid, std::to_string((int)connector->connaddr.sin_addr.s_addr)+std::to_string((int)connector->connaddr.sin_port)));
}

Connector* Host::getConnectorByFd(int connect_fd) {
    if(fdConnectorMap.find(connect_fd)==fdConnectorMap.end()) {
        return nullptr;
    }
    return fdConnectorMap[connect_fd];
}

void Host::updateConnector(time_t current, Connector* connector) {
    if(connector!=nullptr) {
        connector->process();
        while(connector->state==NET_STATE_ESTABLISHED) {
            std::string data=connector->recvData();
            if(data=="") break;
            ReadMsgQueue::getInstance()->push(new IOEvent(NET_CONNECTION_DATA, connector->hid, data));
            connector->active=current;
        }
        time_t _timeout=current-connector->active;
        if(connector->state==NET_STATE_STOP || _timeout>=timeout) {
            ReadMsgQueue::getInstance()->push(new IOEvent(NET_CONNECTION_LEAVE, connector->hid, ""));
            connector->closeConnector();
            connector=nullptr;
        }
    }
}

int Host::process() {
    // printf("Process it!\n");
    time_t current=time(nullptr);
    if(state!=NET_STATE_ESTABLISHED) return 0;
    // Epoll Events
    int nfds;
    struct epoll_event events[MAX_HOST_CLIENTS_INDEX+1];
    if((nfds=epoll_wait(epoll_fd, events, MAX_HOST_CLIENTS_INDEX+1, 20))<0) {
        printf("Epoll Wait Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    for(int i=0;i<nfds;++i) {
        if(events[i].data.fd==socket_fd) newConnector(current);
        else updateConnector(current, getConnectorByFd(events[i].data.fd));
    }
    Connector* connector;
    IOEvent* event;
    while(WriteMsgQueue::getInstance()->pop(event)) {
        if(getConnector(event->hid, connector)==0) {
            connector->sendData(event->data);
        }
        if(event!=nullptr) {
            delete event;
            event=nullptr;
        }
    }
}

}