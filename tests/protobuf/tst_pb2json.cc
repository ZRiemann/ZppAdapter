#include "../../zpp/pb2json.h"
#include "../../zpp/zpp.h"
#include "msg.pb.h"
#include "msg1.pb.h"
#include <iostream>
#include <fstream>
#include <zit/base/trace.h>
#include <zit/utility/traceconsole.h>

using namespace uns;
using namespace std;


int main(int argc, char **argv){
  Msg msg;
  ztrace_reg(ztrace_console, NULL);

  fstream input("./msg1.pb", ios::in | ios::binary);
  if(!msg.ParseFromIstream(&input)){
    cerr<<"Failed to parse msg."<<endl;
  }else{
      z::json::RJson js;
      Pb2json(&js, &msg);
      js.Dump();
      Message *pb = NULL;
      Json2pb(&js, &pb);
      if(pb){
          ZDBG("\n%s", pb->DebugString().c_str());
          delete pb;
      }
  }
  return 0;
}
