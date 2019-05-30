#include "rpc_channel.h"

int main() {
    drpc::Channel *channel=new drpc::Channel("127.0.0.1", 10000);
    bool flag=false;

    drpc::ServerService_Stub* stub=new drpc::ServerService_Stub(channel);
    drpc::HelloServerRequest* request=new drpc::HelloServerRequest();
    drpc::HelloServerResponse* response=new drpc::HelloServerResponse();
    request->set_text("This is a test rpc from client, Hello!");

    while(1) {
        sleep(0.02);
        channel->connector->process();
        if(!flag) {
            flag=true;
            // std::cout<<"call rpc"<<std::endl;
            stub->HelloServer(channel->controller, request, response, nullptr);
            std::cout<<"Callback from RPC response is "<<response->text()<<std::endl;
        }
    }
}