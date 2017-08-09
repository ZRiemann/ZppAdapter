#include "tstzmq.h"
#include "tstframework.h"
#include <zit/base/type.h>
#include <zit/base/trace.h>
#include <zit/utility/tracebkg.h>
#include <zit/utility/traceconsole.h>
#include <zit/utility/tracelog.h>
#include <iostream>
#include "tstrpdjson.hpp"
#include <string.h>
#include <zpp/zmq.h>
#include <zpp/mysql.h>
#include <zpp/base/object.h>
#include <zpp/zpp.h>

using namespace std;
int ztrace_zpptst(int level, void *user, const char* msg){
    ztrace_log(level, user, msg);
    return ztrace_console(level, user, msg);
}
int main(int argc, char **argv){
    ztrace_logctl("zpptst.log",32*1024*1024);
    ztrace_bkgctl(ztrace_zpptst);
    //ztrace_reg(ztrace_bkg, 0); // thread safe
    ztrace_reg(ztrace_console, 0);

    if(argc < 2 || 0 == strcmp(argv[1], "-h") || 0 == strcmp(argv[1], "--help")){
        zdbg("\nuseage:\n"
             " RpdJson\n"
             " json_rapid\n"
             " Zmq\n"
             " Framework\n");
    }else if(0 == strcmp(argv[1], "RpdJson")){
        tstrpdjson(argc, argv);
    }else if(0 == strcmp(argv[1], "json_rapid")){
        tstjson_rapid(argc, argv);
    }else if(0 == strcmp(argv[1], "Zmq")){
        tstzmq(argc, argv);
    }else if(0 == strcmp(argv[1], "Framework")){
        tstframework(argc, argv);
    }else{
        zdbg("not support arguments.");
    }
    ztrace_bkgend();
    ztrace_logctl(NULL,0); // close the log file.

    return 0;
}
