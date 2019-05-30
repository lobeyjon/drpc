#include "simpleHost.h"

namespace drpc {

SimpleHost::SimpleHost(time_t _timeout) {
    host=0;
    state=NET_STATE_STOP;
    index=1;
    socket_fd=0;
    epoll_fd=0;
    port=0;
    timeout=_timeout;
    clients.reserve(MAX_HOST_CLIENTS_INDEX+5);
    service=new ServerServiceImpl();
    controller=new DRpcController();
    channel=new DRpcChannel(nullptr, service, controller);
}

SimpleHost::~SimpleHost() {
    // Delete events in event queue
    clearQueue(connection_event_queue);
}

template<typename T>
void SimpleHost::clearQueue(std::queue<T> q) {
    while(!q.empty()) {
        delete q.front();
        q.pop();
    }
}

// Generate a ID for every client connections
void SimpleHost::generateID(unsigned int& hid, int& pos) {
    int i=0;
    for(;i<clients.size() && clients[i]!=nullptr;++i);
    pos=i;
    if(i==clients.size()) clients.push_back(nullptr);
    hid=(pos & MAX_HOST_CLIENTS_INDEX) | (index << MAX_HOST_CLIENTS_BYTES);
    index+=1;
    if(index>=MAX_HOST_CLIENTS_INDEX) index=1;
}

// Get message event
ConnectionEvent* SimpleHost::read() {
    if(connection_event_queue.empty()) return nullptr;
    ConnectionEvent* event=connection_event_queue.front();
    connection_event_queue.pop();
    return event;
}

// Startup Server
int SimpleHost::startup(int port) {
    SimpleHost::shutdown();
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

int SimpleHost::shutdown() {
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
    for(int i=0;i<clients.size();++i)
        if(clients[i]!=nullptr) clients[i]->closeClient();
    clients.clear();
    clearQueue(connection_event_queue);
    state=NET_STATE_STOP;
    printf("Shutdown Server Success\n");
    return 0;
}

int SimpleHost::getClient(unsigned int hid, ClientConnection* &client) {
    int pos=(hid & MAX_HOST_CLIENTS_INDEX);
    if((pos<0) || (pos>=clients.size())) return -1;
    if(clients[pos]==nullptr) return -2;
    if(clients[pos]->hid!=hid) return -3;
    client=clients[pos].get();
    return 0;
}

int SimpleHost::closeClient(unsigned int hid) {
    ClientConnection* client;
    int code=getClient(hid, client);
    if(code<0) return code;
    client->closeClient();
    return 0;
}

int SimpleHost::sendClient(unsigned int hid, char* data) {
    ClientConnection* client;
    int code=getClient(hid, client);
    if(code<0) return code;
    client->sendData(data);
    client->process();
    return 0;
}

int SimpleHost::clientNoDelay(unsigned int hid, int nodelay) {
    ClientConnection* client;
    int code=getClient(hid, client);
    return client->nodelay(nodelay);
}

int SimpleHost::newClientConnect(time_t current) {
    int connect_fd=0;
    // Accept Client Connection
    if((connect_fd=accept(socket_fd, (struct sockaddr*)nullptr, nullptr))==-1) {
        printf("Accept Connection Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    if(clients.size()>MAX_HOST_CLIENTS_INDEX) {
        if(close(connect_fd)<0) {
            printf("Close Client Connection Error: %s(errno: %d)\n", strerror(errno), errno);
        }
        connect_fd=0;
    }
    if(!connect_fd) {
        printf("Client Connections exceed %d\n", 0xffff);
        return -1;
    }
    // Add Epoll Control
    struct epoll_event ev;
    ev.events=EPOLLIN|EPOLLET;
    ev.data.fd=connect_fd;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connect_fd, &ev)<0) {
        printf("Add Epoll Control Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    unsigned int hid;
    int pos;
    generateID(hid, pos);
    printf("New Client connected: %d %d\n", hid, pos);
    ClientConnection *client=new ClientConnection();
    client->assign(connect_fd);
    client->hid=hid;
    client->active=current;
    struct sockaddr_in connaddr;
    socklen_t sockAddrLen=sizeof(connaddr);
    if(getsockname(socket_fd, (struct sockaddr*)&(connaddr), &sockAddrLen)<0) {
        printf("Get Connection Name Error: %s(errno: %d)\n", strerror(errno), errno);
    }
    client->connaddr=connaddr;
    SP_ClientConnection p(client);
    clients[pos]=p;
    fdClientMap[connect_fd]=client;
    connection_event_queue.push(new ConnectionEvent(NET_CONNECTION_NEW, hid, std::to_string((int)client->connaddr.sin_addr.s_addr)+std::to_string((int)client->connaddr.sin_port)));
}

ClientConnection* SimpleHost::getClientByFd(int connect_fd) {
    if(fdClientMap.find(connect_fd)==fdClientMap.end()) {
        return nullptr;
    }
    return fdClientMap[connect_fd];
}

void SimpleHost::updateClient(time_t current, ClientConnection* client) {
    if(client!=nullptr) {
        client->process();
        while(client->state==NET_STATE_ESTABLISHED) {
            std::string data=client->recvData();
            // printf("updateClient recvData length is %d\n", data.size());
            if(data=="") break;
            
            channel->client=client;
            channel->fromRequest(data);

            connection_event_queue.push(new ConnectionEvent(NET_CONNECTION_DATA, client->hid, data));
            client->active=current;
        }
        time_t _timeout=current-client->active;
        // printf("updateClient check _timeout:%d timeout:%d\n", _timeout, timeout);
        if(client->state==NET_STATE_STOP || _timeout>=timeout) {
            connection_event_queue.push(new ConnectionEvent(NET_CONNECTION_LEAVE, client->hid, ""));
            client->closeClient();
            client=nullptr;
        }
    }
}

int SimpleHost::process() {
    // printf("Process it!\n");
    time_t current=time(nullptr);
    if(state!=NET_STATE_ESTABLISHED) return 0;
    // Epoll Events
    int nfds;
    struct epoll_event events[MAX_HOST_CLIENTS_INDEX+1];
    if((nfds=epoll_wait(epoll_fd, events, MAX_HOST_CLIENTS_INDEX+1, 1000*1))<0) {
        printf("Epoll Wait Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    for(int i=0;i<nfds;++i) {
        if(events[i].data.fd==socket_fd) newClientConnect(current);
        else updateClient(current, getClientByFd(events[i].data.fd));
    }
}

}