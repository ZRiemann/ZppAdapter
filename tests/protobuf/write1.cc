#include "msg.pb.h"
#include "msg1.pb.h"
#include <iostream>
#include <fstream>

using namespace std;
using namespace uns;

int main(int argc, char **argv){
  Msg msg;
  AddrIp addr;
  AddrIp *addr1;
  IOPort io;
  IOPort io1;
  NewItem itm;
  AddrIp addr2;
  NewMsg* newMsg;
  google::protobuf::Any *any;
  msg.set_src(0x01010001);
  msg.set_dest(0x01010002);
  msg.set_operate(0x03000001);
  msg.set_new32(12345);
  msg.set_newstr("new string");

  addr.set_type(11);
  addr.set_host("liux-pc1");
  addr.set_rep_port(5858);
  addr1 = msg.mutable_addr();
  addr1->set_type(11);
  addr1->set_host("liux-pc1");
  addr1->set_rep_port(5858);
  
  io.set_type(22);
  io.add_idx(101);
  io.add_idx(102);
  io.add_idx(103);

  io1.set_type(22);
  io1.add_idx(101);
  io1.add_idx(102);
  io1.add_idx(103);

  itm.set_type(1111);
  itm.set_name("item-name");
  
  msg.add_res()->PackFrom(addr);
  msg.add_res()->PackFrom(io);
  msg.add_res()->PackFrom(itm);
  msg.add_res()->PackFrom(io1);

  any = msg.mutable_any();
  addr2.set_type(11);
  addr2.set_host("liux-pc1");
  addr2.set_rep_port(5858);
  any->PackFrom(addr2);
  
  newMsg = msg.mutable_new_msg();
  newMsg->set_src(1);
  newMsg->set_dest(2);
  
  std::cout<<msg.DebugString()<<std::endl;
  fstream output("./msg1.pb", ios::out | ios::trunc | ios::binary);
  if(!msg.SerializeToOstream(&output)){
    cerr<<"Failed to write msg."<<endl;
  }
  size_t len = msg.ByteSizeLong();
  char *buf = (char*)malloc(len);
  msg.SerializeToArray(buf, len);
  cout<<"array len:"<<len<<endl;
  
  return 0;
}
