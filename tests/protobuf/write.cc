#include "msg.pb.h"
#include <iostream>
#include <fstream>

using namespace std;
using namespace uns;

int main(int argc, char **argv){
  Msg msg;
  AddrIp addr;
  IOPort io;
  
  msg.set_src(0x01010001);
  msg.set_dest(0x01010002);
  msg.set_operate(0x03000001);

  addr.set_type(11);
  addr.set_host("liux-pc1");
  addr.set_rep_port(5858);

  io.set_type(22);
  io.add_idx(101);
  io.add_idx(102);
  io.add_idx(103);
  
  msg.add_res()->PackFrom(addr);
  msg.add_res()->PackFrom(io);
  
  fstream output("./msg.pb", ios::out | ios::trunc | ios::binary);
  if(!msg.SerializeToOstream(&output)){
    cerr<<"Failed to write msg."<<endl;
  }
  return 0;
}
