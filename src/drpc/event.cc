#include "event.h"

namespace drpc {

Event::Event() {

}

Event::~Event() {

}

IOEvent::IOEvent(int _connectionState, unsigned int _hid, std::string _data): connectionState(_connectionState), hid(_hid), data(_data) {}

IOEvent::~IOEvent() {

}

}