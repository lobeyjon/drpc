# drpc
A simple c++ rpc framework with protocol buffer.
## Overview
* drpc now only supports running on linux.
* Use protocol buffer as IDL to describe rpc interface and network packet.
* Use non-blocking socket with epoll to manage IO events.
* Sinlge IO thread and a thread pool of worker threads, use boost lock free queue to publish/subscribe messages between IO thread and worker threads.
## Compile and Install
Environment: `protobuf`, `boost`, `linux`, `g++`

    cd drpc/
    make
    make install
    cd example/
    make
## How to use
#### 1. Define custom rpc interface in your proto file
```proto
message EchoRequest {
    required string message=1;
}

message EchoResponse {
    required string message=1;
}

service EchoServer {
    rpc Echo(EchoRequest) returns(EchoResponse);
}
```
compile and generate cpp file:

    cd drpc/example/
    protoc -I=. --cpp_out=. custom_file_name.proto

#### 2. Implement your client-side code
```cpp
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
  testRPC(channel);
  ...
 }
```
#### 3. Implement your server-side code
```cpp
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
```
