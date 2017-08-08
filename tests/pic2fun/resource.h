#ifndef _ZTST_RESOURCE_H_
#define _ZTST_RESOURCE_H_
/**@class Resource
 * @brief base Resource definition
 */
#include "msg.pb.h"

class Resource{
public:
    int type;
    int id;
    int flag;
    uns::Msg msg;
    uns::Msg *pmsg;
public:
    Resource();
    ~Resource();
    int Config(int type, int id, int flag);
    int Operate(void *in, void **out, void*hint);
    void Dump();
    void ToJson();
    void TstNew2Old();
    int LoadFile(const char *file);
    //int Swap(uns::Msg &msg); // can not swap new msg and old msg
    void DumpP();
    void ToJsonP(); // can not to json for new msg;
};
#endif
