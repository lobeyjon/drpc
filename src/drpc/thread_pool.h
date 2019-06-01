#pragma once

#include <atomic>
#include <vector>
#include <thread>
#include <iostream>
#include "msg_queue.h"
#include "event.h"
#include "conf.h"
#include "dispatcher.h"
#include <exception>


namespace drpc {

class WorkThreadPool {
protected:
    std::atomic_bool done;
    std::vector<std::thread> threads;
    void workerThread();

public:
    WorkThreadPool();
    ~WorkThreadPool();
};


}