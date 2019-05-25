#include <string>

namespace drpc {
    
class Event {
public:
    Event();
    virtual ~Event();

};

class ConnectionEvent : public Event {
public:
    ConnectionEvent(int connectionState, unsigned int hid, std::string data);
    virtual ~ConnectionEvent();
};

}