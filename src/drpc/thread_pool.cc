#include "thread_pool.h"

namespace drpc {

WorkThreadPool::WorkThreadPool(): done(false) {
    unsigned const thread_count=std::thread::hardware_concurrency();
    std::cout<<"Threads number is "<<thread_count<<" in thread pool"<<std::endl;
    try {
        for(unsigned i=0;i<thread_count;++i) {
            threads.push_back(std::thread(&WorkThreadPool::workerThread, this));
        }
    } catch(std::exception e) {
        done=true;
        throw;
    }
}

WorkThreadPool::~WorkThreadPool() {
    done=true;
}

void WorkThreadPool::workerThread() {
    while(!done) {
        sleep(0.02);
        IOEvent* event;
        if(ReadMsgQueue::getInstance()->pop(event)) {
            if(event==nullptr) {
                continue;
            }
            if(event->connectionState==NET_CONNECTION_NEW) {
                //TODO
            }
            else if(event->connectionState==NET_CONNECTION_DATA) {
                Dispatcher::getInstance()->dispatch(event->hid, event->data);
            }
            else if(event->connectionState==NET_CONNECTION_LEAVE) {
                //TODO
            }
            delete event;
            event=nullptr;
        }
    }
}

}