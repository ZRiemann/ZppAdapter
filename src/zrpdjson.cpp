#include <zpp/zrpdjson.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/reader.h>
#include <zit/base/trace.h>

using namespace z;
using namespace rapidjson;

int RJson::traceFlag = 0xff;
const char *RJson::traceTitle = "[RJson]";

#define zdbg_rpd(fmt, ...) zdbgx(RJson::traceFlag, "[ln:%04d fn:%s]%s\t" fmt, __LINE__, __FUNCTION__,RJson::traceTitle, ##__VA_ARGS__)
#define zmsg_rpd(fmt, ...) zmsgx(RJson::traceFlag, "[ln:%04d fn:%s]%s\t" fmt, __LINE__, __FUNCTION__,RJson::traceTitle, ##__VA_ARGS__)
#define zwar_rpd(fmt, ...) zwarx(RJson::traceFlag, "[ln:%04d fn:%s]%s\t" fmt, __LINE__, __FUNCTION__,RJson::traceTitle, ##__VA_ARGS__)
#define zerr_rpd(fmt, ...) zerrx(RJson::traceFlag, "[ln:%04d fn:%s]%s\t" fmt, __LINE__, __FUNCTION__,RJson::traceTitle, ##__VA_ARGS__)
#define zinf_rpd(fmt, ...) zinfx(RJson::traceFlag, "[ln:%04d fn:%s]%s\t" fmt, __LINE__, __FUNCTION__,RJson::traceTitle, ##__VA_ARGS__)

RJson::RJson(){
  doc = new(std::nothrow) Document;
  
  if(!doc){
    ZERRC(ZMEM_INSUFFICIENT);
    val = NULL;
    needDel = false;
    alloc = NULL;
  }else{
    alloc = &doc->GetAllocator();
    val = doc;
    needDel = true;
  }
  zdbg_rpd("%s RJson() val<%p>", traceTitle, val);
}

RJson::RJson(Document::AllocatorType *alc){
  if(alc){
    // new Member
    val = new(std::nothrow) Value;
    if(!val){
      ZERRC(ZMEM_INSUFFICIENT);
      needDel = false;
      alloc = NULL;
    }else{
      alloc = alc;
      needDel = true;
    }
  }else{
    // 作为参数传给GetMember
    val = NULL;
    alloc = NULL;
    needDel = false;
  }
  doc = NULL;
}

RJson::~RJson(){
  if(needDel){
    if(doc){
      zdbg_rpd("~RJson() delete doc<%p>...", val);
      delete(doc);
      zdbg_rpd("~RJson() delete doc<%p> OK", val);
    }else{
      zdbg_rpd("~RJson() delete val<%p>...", val);
      delete(val);
      zdbg_rpd("~RJson() delete val<%p> OK", val);
    }
  }else{
    zdbg_rpd("~RJson()");
  }
}

int RJson::LoadFile(const char *file){
  int ret = ZPARAM_INVALID;
  FILE* fp = NULL;
  do{
    if(!file || !doc){
      break;
    }else{
      zmsg_rpd("LoadFile(%s)", file);
    }
    
    fp = fopen(file, "r");//b"); // 非 Windows 平台使用 "r"
    if(!fp){
      break;
    }
    char readBuffer[4096];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    doc->SetObject();
    doc->ParseStream(is);
    if(!doc->IsObject()){
      ZMSG("%s not json format file.", file);
      break;
    }else{
      Dump();
    }
    ret = ZOK;
  }while(0);
  
  if(fp){
    fclose(fp);
  }
  
  ZERRCX(ret);
  return ret;
}

int RJson::SaveFile(const char *file){
  int ret = ZPARAM_INVALID;
  FILE *fp = NULL;
  do{
    if(!file || !doc){
      break;
    }else{
      zmsg_rpd("SaveFile(%s)", file);
    }
#ifdef ZSYS_WINDOWS
    fp = fopen(file, "wb");
#else
    fp = fopen(file, "w");// 非 Windows 平台使用 "w"
#endif
    if(!fp){
      break;
    }else{
      Dump();
    }

    char writeBuffer[4096];
    FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
    Writer<FileWriteStream> writer(os);
    doc->Accept(writer);
    ret = ZOK;
  }while(0);
  
  if(fp){
    fclose(fp);
  }
  ZERRCX(ret);
  return ret;
}

int RJson::LoadString(const char *json){
  int ret = ZFAIL;
  do{
    if(!json || !doc){
      ret = ZPARAM_INVALID;
      break;
    }

    doc->Parse(json);
    if(!doc->IsObject()){
      ret = ZNOT_SUPPORT;
      break;
    }else{
      Dump();
    }
    ret = ZOK;
  }while(0);

  ZERRC(ret);
  return ret;
}

int RJson::SaveString(std::string &str){
  if(!doc){
    return ZPARAM_INVALID;
  }
  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);
  doc->Accept(writer);
  str = buffer.GetString();
  zdbg_rpd("SaveString:\n%s", str.c_str());
  return ZOK;
}


