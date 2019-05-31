#include <drpc/server.h>
#include <echo_service.pb.h>

class EchoServerImpl: public drpc::EchoServer {
public:
    EchoServerImpl() {}
    virtual ~EchoServerImpl() {}

    virtual void Echo(::google::protobuf::RpcController* controller,
                       const ::drpc::EchoRequest* request,
                       ::drpc::EchoResponse* response,
                       ::google::protobuf::Closure* done) {
                           std::cout<<"RPC call from client, request message is "<<request->message()<<std::endl;
                           response->set_message("Echo from Server");
                           done->Run();
                       }
};

int main() {
    drpc::Server server;
    server.registerService(new EchoServerImpl());
    server.run();
}