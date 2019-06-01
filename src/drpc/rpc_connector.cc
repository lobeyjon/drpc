#include "rpc_connector.h"

namespace drpc {

Connector::Connector() {
    connect_fd=0;
    epoll_fd=0;
    state=NET_STATE_STOP;
    errc=0;
    is_listen_epoll_out=false;
}

Connector::Connector(const char* ip, int port) {
    Connector();
    // Create Socket
    if((connect_fd=socket(AF_INET, SOCK_STREAM, 0))<0) {
        printf("Create Socket Error: %s(errno: %d)", strerror(errno), errno);
        return;
    }
    // Set Non-Blocking Socket
    int blocking;
    if(blocking=fcntl(connect_fd, F_GETFL, 0)<0) {
        close(connect_fd);
        printf("Get Socket Blocking State Error: %s(errno: %d)\n", strerror(errno), errno);
        return;
    }
    if(fcntl(connect_fd, F_SETFL, blocking|O_NONBLOCK)<0) {
        close(connect_fd);
        printf("Set Non-Blocking Socket Error: %s(errno: %d)\n", strerror(errno), errno);
        return;
    }
    // Keep Alive
    int optval=1;
    if(setsockopt(connect_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(int))==-1) {
        close(connect_fd);
        printf("Set Socket Option Error: %s(errno: %d)\n", strerror(errno), errno);
        return;
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(port);
    if(inet_pton(AF_INET, ip, &servaddr.sin_addr)<0) {
        printf("Inet_pton Error %s(errno: %d) for ip:%s\n", strerror(errno), errno, ip);
        return;
    }
    // Connect Server
    if(connect(connect_fd, (struct sockaddr*)&servaddr, sizeof(servaddr))<0) {
        // printf("Connect Error: %s(errno: %d)\n", strerror(errno), errno);
    }
    state=NET_STATE_CONNECTING;
    errc=0;
    return;
}

Connector::~Connector() {

}

int Connector::closeConnector() {
    state=NET_STATE_STOP;
    if(connect_fd==0) return 0;
    close(connect_fd);
    connect_fd=0;
    return 0;
}

int Connector::setblocking(int block) {
    int blocking;
    if(blocking=fcntl(connect_fd, F_GETFL, 0)<0) {
        close(connect_fd);
        printf("Get Socket Blocking State Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    if(fcntl(connect_fd, F_SETFL, block==0?(blocking|O_NONBLOCK):(blocking&~O_NONBLOCK))<0) {
        close(connect_fd);
        printf("Set Non-Blocking Socket Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    return 0;
}

int Connector::setEpollIn() {
    // Modify Epoll Control
    struct epoll_event ev;
    ev.events=EPOLLIN|EPOLLET;
    ev.data.fd=connect_fd;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, connect_fd, &ev)<0) {
        printf("Set EpollIn Control Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    is_listen_epoll_out=false;
    printf("Set EpollIn Succeed.\n");
    return 0;
}

int Connector::addEpollOut() {
    // Modify Epoll Control
    struct epoll_event ev;
    ev.events=EPOLLIN|EPOLLOUT|EPOLLET;
    ev.data.fd=connect_fd;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, connect_fd, &ev)<0) {
        printf("Add EpollOut Control Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    is_listen_epoll_out=true;
    printf("Add EpollOut Succeed.\n");
    return 0;
}

int Connector::assign(int sock, int _epoll_fd) {
    closeConnector();
    connect_fd=sock;
    epoll_fd=_epoll_fd;
    // Set Non-Blocking Socket
    int blocking;
    if(blocking=fcntl(connect_fd, F_GETFL, 0)<0) {
        close(connect_fd);
        printf("Get Socket Blocking State Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    if(fcntl(connect_fd, F_SETFL, blocking|O_NONBLOCK)<0) {
        close(connect_fd);
        printf("Set Non-Blocking Socket Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    // Keep Alive
    int optval=1;
    if(setsockopt(connect_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(int))==-1) {
        close(connect_fd);
        printf("Set Socket Option Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    // Add Epoll Control
    struct epoll_event ev;
    ev.events=EPOLLIN|EPOLLET;
    ev.data.fd=connect_fd;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connect_fd, &ev)<0) {
        printf("Set EpollIn Control Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    is_listen_epoll_out=false;
    state=NET_STATE_ESTABLISHED;
    return 0;
}

int Connector::nodelay(int nodelay=0) {
    if(state!=NET_STATE_ESTABLISHED) return -2;
    // No delay
    if(setsockopt(connect_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(int))==-1) {
        close(connect_fd);
        printf("Set Socket Option Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    return 0;
}

int Connector::process() {
    if(state==NET_STATE_STOP) return 0;
    if(state==NET_STATE_CONNECTING) tryConnect();
    if(state==NET_STATE_ESTABLISHED) tryRecv();
    if(state==NET_STATE_ESTABLISHED) trySend();
}

int Connector::tryConnect() {
    // printf("TryConnect state:%d\n", state);
    if(state==NET_STATE_ESTABLISHED) return 1;
    if(state!=NET_STATE_CONNECTING) return -1;
    if(recv(connect_fd, (char*)recv_buf.c_str(), 0, 0)<0) {
        // printf("Recv Error: %s(errno: %d)\n", strerror(errno), errno);
        if(inConn(errno)) return 0;
        if(inErrd(errno)) {
            state=NET_STATE_ESTABLISHED;
            printf("Connect Established!\n");
            return 1;
        }
        close(connect_fd);
        return -1;
    }
    state=NET_STATE_ESTABLISHED;
    printf("Connect Established!\n");
    return 1;
}

bool Connector::inConn(int code) {
    for(int i=0;i<3;++i) {
        if(code==conn[i]) return true;
    }
    return false;
}

bool Connector::inErrd(int code) {
    for(int i=0;i<3;++i) {
        if(code==errd[i]) return true;
    }
    return false;
}

int Connector::sendData(std::string data) {
    unsigned int size=data.size()+NET_HEAD_LENGTH_SIZE;
    std::string wsize;
    Utils::packUINT32(size, wsize);
    sendRaw(wsize+data);
    return 0;
}

std::string Connector::recvData() {
    std::string rsize = peekRaw(NET_HEAD_LENGTH_SIZE);
    // printf("recvData header size is %d:%d\n", rsize.size(), NET_HEAD_LENGTH_SIZE);
    if(rsize.size()<NET_HEAD_LENGTH_SIZE) return "";
    unsigned int size;
    Utils::unpackUINT32(rsize, size);
    // printf("recvData total size is %d:%d\n", recv_buf.size(), size);
    if(recv_buf.size()<size) return "";
    recvRaw(NET_HEAD_LENGTH_SIZE);
    return recvRaw(size-NET_HEAD_LENGTH_SIZE);
}

int Connector::sendRaw(std::string data) {
    send_buf+=data;
    process();
    return 0;
}

int Connector::trySend() {
    unsigned int wsize=0;
    if(send_buf.length()==0) {
        // printf("TrySend but nothing to send.\n");
        return 0;
    }
    if((wsize=send(connect_fd, (char*)send_buf.c_str(), send_buf.size(), 0))<0) {
        if(!inErrd(errno)) {
            errc=errno;
            closeConnector();
            return -1;
        }
    }
    if(wsize<0) wsize=0;
    // printf("Send Message Success, send size is:%d\n", wsize);
    send_buf=send_buf.substr(wsize, send_buf.size()-wsize);
    // if send_buf is not empty, means the send_buff in kernel is full, must listen the writable event by epoll
    if(!send_buf.empty() && !is_listen_epoll_out) addEpollOut();
    else if(is_listen_epoll_out) setEpollIn();
    return wsize;
}

int Connector::tryRecv() {
    std::string rdata="";
    int recv_len=0;
    while(1) {
        char text[2048];
        // printf("TryRecv\n");
        int recv_len=recv(connect_fd, text, 1024, 0);
        // printf("TryRecv %d\n", recv_len);
        if(recv_len<0) {
            // printf("TryRecv Error: %s(errno: %d)\n", strerror(errno), errno);
            if(!inErrd(errno)) {
                errc=errno;
                closeConnector();
                return -1;
            }
        } else if(recv_len==0) {
            errc=10000;
            closeConnector();
            return -1;
        }
        if(recv_len<=0) break;
        text[recv_len]='\0';
        rdata+=std::string(text, text+recv_len);
        // printf("tryRecv text length is %d, rdata size is %d\n", recv_len, rdata.size());
    }
    recv_buf+=rdata;
    // printf("tryRecv recv_buf size is %d\n", recv_buf.size());
    return rdata.size();
}

std::string Connector::peekRaw(unsigned int size) {
    process();
    if(recv_buf.size()==0) return "";
    // printf("peekRaw size:%d recv_buf size:%d\n", size, recv_buf.size());
    return recv_buf.substr(0, (size_t)std::min((size_t)size, recv_buf.size()));
}

std::string Connector::recvRaw(unsigned int size) {
    std::string rdata=peekRaw(size);
    recv_buf=recv_buf.substr(rdata.size(), recv_buf.size()-rdata.size());
    return rdata;
}


}