Document::AllocatorType *RJson::GetAllocator(){
  return alloc;
}

/*
Value("copy", document.GetAllocator()).Move()
*/
int RJson::SetInt(const char *key, int32_t value){
  if(!key){
    if(val){
      val->SetInt(value);
      return ZOK;
    }
    return ZPARAM_INVALID;
  }

  Value::MemberIterator itr = val->FindMember(key);
  if (itr != val->MemberEnd()){
    if(itr->value.IsInt()){
      itr->value.SetInt(value);
    }else{
      return ZCAST_FAIL;
    }
  }else{
    val->AddMember(Value(key, *alloc).Move(), value, *alloc);
  }
  return ZOK;
}

int RJson::GetInt(const char *key, int32_t &value){
  if(!key){
    // just get value, has no key
    if(val->IsInt()){
      value = val->GetInt();
      return ZOK;
    }else{
      return ZPARAM_INVALID;
    }
  }
  // get value by key.
  Value::ConstMemberIterator itr = val->FindMember(key);
  if (itr != val->MemberEnd() && itr->value.IsInt()){
    value = itr->value.GetInt();
    return ZOK;
  }
  ZERRC(ZNOT_EXIST);
  return ZNOT_EXIST;
}

int RJson::SetUint(const char *key, uint32_t value){
  if(!key){
    if(val){
      val->SetUint(value);
      return ZOK;
    }
    return ZPARAM_INVALID;
  }

  Value::MemberIterator itr = val->FindMember(key);
  if (itr != val->MemberEnd()){
    if(itr->value.IsUint()){
      itr->value.SetUint(value);
    }else{
      return ZCAST_FAIL;
    }
  }else{
    val->AddMember(Value(key, *alloc).Move(), value, *alloc);
  }
  return ZOK;
}

int RJson::GetUint(const char *key, uint32_t &value){
  if(!key){
    // just get value, has no key
    if(val->IsUint()){
      value = val->GetUint();
    }else{
      return ZPARAM_INVALID;
    }
  }

  Value::ConstMemberIterator itr = val->FindMember(key);
  if (itr != val->MemberEnd() && itr->value.IsUint()){
    value = itr->value.GetUint();
    return ZOK;
  }
  ZERRC(ZNOT_EXIST);
  return ZNOT_EXIST;
}

int RJson::SetInt64(const char *key, int64_t value){
  if(!key){
    if(val){
      val->SetInt64(value);
      return ZOK;
    }
    return ZPARAM_INVALID;
  }

  Value::MemberIterator itr = val->FindMember(key);
  if (itr != val->MemberEnd()){
    if(itr->value.IsInt64()){
      itr->value.SetInt64(value);
    }else{
      return ZCAST_FAIL;
    }
  }else{
    val->AddMember(Value(key, *alloc).Move(), value, *alloc);
  }
  return ZOK;
}

int RJson::GetInt64(const char *key, int64_t &value){
  if(!key){
    // just get value, has no key
    if(val->IsInt64()){
      value = val->GetInt64();
    }else{
      return ZPARAM_INVALID;
    }
  }

  Value::ConstMemberIterator itr = val->FindMember(key);
  if (itr != val->MemberEnd() && itr->value.IsInt64()){
    value = itr->value.GetInt64();
    return ZOK;
  }
  ZERRC(ZNOT_EXIST);
  return ZNOT_EXIST;
}

int RJson::SetUint64(const char *key, uint64_t value){
  if(!key){
    if(val){
      val->SetUint64(value);
      return ZOK;
    }
    return ZPARAM_INVALID;
  }

  Value::MemberIterator itr = val->FindMember(key);
  if (itr != val->MemberEnd()){
    if(itr->value.IsUint64()){
      itr->value.SetUint64(value);
    }else{
      return ZCAST_FAIL;
    }
  }else{
    val->AddMember(Value(key, *alloc).Move(), value, *alloc);
  }
  return ZOK;
}

int RJson::GetUint64(const char *key, uint64_t &value){
  if(!key){
    // just get value, has no key
    if(val->IsUint64()){
      value = val->GetUint64();
    }else{
      return ZPARAM_INVALID;
    }
  }

  Value::ConstMemberIterator itr = val->FindMember(key);
  if (itr != val->MemberEnd() && itr->value.IsUint64()){
    value = itr->value.GetUint64();
    return ZOK;
  }
  ZERRC(ZNOT_EXIST);
  return ZNOT_EXIST;
}

