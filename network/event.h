#pragma once


#include <string>

namespace drpc {
    
class Event {
public:
    Event();
    virtual ~Event();

};

class IOEvent : public Event {
public:
    IOEvent(int _connectionState, unsigned int _hid, std::string _data);
    virtual ~IOEvent();
public:
    int connectionState;
    unsigned int hid;
    std::string data;
};

}