#pragma once

#include <cstdio>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string>
#include <unordered_map>
#include "conf.h"
#include "utils.h"


namespace drpc {

class Connector {
public:
    Connector();
    Connector(const char* ip, int port);
    virtual ~Connector();

    int connectServer(const char* ip, int port);
    int closeConnector();
    int assign(int sock);
    int nodelay(int nodelay);
    int process();
    int sendData(std::string data);
    std::string recvData();
    int setblocking(int block=0);

    int connect_fd;
    std::string send_buf;
    std::string recv_buf;
    int state;
    int errd[3] = {EINPROGRESS, EALREADY, EWOULDBLOCK};
    int conn[3] = {EISCONN, 10057, 10053};
    int errc;
    struct sockaddr_in connaddr;
    int active;
    int hid;

private:
    int tryConnect();
    int sendRaw(std::string data);
    int trySend();
    int tryRecv();
    std::string peekRaw(unsigned int size);
    std::string recvRaw(unsigned int size);
    bool inConn(int code);
    bool inErrd(int code);
};

}