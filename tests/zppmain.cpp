#include "tstzmq.h"
#include "tstframework.h"
#include <zit/base/type.h>
#include <zit/base/trace.h>
#include <zit/utility/tracebkg.h>
#include <zit/utility/traceconsole.h>
#include <zit/utility/tracelog.h>
#include <iostream>

using namespace std;
int ztrace_zpptst(int level, void *user, const char* msg){
  ztrace_log(level, user, msg);
  return ztrace_console(level, user, msg);
}
int main(int argc, char **argv){
  ztrace_logctl("zpptst.log",32*1024*1024);
  ztrace_bkgctl(ztrace_zpptst);
  ztrace_reg(ztrace_bkg, 0); // thread safe

#ifdef _ZTST_ZMQ_H_
  tstzmq(argc, argv);
#endif
  
#ifdef _ZTST_FRAMEWORK_H_
  tstframework(argc, argv);
#endif

  ztrace_bkgend();
  ztrace_logctl(NULL,0); // close the log file.

  return 0;
}
