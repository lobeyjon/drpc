// #include <iostream>
// #include <fstream>
// #include <string>
// #include "packet.pb.h"
// #include <google/protobuf/io/coded_stream.h>
// #include <google/protobuf/io/zero_copy_stream.h>
// #include <google/protobuf/io/zero_copy_stream_impl.h>
// #include <google/protobuf/io/zero_copy_stream_impl_lite.h>
// using namespace std;


// int main(int argc, char* argv[]) {
//     unsigned int a=12345678;
//     unsigned char buf[4];
//     buf[0]=a>>24;
//     buf[1]=a>>16;
//     buf[2]=a>>8;
//     buf[3]=a;
//     string res(buf, buf+4);
//     cout<<res.size()<<endl;
//     unsigned int b=0;
//     b+=(unsigned char)(res[0])<<24;
//     b+=(unsigned char)(res[1])<<16;
//     b+=(unsigned char)(res[2])<<8;
//     b+=(unsigned char)(res[3]);
//     cout<<b<<endl;


//     // string res;
//     // google::protobuf::io::ZeroCopyOutputStream* zcos = new google::protobuf::io::StringOutputStream(&res);
//     // google::protobuf::io::CodedOutputStream* os = new google::protobuf::io::CodedOutputStream(zcos);
//     // os->WriteLittleEndian32(10);
//     // cout<<"    "<<res.length()<<endl;

//     // unsigned int ans;
//     // google::protobuf::io::ZeroCopyInputStream* zcis = new google::protobuf::io::ArrayInputStream(res.data(), res.size());
//     // google::protobuf::io::CodedInputStream* is = new google::protobuf::io::CodedInputStream(zcis);
//     // is->ReadVarint32(&ans);
//     // cout<<ans<<endl;
    
// }