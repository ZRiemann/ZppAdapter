#include "tstzmq.h"
#include "tstframework.h"
#include <zit/base/type.h>
#include <zit/base/trace.h>
#include <zit/utility/traceconsole.h>
#include <zit/utility/tracelog.h>
#include <iostream>

using namespace std;
int ztrace_zpptst(int level, void *user, const char* msg){
  ztrace_log(level, user, msg);
  return ztrace_console(level, user, msg);
}
int main(int argc, char **argv){
  ztrace_reg(ztrace_zpptst, 0);
  ztrace_logctl("zpptst.log", 16*1024*1024);

#ifdef _ZTST_ZMQ_H_
  tstzmq(argc, argv);
#endif
  
#ifdef _ZTST_FRAMEWORK_H_
  tstframework(argc, argv);
#endif

  return 0;
}
