#include "msg.pb.h"
#include "msg1.pb.h"
#include <iostream>
#include <fstream>
#include <stdio.h>

using namespace std;
using namespace uns;

void listmsg(Msg &msg){
  int i;
  printf("src:\t%08x\ndest:\t%08x\noprt:\t%08x\n", msg.src(), msg.dest(), msg.operate());

  //test memory change
  int size = msg.res_size();
  for(i = 0;  i<size; i++){
    const google::protobuf::Any& res = msg.res(i);
    ::google::protobuf::Any* res1 = msg.mutable_res(i);
    if(res1->Is<AddrIp>()){
      AddrIp addr;
      res1->UnpackTo(&addr);
      addr.set_host("linux-pc1-modify");
      res1->PackFrom(addr);
    }
    printf("res: %p res1: %p\n", &res,  res1);
  }
}
int main(int argc, char **argv){
  Msg msg;

  fstream input("./msg1.pb", ios::in | ios::binary);
  if(!msg.ParseFromIstream(&input)){
    cerr<<"Failed to parse msg."<<endl;
  }else{
      std::cout<<msg.DebugString()<<std::endl;
#if 0 // test set_ip(i);
    listmsg(msg1);

    uns::AddrIp ip;
    cout<<"ip size: "<<ip.ip_size()<<endl;
    ip.add_ip("1.1.1.1");
    cout<<"ip size: "<<ip.ip_size()<<endl;
    ip.set_ip(0, "3.3.3.3");
    cout<<"test set ip(3): \n"<<ip.DebugString()<<endl;
    cout<<"get ip(3): "<<ip.ip(0)<<endl;
#endif
  }
  return 0;
}
