#pragma once

#include <string>
#include "conf.h"

namespace drpc {

class Utils {
public:
    static void unpackUINT32(const std::string& wsize, unsigned int& size);
    static void packUINT32(const unsigned int size, std::string& wsize);
};


}