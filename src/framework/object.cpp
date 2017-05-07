#include <zpp/framework/object.h>
#include <zit/base/trace.h>

using namespace z;

#define ZTRACE_OBJECT 1
Archive::Archive(){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
}

Archive::~Archive(){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
}

int Archive::GetData(zvalue_t in, zvalue_t *out, zvalue_t hint){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
  return ZOK;
}

Object::Object(){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
}

Object::~Object(){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
}

int Object::Serialize(zvalue_t in, zvalue_t *out, zvalue_t hint){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
  return ZOK;
}

int Object::Clone(zvalue_t in, zvalue_t *out, zvalue_t hint){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
  return ZOK;
}

int Object::Release(zvalue_t in, zvalue_t *out, zvalue_t hint){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
  return ZOK;
}

int Object::Operate(zvalue_t in, zvalue_t *out, zvalue_t hint){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
  return ZOK;
}

Device::Device(){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
}

Device::~Device(){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
}

int Device::Init(zvalue_t in, zvalue_t *out, zvalue_t hint){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
  return ZOK;
}

int Device::Fini(zvalue_t in, zvalue_t *out, zvalue_t hint){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
  return ZOK;
}

int Device::Run(zvalue_t in, zvalue_t *out, zvalue_t hint){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
  return ZOK;
}

int Device::Stop(zvalue_t in, zvalue_t *out, zvalue_t hint){
#if ZTRACE_OBJECT
  ZERRC(ZOK);
#endif
  return ZOK;
}
