#pragma once

#include <boost/lockfree/queue.hpp>
#include "event.h"
#include "conf.h"

namespace drpc {

class ReadMsgQueue {
protected:
    ReadMsgQueue();
private:
    static ReadMsgQueue* m_instance;
    boost::lockfree::queue<IOEvent*>* queue;
public:
    static ReadMsgQueue* getInstance();
    bool push(IOEvent* event);
    bool pop();
    bool pop(IOEvent* &event);
    bool empty();
};

class WriteMsgQueue {
protected:
    WriteMsgQueue();
private:
    static WriteMsgQueue* m_instance;
    boost::lockfree::queue<IOEvent*>* queue;
public:
    static WriteMsgQueue* getInstance();
    bool push(IOEvent* event);
    bool pop();
    bool pop(IOEvent* &event);
    bool empty();
};

}