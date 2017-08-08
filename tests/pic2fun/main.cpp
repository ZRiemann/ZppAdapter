#include <zit/base/trace.h>
#include <zit/base/type.h>
#include <zit/framework/plugin.h>
#include <zit/utility/traceconsole.h>
#include "resource.h"
#include "../../zpp/json_rapid.hpp"
#include "../../zpp/pb2json.h"
#include "../../zpp/zpp.h"

using namespace std;

int main(int argc, char **argv){
    ztrace_reg(ztrace_console, 0);
    ZDBG("Begin test pic2fun");

    zplg_itf_t itf;
    zplg_t plg;
    zplg_open(&itf, "libtst_res.so");
    plg.itf = &itf;
    plg.handle = 0;

    zplg_init(&plg, NULL,NULL,NULL);
    zplg_run(&plg, NULL, NULL);

    Resource res;
    res.Config(1,2,3);
    res.Operate(ZOP_NULL);
    res.LoadFile("../protobuf/msg1.pb");
    res.ToJson();
    zplg_operate(&plg, NULL, &res); // test res->res(pic);
    //res.Dump();
    ZDBG("main dump plg ptr");
    res.DumpP(); // dump new msg ptr
    
    res.ToJsonP();
    res.TstNew2Old();
    zplg_stop(&plg, NULL,NULL);
    zplg_fini(&plg, NULL, NULL);
    zplg_close(&itf);
    ZDBG("End test pic2fun");
    return 0;
}