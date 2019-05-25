#include "clientConnection.h"
#include "drpcChannel.h"

// std::string generatePacket() {
//     drpc::Names* names=new drpc::Names();
//     names->add_name("Panbark");
//     names->add_name("LiLei");
//     names->add_name("Durex");
//     std::string ret;
//     names->SerializeToString(&ret);

//     drpc::Names *names1=new drpc::Names();
//     names1->ParseFromString(ret);
//     std::cout<<"updateClients recvData: "<<names1->name(0)<<" "<<names1->name(1)<<" "<<names1->name(2)<<std::endl;
//     return ret;
// }

int main() {
    drpc::ClientConnection *client=new drpc::ClientConnection();
    std::string ip="127.0.0.1";
    client->connectServer(ip.c_str(), 10000);
    bool flag=false;

    google::protobuf::RpcController* controller=new drpc::DRpcController();
    google::protobuf::RpcChannel* channel=new drpc::DRpcChannel(client, nullptr, controller);
    drpc::ServerService_Stub* stub=new drpc::ServerService_Stub(channel);
    drpc::ByeRequest* request=new drpc::ByeRequest();
    request->set_text("This is a test rpc argemnet, Bye Bye!");

    while(1) {
        sleep(1);
        client->process();
        if(!flag) {
            flag=true;
            stub->Bye(controller, request, nullptr, nullptr);
        }
    }
}