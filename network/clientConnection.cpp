#include "clientConnection.h"

namespace drpc {

ClientConnection::ClientConnection() {
    connect_fd=0;
    state=NET_STATE_STOP;
    errc=0;
}

ClientConnection::~ClientConnection() {

}

int ClientConnection::connectServer(const char* ip, int port) {
    // Create Socket
    if((connect_fd=socket(AF_INET, SOCK_STREAM, 0))<0) {
        printf("Create Socket Error: %s(errno: %d)", strerror(errno), errno);
        return errno;
    }
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

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(port);
    if(inet_pton(AF_INET, ip, &servaddr.sin_addr)<0) {
        printf("Inet_pton Error %s(errno: %d) for ip:%s\n", strerror(errno), errno, ip);
        return errno;
    }
    // Connect Server
    if(connect(connect_fd, (struct sockaddr*)&servaddr, sizeof(servaddr))<0) {
        printf("Connect Error: %s(errno: %d)\n", strerror(errno), errno);
    }
    state=NET_STATE_CONNECTING;
    errc=0;
    return 0;
}

int ClientConnection::closeClient() {
    state=NET_STATE_STOP;
    if(connect_fd==0) return 0;
    close(connect_fd);
    connect_fd=0;
    return 0;
}

int ClientConnection::assign(int sock) {
    closeClient();
    connect_fd=sock;
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
    state=NET_STATE_ESTABLISHED;
    return 0;
}

int ClientConnection::nodelay(int nodelay=0) {
    if(state!=NET_STATE_ESTABLISHED) return -2;
    // No delay
    int optval=1;
    if(setsockopt(connect_fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(int))==-1) {
        close(connect_fd);
        printf("Set Socket Option Error: %s(errno: %d)\n", strerror(errno), errno);
        return errno;
    }
    return 0;
}

int ClientConnection::process() {
    if(state==NET_STATE_STOP) return 0;
    if(state==NET_STATE_CONNECTING) tryConnect();
    if(state==NET_STATE_ESTABLISHED) tryRecv();
    if(state==NET_STATE_ESTABLISHED) trySend();
}

int ClientConnection::tryConnect() {
    printf("TryConnect state:%d\n", state);
    if(state==NET_STATE_ESTABLISHED) return 1;
    if(state!=NET_STATE_CONNECTING) return -1;
    if(recv(connect_fd, (char*)recv_buf.c_str(), 0, 0)<0) {
        printf("Recv Error: %s(errno: %d)\n", strerror(errno), errno);
        if(inConn(errno)) return 0;
        if(inErrd(errno)) {
            state=NET_STATE_ESTABLISHED;
            return 1;
        }
        close(connect_fd);
        return -1;
    }
    state=NET_STATE_ESTABLISHED;
    printf("Connect Established!\n");
    return 1;
}

bool ClientConnection::inConn(int code) {
    for(int i=0;i<3;++i) {
        if(code==conn[i]) return true;
    }
    return false;
}

bool ClientConnection::inErrd(int code) {
    for(int i=0;i<3;++i) {
        if(code==errd[i]) return true;
    }
    return false;
}

void ClientConnection::packHeader(unsigned int size, std::string& wsize) {
    wsize="";
    for(int i=0;i<NET_HEAD_LENGTH_SIZE;++i) {
        printf("packHeader %d %d\n", i, (unsigned char)(size>>(i*8)));
        wsize+=(unsigned char)(size>>(i*8));
    }
}

void ClientConnection::unpackHeader(const std::string& wsize, unsigned int& size) {
    size=0;
    for(int i=0;i<NET_HEAD_LENGTH_SIZE;++i) {
        printf("unpackHeader %d %d\n", i, (unsigned char)(wsize[i])<<(i*8));
        size+=(unsigned int)(wsize[i])<<(i*8);
    }
}

int ClientConnection::sendData(std::string data) {
    unsigned int size=data.size()+NET_HEAD_LENGTH_SIZE;
    std::string wsize;
    packHeader(size, wsize);
    sendRaw(wsize+data);
    return 0;
}

std::string ClientConnection::recvData() {
    std::string rsize = peekRaw(NET_HEAD_LENGTH_SIZE);
    printf("recvData header size is %d:%d\n", rsize.size(), NET_HEAD_LENGTH_SIZE);
    if(rsize.size()<NET_HEAD_LENGTH_SIZE) return "";
    unsigned int size;
    unpackHeader(rsize, size);
    printf("recvData total size is %d:%d\n", recv_buf.size(), size);
    if(recv_buf.size()<size) return "";
    recvRaw(NET_HEAD_LENGTH_SIZE);
    return recvRaw(size-NET_HEAD_LENGTH_SIZE);
}

int ClientConnection::sendRaw(std::string data) {
    send_buf+=data;
    process();
    return 0;
}

int ClientConnection::trySend() {
    unsigned int wsize=0;
    if(send_buf.length()==0) {
        printf("TrySend but nothing to send.\n");
        return 0;
    }
    if((wsize=send(connect_fd, (char*)send_buf.c_str(), send_buf.size(), 0))<0) {
        if(!inErrd(errno)) {
            errc=errno;
            closeClient();
            return -1;
        }
    }
    printf("Send Message Success, send size is:%d\n", wsize);
    send_buf=send_buf.substr(wsize, send_buf.size()-wsize);
    return wsize;
}

int ClientConnection::tryRecv() {
    std::string rdata="";
    int recv_len=0;
    while(1) {
        char text[2048];
        int recv_len=recv(connect_fd, text, 1024, 0);
        printf("TryRecv %d\n", recv_len);
        if(recv_len<0) {
            printf("TryRecv Error: %s(errno: %d)\n", strerror(errno), errno);
            if(!inErrd(errno)) {
                errc=errno;
                closeClient();
                return -1;
            }
        } else if(recv_len==0) {
            errc=10000;
            closeClient();
            return -1;
        }
        if(recv_len<=0) break;
        text[recv_len]='\0';
        rdata+=std::string(text, text+recv_len);
        printf("tryRecv text length is %d, rdata size is %d\n", recv_len, rdata.size());
    }
    recv_buf+=rdata;
    printf("tryRecv recv_buf size is %d\n", recv_buf.size());
    return rdata.size();
}

std::string ClientConnection::peekRaw(unsigned int size) {
    process();
    if(recv_buf.size()==0) return "";
    printf("peekRaw size:%d recv_buf size:%d", size, recv_buf.size());
    return recv_buf.substr(0, (size_t)std::min((size_t)size, recv_buf.size()));
}

std::string ClientConnection::recvRaw(unsigned int size) {
    std::string rdata=peekRaw(size);
    recv_buf=recv_buf.substr(rdata.size(), recv_buf.size()-rdata.size());
    return rdata;
}

}