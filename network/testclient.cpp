#include "clientConnection.h"

std::string generatePacket() {
    drpc::Names* names=new drpc::Names();
    names->add_name("Panbark");
    names->add_name("LiLei");
    names->add_name("Durex");
    std::string ret;
    names->SerializeToString(&ret);

    drpc::Names *names1=new drpc::Names();
    names1->ParseFromString(ret);
    std::cout<<"updateClients recvData: "<<names1->name(0)<<" "<<names1->name(1)<<" "<<names1->name(2)<<std::endl;
    return ret;
}

int main() {
    drpc::ClientConnection *client=new drpc::ClientConnection();
    std::string ip="127.0.0.1";
    client->connectServer(ip.c_str(), 10000);
    bool flag=false;

    while(1) {
        sleep(1);
        client->process();
        if(!flag) {
            flag=true;
            client->sendData(generatePacket());
        }
    }
}