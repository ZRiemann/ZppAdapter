#include "tstframework.h"
void tstframework(int argc, char **argv){

}

#if 0
#include <zpp/framework/object.h>
#include <zpp/framework/unitsys_defs.h>
#include <zit/base/trace.h>
#include <string.h>

void tstbase(int argc, char **argv);
void tstframework(int argc, char **argv){
  zdbg("usage:");
  zdbg("zpp_test frw_base");
  if(argc ==2 && strcmp("frw_base", argv[1]) == 0){
    tstbase(argc, argv);
  }
}

void tstbase(int argc, char **argv){
  z::Object obj;
  obj.Serialize(ZOP_NULL);
  obj.Clone(ZOP_NULL);
  //obj.Release(ZOP_NULL);
  obj.Operate(ZOP_NULL);

  z::Device dev;
  dev.Serialize(ZOP_NULL);
  dev.Clone(ZOP_NULL);
  //dev.Release(ZOP_NULL);
  dev.Operate(ZOP_NULL);

  dev.Init(ZOP_NULL);
  dev.Fini(ZOP_NULL);
  dev.Run(ZOP_NULL);
  dev.Stop(ZOP_NULL);

  uns::CMS *cms = uns::CMS::Instance();
  cms->Serialize(ZOP_NULL);
  cms->Clone(ZOP_NULL);
  //cms->Release(ZOP_NULL);
  cms->Operate(ZOP_NULL);

  cms->Init(ZOP_NULL);
  cms->Fini(ZOP_NULL);
  cms->Run(ZOP_NULL);
  cms->Stop(ZOP_NULL);

  z::Device *pdev = dynamic_cast<z::Device*>(cms);
  z::Resource *pres = dynamic_cast<z::Resource*>(cms);
  z::Object *pobj = dynamic_cast<z::Object*>(cms);
  zdbg("CMS<cms:%p dev:%p res:%p obj:%p",cms, pdev, pres, pobj);
  uns::CMS::Uninstance();
}
#endif
