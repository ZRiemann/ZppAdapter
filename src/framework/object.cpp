#include <zpp/framework/object.h>
#include <zit/base/trace.h>
#include <zpp/framework/mapfind.hpp>

#define ZTRACE_OBJECT 1
namespace z{
const int DEV_STAT_MASK = 0x00000003;
const int DEV_STAT_FINI = 0; // 未初始化状态
const int DEV_STAT_INIT = 1; // 初始化状态
const int DEV_STAT_RUN = 2; // 运行状态
const int DEV_STAT_EXCP = 3; // 异常状态
}

using namespace z;

Archive::Archive()
  :isLoad(1){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
}

Archive::~Archive(){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
}

int Archive::GetData(ZOP_ARG){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
  return ZOK;
}
otype_t Archive::GetType(){
  return 0;
}
//(Archive *ar); // in=0,hint=type; in=Archive*,hint=0:out(to meta)/1:in(build obj)
int Object::CreateObj(ZOP_ARG){
  int ret = ZNOT_EXIST;
  zoperate dc = 0;
  Archive *ar = (Archive*)in;
  otype_t type= 0;
  if(ar){
    type = ar->GetType();
  }else{
    type = *((otype_t*)hint);
  }

  if(MapFind(*pdyn_map, type, dc)){
    ret  = dc(in, out, hint);
  }else{
    ZWAR("Object type<%08x> not registed!", type);
  }
  return ret;
}
std::map<otype_t, zoperate>* Object::pdyn_map;

#if ZUSE_OBJ_CNT
uint32_t Object::obj_cnt;
#endif

Object::Object()
  :type(0){
#if ZTRACE_OBJECT
#if ZUSE_OBJ_CNT
  ZDBG("obj<%p> total:%d>", this, zatm_add(&obj_cnt, 1));
#else
  ZDBG("obj<%p>", this);
#endif
#endif
}

Object::~Object(){
#if ZTRACE_OBJECT
#if ZUSE_OBJ_CNT
  ZDBG("obj<%p> total:%d>", this, zatm_sub(&obj_cnt, 1));
#else
  ZDBG("obj<%p>", this);
#endif
#endif
}

int Object::Serialize(ZOP_ARG){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
  return ZOK;
}

int Object::Clone(ZOP_ARG){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
  return ZOK;
}

int Object::Release(ZOP_ARG){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
  return ZOK;
}

int Object::Operate(ZOP_ARG){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
  return ZOK;
}
/*
int Object::GetArchive(ZOP_ARG){
  if(out){
    *out = NULL;
  }
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
  return ZOK;
}
int Object::ReleaseArchive(ZOP_ARG){
  if(out && *out){
    delete ((Archive*)*out);
    *out = NULL;
  }
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
  return ZOK;
}
*/
//ZIMPLEMENT_OBJ(Resource, z::Resource);
Resource::Resource()
  :id(0)
  ,state(0){
  type = 1;
}

Resource::~Resource(){
}
/*
int Resource::Serialize(ZOP_ARG){
  // pb point
  int ret = ZOK;
  if(!hint || !in){// || !out){
    ZERRCXR(ZPARAM_INVALID);
  }
  //Archive *ar = (Archive*)in;
  if(ARC_IS_LOAD(*((int*)hint))){
    // load from archive
    ZDBG("zpp task NOW...");
  }else{
    // serial to archive
    
  }
  return ret;
}
*/
Device::Device(){
  type = 2;
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
}

Device::~Device(){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
}

int Device::Init(ZOP_ARG){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
  return ZOK;
}

int Device::Fini(ZOP_ARG){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
  return ZOK;
}

int Device::Run(ZOP_ARG){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
  return ZOK;
}

int Device::Stop(ZOP_ARG){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
  return ZOK;
}

