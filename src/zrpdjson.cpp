#include <zpp/zrpdjson.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/reader.h>
#include <zit/base/trace.h>

using namespace z;
using namespace rapidjson;

int RJson::traceFlag = 0xff;
const char *RJson::traceTitle = "[RJson]";

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
  zdbgx(traceTitle, traceFlag, "RJson() val<%p>", val);
}

RJson::RJson(Value *value, Document::AllocatorType *alc){
  /*if(value){
    val = value;
    alloc = alc;
    needDel = false;
    }else */
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
    delete(val);
    zdbgx(traceTitle, traceFlag, "~RJson() delete val<%p>", val);
  }else{
    zdbgx(traceTitle, traceFlag, "~RJson()");
  }

}

int RJson::LoadFile(const char *file){
  int ret = ZPARAM_INVALID;
  FILE* fp = NULL;
  do{
    if(!file || !doc){
      break;
    }else{
      zmsgx(traceTitle, traceFlag, "LoadFile(%s)", file);
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
      zmsgx(traceTitle, traceFlag, "SaveFile(%s)", file);
    }

    fp = fopen(file, "w");//b"); // 非 Windows 平台使用 "r"
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
  zmsgx(traceTitle, traceFlag, "SaveString:\n%s", str.c_str());
  return ZOK;
}

int RJson::SetInt(const char *key, int32_t value){
  if(!key){
    ZERRC(ZPARAM_INVALID);
    return ZPARAM_INVALID;
  }

  return ZOK;
}

Document::AllocatorType *RJson::GetAllocator(){
  return alloc;
}
/*
int RJson::GetValue(const char *key, Value **value){
  if(!key){
    ZERRC(ZPARAM_INVALID);
    return ZPARAM_INVALID;
  }
  Value::MemberIterator itr = doc.FindMember(key);
  if (itr != doc.MemberEnd()){
    *value = &(itr->value);
    return ZOK;
  }
  zwarx(traceTitle, traceFlag, "Not Find key<%s>", key);
  return ZNOT_EXIST;
}

int RJson::GetArray(const char *key, Value **value){
  int ret = ZOK;

  ret = GetValue(key, value);
  if(ZOK == ret){
    if(!(*value)->IsArray()){
      ret = ZNOT_EXIST;
    }
  }
  return ret;
}

int RJson::SetValue(const char *key, const Value& value){
  if(!key){
    ZERRC(ZPARAM_INVALID);
    return ZPARAM_INVALID;
  }
  
  return ZOK;
}
*/
int RJson::GetInt(const char *key, int32_t &value){
  if(!key){
    ZERRC(ZPARAM_INVALID);
    return ZPARAM_INVALID;
  }
  
  return ZOK;
}

int RJson::SetUint(const char *key, uint32_t value){
  if(!key){
    ZERRC(ZPARAM_INVALID);
    return ZPARAM_INVALID;
  }
  
  return ZOK;
}

int RJson::GetUint(const char *key, uint32_t &value){
  if(!key){
    ZERRC(ZPARAM_INVALID);
    return ZPARAM_INVALID;
  }
  
  return ZOK;
}
int RJson::SetInt64(const char *key, int64_t value){
  if(!key){
    ZERRC(ZPARAM_INVALID);
    return ZPARAM_INVALID;
  }
  
  return ZOK;
}

int RJson::GetInt64(const char *key, int64_t &value){
  if(!key){
    ZERRC(ZPARAM_INVALID);
    return ZPARAM_INVALID;
  }
  
  return ZOK;
}

int RJson::SetUint64(const char *key, uint64_t value){
  if(!key){
    ZERRC(ZPARAM_INVALID);
    return ZPARAM_INVALID;
  }
  
  return ZOK;
}

int RJson::GetUint64(const char *key, uint64_t &value){
  if(!key){
    ZERRC(ZPARAM_INVALID);
    return ZPARAM_INVALID;
  }
  
  return ZOK;
}

int RJson::SetString(const char *key, const char *str){
  if(!key || !str){
    ZERRC(ZPARAM_INVALID);
    return ZPARAM_INVALID;
  }

  return ZOK;
}

int RJson::GetString(const char *key, std::string &str){
  if(!key){
    ZERRC(ZPARAM_INVALID);
    return ZPARAM_INVALID;
  }

  return ZOK;
}

int RJson::SetDouble(const char *key, double value){
  if(!key){
    ZERRC(ZPARAM_INVALID);
    return ZPARAM_INVALID;
  }

  return ZOK;
}
int RJson::GetDouble(const char *key, double &value){
  if(!key){
    ZERRC(ZPARAM_INVALID);
    return ZPARAM_INVALID;
  }
  
  return ZOK;
}

int RJson::SetBool(const char *key, bool value){
  if(!key){
    ZERRC(ZPARAM_INVALID);
    return ZPARAM_INVALID;
  }
  
  return ZOK;
}
int RJson::GetBool(const char *key, bool &value){
  if(!key){
    ZERRC(ZPARAM_INVALID);
    return ZPARAM_INVALID;
  }
  
  return ZOK;
}

void RJson::Dump(){
  if(!doc || !(traceFlag & ZTRACE_FLAG_INF)){
    return;
  }
  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);
  doc->Accept(writer);
  zinfx(traceTitle, traceFlag, "%s", buffer.GetString());
}