int RJson::SetString(const char *key, const char *str){
  if(!alloc){
    return ZPARAM_INVALID;
  }
  
  if(!key){
    if(val){
      val->SetString(str, *alloc);
      return ZOK;
    }
    return ZPARAM_INVALID;
  }

  Value::MemberIterator itr = val->FindMember(key);
  if (itr != val->MemberEnd()){
    if(itr->value.IsString()){
      itr->value.SetString(str, *alloc);
      //val->RemoveMember(itr);
      //val->AddMember(Value(key, *alloc).Move(), Value(str, *alloc).Move(), *alloc);
    }else{
      return ZCAST_FAIL;
    }
  }else{
    val->AddMember(Value(key, *alloc).Move(), Value(str, *alloc).Move(), *alloc);
  }
  return ZOK;
}

int RJson::GetString(const char *key, std::string &str){
  if(!key){
    // just get value, has no key
    if(val->IsString()){
      str = val->GetString();
    }else{
      return ZPARAM_INVALID;
    }
  }

  Value::ConstMemberIterator itr = val->FindMember(key);
  if (itr != val->MemberEnd() && itr->value.IsString()){
    str = itr->value.GetString();
    return ZOK;
  }
  ZERRC(ZNOT_EXIST);
  return ZNOT_EXIST;
}

int RJson::SetDouble(const char *key, double value){
  if(!key){
    if(val){
      val->SetDouble(value);
      return ZOK;
    }
    return ZPARAM_INVALID;
  }

  Value::MemberIterator itr = val->FindMember(key);
  if (itr != val->MemberEnd()){
    if(itr->value.IsDouble()){
      itr->value.SetDouble(value);
    }else{
      return ZCAST_FAIL;
    }
  }else{
    val->AddMember(Value(key, *alloc).Move(), value, *alloc);
  }
  return ZOK;
}

int RJson::GetDouble(const char *key, double &value){
  if(!key){
    // just get value, has no key
    if(val->IsDouble()){
      value = val->GetDouble();
    }else{
      return ZPARAM_INVALID;
    }
  }

  Value::ConstMemberIterator itr = val->FindMember(key);
  if (itr != val->MemberEnd() && itr->value.IsDouble()){
    value = itr->value.GetDouble();
    return ZOK;
  }
  ZERRC(ZNOT_EXIST);
  return ZNOT_EXIST;
}

int RJson::SetBool(const char *key, bool value){
  if(!key){
    if(val){
      val->SetBool(value);
      return ZOK;
    }
    return ZPARAM_INVALID;
  }

  Value::MemberIterator itr = val->FindMember(key);
  if (itr != val->MemberEnd()){
    if(itr->value.IsBool()){
      itr->value.SetBool(value);
    }else{
      return ZCAST_FAIL;
    }
  }else{
    val->AddMember(Value(key, *alloc).Move(), value, *alloc);
  }
  return ZOK;
}

int RJson::GetBool(const char *key, bool &value){
  if(!key){
    // just get value, has no key
    if(val->IsBool()){
      value = val->GetBool();
    }else{
      return ZPARAM_INVALID;
    }
  }


  Value::ConstMemberIterator itr = val->FindMember(key);
  if (itr != val->MemberEnd() && itr->value.IsBool()){
    value = itr->value.GetBool();
    return ZOK;
  }
  ZERRC(ZNOT_EXIST);
  return ZNOT_EXIST;
}

void RJson::Dump(){
  if(!val || !(traceFlag & ZTRACE_FLAG_INF)){
    return;
  }
  StringBuffer buffer;
  //Writer<StringBuffer> writer(buffer);
  PrettyWriter<StringBuffer> writer(buffer);
  val->Accept(writer);
  zinf_rpd("\n%s", buffer.GetString());
}

int RJson::GetMember(const char *key, RJson *rj){
  if(!key){
    ZERRC(ZPARAM_INVALID);
    return ZPARAM_INVALID;
  }
  
  Value::MemberIterator itr = val->FindMember(key);
  if (itr != val->MemberEnd()){
    rj->val = &(itr->value);
    return ZOK;
  }
  zerr_rpd("%s %s", key, zstrerr(ZNOT_EXIST));
  return ZNOT_EXIST;
}

int RJson::AddMember(const char *key, RJson *js){
  if(!key || !alloc){
    ZERRC(ZPARAM_INVALID);
    return ZPARAM_INVALID;
  }

  Value::MemberIterator itr = val->FindMember(key);
  if (itr != val->MemberEnd()){
    if(itr->value.IsObject() || itr->value.IsArray()){
      val->EraseMember(itr);
      val->AddMember(Value(key, *alloc).Move(), js->val->Move(), *alloc);
    }else{
      return ZCAST_FAIL;
    }
  }else{
    val->AddMember(Value(key, *alloc).Move(), js->val->Move(), *alloc);
  }
  return ZOK;
}

bool RJson::DelMember(const char *key){
  return val->RemoveMember(key);
}

void RJson::ForEach(HandleEach fun, void *hint){
  if(!val || !val->IsArray()){
    return;
  }

  RJson js(NULL);
  SizeType size = val->Size();
  for(SizeType idx = 0; idx<size; idx++){
    js.SetValue((*val)[idx]);
    (*fun)(&js, hint);
  }
}
