#include <drpc/rpc_channel.h>
#include <echo_service.pb.h>

int main() {
    drpc::Channel *channel=new drpc::Channel("127.0.0.1", 10000);
    bool flag=false;

    drpc::EchoServer_Stub* stub=new drpc::EchoServer_Stub(channel);
    drpc::EchoRequest* request=new drpc::EchoRequest();
    drpc::EchoResponse* response=new drpc::EchoResponse();
    request->set_message("This is a test rpc from client, Hello!");

    while(1) {
        sleep(0.02);
        channel->connector->process();
        if(!flag) {
            flag=true;
            stub->Echo(channel->controller, request, response, nullptr);
            std::cout<<"Callback from RPC response is "<<response->message()<<std::endl;
        }
    }
}