#include <cstdio>
#include <iostream>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <queue>
#include <string>
#include <time.h>
#include <memory>
#include "conf.h"
#include "clientConnection.h"
#include "event.h"

namespace drpc {
class SimpleHost {
public:
    SimpleHost(time_t _timeout=NET_HOST_DEFAULT_TIMEOUT);
    virtual ~SimpleHost();

    void generateID(unsigned int& hid, int& pos);
    int startup(int port=10000);
    int shutdown();
    ConnectionEvent* read();
    int getClient(unsigned int hid, ClientConnection* &client);
    int sendClient(unsigned int hid, char* data);
    int closeClient(unsigned int hid);
    int clientNoDelay(unsigned int hid, int nodelay=0);
    int newClientConnect(time_t current);
    void updateClients(time_t current);
    int process();

public:
    int host;
    int state;
    int index;
    int socket_fd;
    int port;
    int timeout;
    typedef std::shared_ptr<ClientConnection> SP_ClientConnection;
    std::vector<SP_ClientConnection> clients;
    std::queue<ConnectionEvent*> connection_event_queue;

private:
    template<typename T>
    void clearQueue(std::queue<T> q);
};
}