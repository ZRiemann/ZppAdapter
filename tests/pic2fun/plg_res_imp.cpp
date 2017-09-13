#include "plg_res_imp.h"
#include "resource.h"
#include "../protobuf/msg1.pb.h"
#include <zit/base/trace.h>
#include <fstream>
#include "../../zpp/pb2json.h"
#include "../../zpp/zpp.h"

PlgResImp::PlgResImp(){
    ZDBGC(ZOK);
}
PlgResImp::~PlgResImp(){
    ZDBGC(ZOK);
}
int PlgResImp::Init(ZOP_ARG){
    ZDBGC(ZOK);
    return ZOK;
}
int PlgResImp::Fini(ZOP_ARG){
    ZDBGC(ZOK);
    return ZOK;
}
int PlgResImp::Run(ZOP_ARG){
    ZDBGC(ZOK);
    return ZOK;
}
int PlgResImp::Stop(ZOP_ARG){
    ZDBGC(ZOK);
    return ZOK;
}
int PlgResImp::Operate(ZOP_ARG){
    Resource *res = (Resource*)hint;
    //res->LoadFile("../protobuf/msg1.pb");
    uns::Msg *msg = new uns::Msg;
    std::fstream input("../protobuf/msg1.pb", std::ios::in | std::ios::binary);
    if(!msg->ParseFromIstream(&input)){
        ZERR("LOAD FAIL");
    }else{
        ZDBG("libtst_res::Dump(%d)\n%s\n", msg->ByteSizeLong(), msg->DebugString().c_str());
    }
    ZDBG("plg dump old msg:");
    res->Dump();
    // find newItem from old message
    int ret = ZOK;
    int i, size;
    ::google::protobuf::Any* attr = NULL;
    uns::NewItem item;
    size = res->msg.res_size();
    for(i = 0; i<size; i++){
        attr = res->msg.mutable_res(i);
        if(attr->Is<uns::NewItem>()){
            break;
        }else{
            attr = NULL;
        }
    }
    if(attr){
        attr->UnpackTo(&item);
        ZDBG("find new item from old msg OK: \n%s", item.DebugString().c_str());
    }else{
        ret = ZNOT_EXIST;
        ZDBG("find new item from old msg NOT_EXIST");
    }
    //res->Swap(msg); // can not swap
    res->pmsg = msg;
    res->ToJsonP();
    res->Operate(NULL, out, NULL);
    ZDBGC(ret);
    return ret;
}