#include "resource.h"
#include <zit/base/trace.h>
#include <fstream>
#include "../../zpp/pb2json.h"
#include <stdlib.h>

Resource::Resource()
    :type(0)
    ,id(0)
    ,flag(0){
    ZDBGC(ZOK);
}

Resource::~Resource(){
    ZDBGC(ZOK);
}

int Resource::Operate(void *in, void **out, void*hint){
    ZDBG("Resource<type:%d id:%d :flag:%d", type, id, flag);
    return ZOK;
}

int Resource::Config(int type, int id, int flag){
    this->type = type;
    this->id = id;
    this->flag = flag;
    return ZOK;
}

void Resource::Dump(){
    ZDBG("\n%s\n", msg.DebugString().c_str());
}

int Resource::LoadFile(const char *file){
    std::fstream input(file, std::ios::in | std::ios::binary);
    if(!msg.ParseFromIstream(&input)){
        ZERR("LOAD FAIL");
    }else{
        Dump();
    }
    return ZOK;
}

void Resource::TstNew2Old(){
    char *buf1 = NULL;
    size_t len = pmsg->ByteSizeLong();
    char *buf = (char*)malloc(len); if(!buf)return;
    pmsg->SerializeToArray(buf, len);
    uns::Msg oldMsg;
    if(!oldMsg.ParseFromArray(buf, len)){
        ZDBG("plg msg to buf fail. plg msg.src(%d)", pmsg->src());
        ZDBG("plg msg dump: %s", pmsg->DebugString().c_str());
        return;
    }
    ZDBG("New to old msg: len:%d\n%s\n", len, oldMsg.DebugString().c_str());
    ZDBG("oldSrc: %d", oldMsg.src());
    size_t len1 = oldMsg.ByteSizeLong();
    if(len1 == len){
        buf1 = (char*)malloc(len1); if(!buf1)return; // just test , ignore memory leak
        bool checkOk = true;
        oldMsg.SerializeToArray(buf1, len1);
        for(size_t i=0; i<len; ++i){
            if(buf[i] != buf1[i]){
                checkOk = false;
                break;
            }
        }
        ZDBG("oldMsg Array is (%s) to newMsg.", checkOk ? "EQUAL" : "NOT EQUAL");
    }
    free(buf);
    free(buf1);
}

/*
int Resource::Swap(uns::Msg &_msg){
    //msg.GetReflection()->Swap(&msg, &_msg);
    _msg.GetReflection()->Swap(&msg, &_msg);
    return ZOK;
}
*/

void Resource::ToJson(){
    z::json::RJson js;
    z::json::Pb2json(&js, &msg);
    js.Dump();
}

void Resource::ToJsonP(){
    z::json::RJson js;
    z::json::Pb2json(&js, pmsg);
    js.Dump();
}

void Resource::DumpP(){
    ZDBG("\n%s\n", pmsg->DebugString().c_str());
}
