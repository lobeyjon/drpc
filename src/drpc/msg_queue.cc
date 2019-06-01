#include "msg_queue.h"


namespace drpc {

ReadMsgQueue* ReadMsgQueue::m_instance=new ReadMsgQueue;

ReadMsgQueue::ReadMsgQueue(){
    queue=new boost::lockfree::queue<IOEvent*>(MAX_HOST_CLIENTS_INDEX);
};

ReadMsgQueue* ReadMsgQueue::getInstance() {
    return m_instance;
}

bool ReadMsgQueue::push(IOEvent* event) {
    return queue->push(event);
}

bool ReadMsgQueue::pop() {
    IOEvent* event;
    return queue->pop(event);
}

bool ReadMsgQueue::pop(IOEvent* &event) {
    return queue->pop(event);
}

bool ReadMsgQueue::empty() {
    return queue->empty();
}

WriteMsgQueue* WriteMsgQueue::m_instance=new WriteMsgQueue;

WriteMsgQueue::WriteMsgQueue(){
    queue=new boost::lockfree::queue<IOEvent*>(MAX_HOST_CLIENTS_INDEX);
};

WriteMsgQueue* WriteMsgQueue::getInstance() {
    return m_instance;
}

bool WriteMsgQueue::push(IOEvent* event) {
    return queue->push(event);
}

bool WriteMsgQueue::pop() {
    IOEvent* event;
    return queue->pop(event);
}

bool WriteMsgQueue::pop(IOEvent* &event) {
    return queue->pop(event);
}

bool WriteMsgQueue::empty() {
    return queue->empty();
}

}