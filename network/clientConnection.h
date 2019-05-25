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
#include "conf.h"
#include "packet.pb.h"

namespace drpc {

class ClientConnection {
public:
    ClientConnection();
    virtual ~ClientConnection();

    int connectServer(const char* ip, int port);
    int closeClient();
    int assign(int sock);
    int nodelay(int nodelay);
    int process();
    int sendData(std::string data);
    std::string recvData();

public:
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
    void packHeader(unsigned int size, std::string& wsize);
    void unpackHeader(const std::string& wsize, unsigned int& size);
};

}