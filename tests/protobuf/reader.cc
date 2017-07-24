#include "msg.pb.h"
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
  std::cout<<msg.DebugString()<<std::endl;
#if 0
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
#endif
}
int main(int argc, char **argv){
  Msg msg1;

  fstream input("./msg1.pb", ios::in | ios::binary);
  if(!msg1.ParseFromIstream(&input)){
    cerr<<"Failed to parse msg."<<endl;
  }else{
      std::cout<<msg1.DebugString()<<std::endl;
      listmsg(msg1);

      fstream output("./msg2.pb", ios::out | ios::trunc | ios::binary);
      if(!msg1.SerializeToOstream(&output)){
          cerr<<"Failed to write msg."<<endl;
      }
  }
#if 0
  else{
    listmsg(msg1);
#if 1 // test set_ip(i);
    uns::AddrIp ip;
    cout<<"ip size: "<<ip.ip_size()<<endl;
    ip.add_ip("1.1.1.1");
    cout<<"ip size: "<<ip.ip_size()<<endl;
    ip.set_ip(0, "3.3.3.3");
    cout<<"test set ip(3): \n"<<ip.DebugString()<<endl;
    cout<<"get ip(3): "<<ip.ip(0)<<endl;
#endif
   
#if 0 // test merg from
    // merge no any
    uns::AddrIp addr1;
    uns::AddrIp addr2;
    // stuff addr1 , addr2
    addr1.set_type(1);
    //addr1.set_host("host1");
    addr1.set_rep_port(1);
    addr1.add_ip("1.1.1.1");
    addr1.add_ip("11.11.11.11");
    
    addr2.set_type(2);
    addr2.set_host("host2");
    addr2.set_pull_port(2);
    addr2.add_ip("2.2.2.2");
    addr2.add_ip("1.1.1.1");
    //cout<<"addr1 pull_port: "<< addr1.field_count()<<endl;
#if 1 // 获取字段属性
    const google::protobuf::Descriptor* descriptor = addr1.GetDescriptor();
    const google::protobuf::Reflection* reflection = addr1.GetReflection();
    vector< const google::protobuf::FieldDescriptor * > vfd;
    reflection->ListFields (addr1,  &vfd);
    cout<<"addr1 vfd size: "<<vfd.size()<<endl;
    for(int j = 0; j<vfd.size(); j++){
      cout<<"addr vfd["<<j<<"] :"<<vfd[j]->number()<<endl;
    }
    cout<<"addr1 field_count(): "<< descriptor->field_count()<<endl;
    for (int i = 0; i < descriptor->field_count(); ++i) {
        const google::protobuf::FieldDescriptor* field = descriptor->field(i);
        //bool has_field = reflection->HasField(addr1, field);
	cout<<"field["<<i<<"] fieldsize:"<<reflection->FieldSize(addr1, field)<<endl;
    }
#endif
    cout<<"addr1 before merge:\n"<<addr1.DebugString()<<endl;
    cout<<"addr2 before merge:\n"<<addr2.DebugString()<<endl;
    addr1.MergeFrom(addr2);
    cout<<"addr1 after merge:\n"<<addr1.DebugString()<<endl;
    cout<<"addr2 after merge:\n"<<addr2.DebugString()<<endl;
    // merge has any
    
#endif
#if 0 // test sawp
    // 结论: swap 是引用互换，效率极高
    Msg msg0;
    cout<<"msg1 DebugString:\n"<<msg1.DebugString()<<endl;
    cout<<"msg0 DebugString:\n"<<msg0.DebugString()<<endl;
    cout<<"msg0 IsInitialized: "<<msg0.IsInitialized()<<endl;
    cout<<"msg1 IsInitialized: "<<msg1.IsInitialized()<<endl;
    cout<<"msg0.Swap(&msg1)"<<endl;
    msg0.Swap(&msg1);
    cout<<"msg0 IsInitialized: "<<msg0.IsInitialized()<<endl;
    cout<<"msg1 IsInitialized: "<<msg1.IsInitialized()<<endl;
    cout<<"msg1 DebugString:\n"<<msg1.DebugString()<<endl;
    cout<<"msg0 DebugString:\n"<<msg0.DebugString()<<endl;

    std::string block;
    msg0.SerializeToString(&block);
    cout<<"block size<"<<block.size()<<">"<<endl;
    
    if(!msg1.ParseFromString(block)){
      cerr<<"Failed to parse msg."<<endl;
    }else{
      cout<<"msg1 DebugStringxxx:\n"<<msg1.DebugString()<<endl;
    }
#endif
  }
#endif
  return 0;
}
