#include "simpleHost.h"

int main() {
    drpc::SimpleHost* host=new drpc::SimpleHost();
    host->startup();
    while(1) {
        host->process();
    }
}