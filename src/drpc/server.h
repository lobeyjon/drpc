#include "host.h"
#include "dispatcher.h"

namespace drpc {

class Server {
public:
    Server();
    ~Server();

    void run();
    void registerService(google::protobuf::Service* service);

private:
    void tick();

    Host* host;
    Dispatcher* dispatcher;
};

}