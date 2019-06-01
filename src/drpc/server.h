#include "host.h"
#include "dispatcher.h"
#include "thread_pool.h"
#include <boost/lockfree/queue.hpp>

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
    WorkThreadPool* work_thread_pool;
    
};

}