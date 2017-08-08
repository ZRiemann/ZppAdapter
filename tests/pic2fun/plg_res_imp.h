#ifndef _ZTST_PLG_RES_IMP_H_
#define _ZTST_PLG_RES_IMP_H_

#include <zit/base/type.h>

class PlgResImp{
public:
    PlgResImp();
    ~PlgResImp();
    int Init(ZOP_ARG);
    int Fini(ZOP_ARG);
    int Run(ZOP_ARG);
    int Stop(ZOP_ARG);
    int Operate(ZOP_ARG);
public:
    int id;
    zoperate cb;
    zvalue_t cb_hint;
};
#endif
