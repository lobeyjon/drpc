#include "utils.h"

namespace drpc {

void Utils::packUINT32(unsigned int size, std::string& wsize) {
    wsize="";
    for(int i=0;i<UINT32_BYTE_SIZE;++i) {
        // printf("packHeader %d %d\n", i, (unsigned char)(size>>(i*8)));
        wsize+=(unsigned char)(size>>(i*8));
    }
}

void Utils::unpackUINT32(const std::string& wsize, unsigned int& size) {
    size=0;
    for(int i=0;i<UINT32_BYTE_SIZE;++i) {
        // printf("unpackHeader %d %d\n", i, (unsigned char)(wsize[i])<<(i*8));
        size+=(unsigned int)(wsize[i])<<(i*8);
    }
}

}