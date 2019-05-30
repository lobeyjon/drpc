#include "host.h"
#include "services.h"
#include "dispatcher.h"

namespace drpc {

class Server {
public:
    Server();
    ~Server();

    void run();

private:
    void tick();

    Host* host;
    Dispatcher* dispatcher;
};

}