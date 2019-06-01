#pragma once


#include <string>

namespace drpc {

class IOEvent {
public:
    int connectionState;
    unsigned int hid;
    std::string data;
    IOEvent();
    IOEvent(int _connectionState, unsigned int _hid, std::string _data);
    IOEvent(IOEvent* &event);
};

}