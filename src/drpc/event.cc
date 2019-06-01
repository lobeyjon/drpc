#include "event.h"

namespace drpc {

IOEvent::IOEvent(): connectionState(-1), hid(-1), data("") {

}

IOEvent::IOEvent(int _connectionState, unsigned int _hid, std::string _data): connectionState(_connectionState), hid(_hid), data(_data) {

}

IOEvent::IOEvent(IOEvent* &event) {
    connectionState=event->connectionState;
    hid=event->hid;
    data=event->data;   
}


}