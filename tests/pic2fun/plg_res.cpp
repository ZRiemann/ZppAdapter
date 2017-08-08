#include "plg_res.h"
#include "plg_res_imp.h"
#include <zit/base/trace.h>

int init(ZOP_ARG){
    zvalue_t *values = (zvalue_t*)in;
    PlgResImp *imp = new PlgResImp;
    *out = (void*)imp;
    imp->cb = (zoperate)values[0];
    imp->cb_hint = values[1];
    imp->Init(NULL, NULL, hint);
    ZDBGC(ZOK);
    return ZOK;
}

int fini(ZOP_ARG){
    return ((PlgResImp*)in)->Fini(NULL, out, hint);
}

int run(ZOP_ARG){
    return ((PlgResImp*)in)->Run(NULL, out, hint);
}

int stop(ZOP_ARG){
    return ((PlgResImp*)in)->Stop(NULL, out, hint);
}

int operate(ZOP_ARG){
    return ((PlgResImp*)in)->Operate(NULL, out, hint); // hint => Resource*
}

int ability(ZOP_ARG){
    ZDBGC(ZOK);
    return ZOK;
}
