#include "msg.pb.h"
#include <iostream>
#include <fstream>
#include <stdio.h>

using namespace std;
using namespace uns;

void listmsg(const Msg &msg){
  printf("src:\t%08x\ndest:\t%08x\noprt:\t%08x\n", msg.src(), msg.dest(), msg.operate());
  for (const google::protobuf::Any& resource : msg.res()){
    if (resource.Is<AddrIp>()) {
      AddrIp addr;
      resource.UnpackTo(&addr);
      //dump addr
      printf("addr.type:\t%d\naddr.host:\t%s\n", addr.type(),addr.host().c_str());
    }else if(resource.Is<IOPort>()){
      IOPort io;
      resource.UnpackTo(&io);
      printf("io.type:\t%d\n", io.type());
#if 0
      int size = io.idx_size();
      for(int i=0; i<size; i++){
	unsigned int iostae= io.idx(i);
	printf("io.idx[%d]:\t%d\n", i, iostate);
      }
#else
      int i = 0;
      for(const int &iostate : io.idx()){
	printf("io.idx[%d]:\t%d\n", i++, iostate);
      }
#endif    
    }
  }
}
int main(int argc, char **argv){
  Msg msg1;

  fstream input("./msg.pb", ios::in | ios::binary);
  if(!msg1.ParseFromIstream(&input)){
    cerr<<"Failed to parse msg."<<endl;
  }else{
    listmsg(msg1);
  }
  
  return 0;
}
