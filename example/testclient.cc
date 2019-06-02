#include <drpc/rpc_channel.h>
#include <echo_service.pb.h>

void testRPC(drpc::Channel* channel) {
    drpc::EchoServer_Stub* stub=new drpc::EchoServer_Stub(channel);
    drpc::EchoRequest* request=new drpc::EchoRequest();
    drpc::EchoResponse* response=new drpc::EchoResponse();
    request->set_message("This is a test rpc from client, Hello!");

    stub->Echo(channel->controller, request, response, nullptr);
    std::cout<<"Callback from RPC response is "<<response->message()<<std::endl;
}

int main() {
    drpc::Channel *channel=new drpc::Channel("127.0.0.1", 10000);

    while(1) {
        sleep(0.02);
        int ret=channel->connector->process();
        if(ret<0) {
            std::cout<<"Disconnect with server"<<std::endl;
            break;
        } else if(ret){
            testRPC(channel);
            break;
        }
    }
}