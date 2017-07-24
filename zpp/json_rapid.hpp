#ifndef _ZPP_JSON_RAPID_HPP_
#define _ZPP_JSON_RAPID_HPP_
/**@file zpp/json_rapid.hpp
 * @brief single file wrapper for rapidjson
 * @note
 *
 */

// RJson interface definitons
#include <rapidjson/document.h>
namespace z{
    namespace json{
        class RJson{
        public:
            RJson();
            RJson(rapidjson::Document::AllocatorType *alloc); // Value alloc==NULL
            ~RJson();

            // 加载与保存
            int LoadFile(const char *file);
            int SaveFile(const char *file);

            int LoadString(const char *json);
            int SaveString(std::string &str);

            void Swap(RJson *js);
            // 设置与获取
            rapidjson::Document::AllocatorType *GetAllocator();

            int GetMember(const char *key, RJson *js);
            int AddMember(const char *key, RJson *js);
            bool DelMember(const char *key);
            // key=NULL, return value; key!=NULL return key value;
            int SetInt(const char *key, int32_t value);
            int GetInt(const char *key, int32_t &value);
            int SetUint(const char *key, uint32_t value);
            int GetUint(const char *key, uint32_t &value);
            int SetInt64(const char *key, int64_t value);
            int GetInt64(const char *key, int64_t &value);
            int SetUint64(const char *key, uint64_t value);
            int GetUint64(const char *key, uint64_t &value);
            int SetString(const char *key, const char *str);
            int GetString(const char *key, std::string &str);
            int SetDouble(const char *key, double value);
            int GetDouble(const char *key, double &value);
            int SetBool(const char *key, bool value);
            int GetBool(const char *key, bool &value);

            //valType:{ "Null", "False", "True", "Object", "Array", "String", "Number" };
            enum ValueType{
                valNull = 0,
                valFalse,
                valTrue,
                valObject,
                valArray,
                valString,
                valNumber,
                statArrayBegin,
                statArrayEnd,
                statObjectBegin,
                statObjectEnd
            };
            ValueType GetType(){return (ValueType)val->GetType();}
            const char *GetCString(){return val->GetString();} // must string value;
            bool IsArray(){return val->IsArray();}
            bool IsObject(){return val->IsObject();}
            bool IsString(){return val->IsString();}
            bool IsNull(){return val->IsNull();}
            bool IsNumber(){return val->IsNumber();}
            bool IsUint(){return val->IsUint();}
            bool IsInt(){return val->IsInt();}
            bool IsUint64(){return val->IsUint64();}
            bool IsInt64(){return val->IsInt64();}
            bool IsDouble(){return val->IsDouble();}
            // Array ; Value mast a array
            void SetArray(){val->SetArray();}
            void SetObject(){val->SetObject();}
            template< typename T >
            int PushBack(T &t){
                val->PushBack(t, *alloc);
                return 0;
            }
            int PopBack(){
                val->PopBack();
                return 0;
            }
            void SetValue(rapidjson::Value &v){val = &v;}
            void SetAllocator(rapidjson::Document::AllocatorType *alc){alloc = alc;}

            typedef int (*CallbackEach)(ValueType vType, RJson &name, RJson &value, void *hint);
            int ForEach(CallbackEach fun, void *hint);
            int EachObject(CallbackEach fun, void *hint);
            int EachArray(CallbackEach fun, void *hint);
        public:
            void Dump();
            std::string &ToString(std::string &str, bool isPretty = false);

        public:
            bool needDel;
            rapidjson::Value *val;
            rapidjson::Document *doc;
            rapidjson::Document::AllocatorType *alloc;
        };
    }
}

// RJson implements

#include <zit/base/trace.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/reader.h>

#define ZTRACE_RJSON 1 // enable or disable RJson

#if ZTRACE_RJSON
#define zdbg_rpd(fmt, ...) zdbgx(g_ztrace_flag, "[ln:%04d fn:%s]%s\t" fmt, __LINE__, __FUNCTION__,"[RJson]", ##__VA_ARGS__)
#define zmsg_rpd(fmt, ...) zmsgx(g_ztrace_flag, "[ln:%04d fn:%s]%s\t" fmt, __LINE__, __FUNCTION__,"[RJson]", ##__VA_ARGS__)
#define zwar_rpd(fmt, ...) zwarx(g_ztrace_flag, "[ln:%04d fn:%s]%s\t" fmt, __LINE__, __FUNCTION__,"[RJson]", ##__VA_ARGS__)
#define zerr_rpd(fmt, ...) zerrx(g_ztrace_flag, "[ln:%04d fn:%s]%s\t" fmt, __LINE__, __FUNCTION__,"[RJson]", ##__VA_ARGS__)
#define zinf_rpd(fmt, ...) zinfx(g_ztrace_flag, "[ln:%04d fn:%s]%s\t" fmt, __LINE__, __FUNCTION__,"[RJson]", ##__VA_ARGS__)
#else
#define zdbg_rpd(fmt, ...)
#define zmsg_rpd(fmt, ...)
#define zwar_rpd(fmt, ...)
#define zerr_rpd(fmt, ...)
#define zinf_rpd(fmt, ...)
#endif

inline z::json::RJson::RJson(){
    doc = new(std::nothrow) rapidjson::Document;
    if(!doc){
        // ZERRC(ZMEM_INSUFFICIENT);
        val = NULL;
        needDel = false;
        alloc = NULL;
    }else{
        alloc = &doc->GetAllocator();
        val = doc;
        needDel = true;
    }
    //zdbg_rpd("RJson() val<%p>", val);
}

inline z::json::RJson::RJson(rapidjson::Document::AllocatorType *alc){
    if(alc){
        // new Member
        val = new(std::nothrow) rapidjson::Value;
        if(!val){
            //ZERRC(ZMEM_INSUFFICIENT);
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
    //zdbg_rpd("RJson(alc<%p>) val<%p>", alc, val);
}

inline z::json::RJson::~RJson(){
    if(needDel){
        doc ? delete(doc) : delete(val);
    }
    //zdbg_rpd("~RJson() val<%p>", val);
}


inline void z::json::RJson::Swap(z::json::RJson *js){
    val->Swap(*js->val);
}

inline int z::json::RJson::LoadFile(const char *file){
    int ret = ZPARAM_INVALID;
    FILE* fp = NULL;
    do{
        if(!file || !doc){
            break;
        }//else{
        //    zmsg_rpd("LoadFile(%s)", file);
        //}
#if ZSYS_WINDOWS
        fp = fopen(file, "rb");
#else
        fp = fopen(file, "r");//b"); // 非 Windows 平台使用 "r"
#endif
        if(!fp){
            break;
        }
        char readBuffer[4096];
        rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
        doc->SetObject();
        doc->ParseStream(is);
        if(!doc->IsObject()){
            break;
        }else{
            Dump();
        }
        ret = ZOK;
    }while(0);
  
    if(fp){
        fclose(fp);
    }
  
    //ZERRCX(ret);
    return ret;
}

inline int z::json::RJson::SaveFile(const char *file){
    int ret = ZPARAM_INVALID;
    FILE *fp = NULL;
    do{
        if(!file || !doc){
            break;
        }//else{
        //    zmsg_rpd("SaveFile(%s)", file);
        //}
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
        rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
        rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
        doc->Accept(writer);
        ret = ZOK;
    }while(0);
  
    if(fp){
        fclose(fp);
    }
    //ZERRCX(ret);
    return ret;
}

inline int z::json::RJson::LoadString(const char *json){
    int ret = ZFAIL;
    do{
        if(!json || !doc){
            ret = ZPARAM_INVALID;
            break;
        }

        doc->Parse(json);
        if(!doc->IsObject()){
            ret = ZPARAM_INVALID;
            break;
        }else{
            Dump();
        }
        ret = ZOK;
    }while(0);

    //ZERRC(ret);
    return ret;
}

inline int z::json::RJson::SaveString(std::string &str){
    if(!doc){
        return ZPARAM_INVALID;
    }
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc->Accept(writer);
    str = buffer.GetString();
    return ZOK;
}


inline rapidjson::Document::AllocatorType *z::json::RJson::GetAllocator(){
    return alloc;
}

inline void z::json::RJson::Dump(){
    if(!val || !(g_ztrace_flag & ZTRACE_FLAG_INF)){
        return;
    }
    rapidjson::StringBuffer buffer;
    //Writer<StringBuffer> writer(buffer);
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    val->Accept(writer);
    zinf_rpd("\n%s", buffer.GetString());
}
inline std::string &z::json::RJson::ToString(std::string &str, bool isPretty){
    if(!val || !(g_ztrace_flag & ZTRACE_FLAG_INF)){
        str.clear();
        return str;
    }
    rapidjson::StringBuffer buffer;
    if(isPretty){
        rapidjson::PrettyWriter<rapidjson::StringBuffer>writer(buffer);
        val->Accept(writer);
    }else{
        rapidjson::Writer<rapidjson::StringBuffer>writer(buffer);
        val->Accept(writer);
    }
    str = buffer.GetString();
    return str;
}
inline int z::json::RJson::SetInt(const char *key, int32_t value){
    if(!key){
        if(val){
            val->SetInt(value);
            return ZOK;
        }
        return ZPARAM_INVALID;
    }

    rapidjson::Value::MemberIterator itr = val->FindMember(key);
    if (itr != val->MemberEnd()){
        if(itr->value.IsInt()){
            itr->value.SetInt(value);
        }else{
            return ZCAST_FAIL;
        }
    }else{
        val->AddMember(rapidjson::Value(key, *alloc).Move(), value, *alloc);
    }
    return ZOK;
}

inline int z::json::RJson::GetInt(const char *key, int32_t &value){
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
    rapidjson::Value::ConstMemberIterator itr = val->FindMember(key);
    if (itr != val->MemberEnd() && itr->value.IsInt()){
        value = itr->value.GetInt();
        return ZOK;
    }
    //ZERRC(ZNOT_EXIST);
    return ZNOT_EXIST;
}

inline int z::json::RJson::SetUint(const char *key, uint32_t value){
    if(!key){
        if(val){
            val->SetUint(value);
            return ZOK;
        }
        return ZPARAM_INVALID;
    }

    rapidjson::Value::MemberIterator itr = val->FindMember(key);
    if (itr != val->MemberEnd()){
        if(itr->value.IsUint()){
            itr->value.SetUint(value);
        }else{
            return ZCAST_FAIL;
        }
    }else{
        val->AddMember(rapidjson::Value(key, *alloc).Move(), value, *alloc);
    }
    return ZOK;
}

inline int z::json::RJson::GetUint(const char *key, uint32_t &value){
    if(!key){
        // just get value, has no key
        if(val->IsUint()){
            value = val->GetUint();
        }else{
            return ZPARAM_INVALID;
        }
    }

    rapidjson::Value::ConstMemberIterator itr = val->FindMember(key);
    if (itr != val->MemberEnd() && itr->value.IsUint()){
        value = itr->value.GetUint();
        return ZOK;
    }
    //ZERRC(ZNOT_EXIST);
    return ZNOT_EXIST;
}

inline int z::json::RJson::SetInt64(const char *key, int64_t value){
    if(!key){
        if(val){
            val->SetInt64(value);
            return ZOK;
        }
        return ZPARAM_INVALID;
    }

    rapidjson::Value::MemberIterator itr = val->FindMember(key);
    if (itr != val->MemberEnd()){
        if(itr->value.IsInt64()){
            itr->value.SetInt64(value);
        }else{
            return ZCAST_FAIL;
        }
    }else{
        val->AddMember(rapidjson::Value(key, *alloc).Move(), value, *alloc);
    }
    return ZOK;
}

inline int z::json::RJson::GetInt64(const char *key, int64_t &value){
    if(!key){
        // just get value, has no key
        if(val->IsInt64()){
            value = val->GetInt64();
        }else{
            return ZPARAM_INVALID;
        }
    }

    rapidjson::Value::ConstMemberIterator itr = val->FindMember(key);
    if (itr != val->MemberEnd() && itr->value.IsInt64()){
        value = itr->value.GetInt64();
        return ZOK;
    }
    //ZERRC(ZNOT_EXIST);
    return ZNOT_EXIST;
}

inline int z::json::RJson::SetUint64(const char *key, uint64_t value){
    if(!key){
        if(val){
            val->SetUint64(value);
            return ZOK;
        }
        return ZPARAM_INVALID;
    }

    rapidjson::Value::MemberIterator itr = val->FindMember(key);
    if (itr != val->MemberEnd()){
        if(itr->value.IsUint64()){
            itr->value.SetUint64(value);
        }else{
            return ZCAST_FAIL;
        }
    }else{
        val->AddMember(rapidjson::Value(key, *alloc).Move(), value, *alloc);
    }
    return ZOK;
}

inline int z::json::RJson::GetUint64(const char *key, uint64_t &value){
    if(!key){
        // just get value, has no key
        if(val->IsUint64()){
            value = val->GetUint64();
        }else{
            return ZPARAM_INVALID;
        }
    }

    rapidjson::Value::ConstMemberIterator itr = val->FindMember(key);
    if (itr != val->MemberEnd() && itr->value.IsUint64()){
        value = itr->value.GetUint64();
        return ZOK;
    }
    //ZERRC(ZNOT_EXIST);
    return ZNOT_EXIST;
}

inline int z::json::RJson::SetString(const char *key, const char *str){
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

    rapidjson::Value::MemberIterator itr = val->FindMember(key);
    if (itr != val->MemberEnd()){
        if(itr->value.IsString()){
            itr->value.SetString(str, *alloc);
            //val->RemoveMember(itr);
            //val->AddMember(rapidjson::Value(key, *alloc).Move(), rapidjson::Value(str, *alloc).Move(), *alloc);
        }else{
            return ZCAST_FAIL;
        }
    }else{
        val->AddMember(rapidjson::Value(key, *alloc).Move(), rapidjson::Value(str, *alloc).Move(), *alloc);
    }
    return ZOK;
}

inline int z::json::RJson::GetString(const char *key, std::string &str){
    if(!key){
        // just get value, has no key
        if(val->IsString()){
            str = val->GetString();
        }else{
            return ZPARAM_INVALID;
        }
    }

    rapidjson::Value::ConstMemberIterator itr = val->FindMember(key);
    if (itr != val->MemberEnd() && itr->value.IsString()){
        str = itr->value.GetString();
        return ZOK;
    }
    //ZERRC(ZNOT_EXIST);
    return ZNOT_EXIST;
}

inline int z::json::RJson::SetDouble(const char *key, double value){
    if(!key){
        if(val){
            val->SetDouble(value);
            return ZOK;
        }
        return ZPARAM_INVALID;
    }

    rapidjson::Value::MemberIterator itr = val->FindMember(key);
    if (itr != val->MemberEnd()){
        if(itr->value.IsDouble()){
            itr->value.SetDouble(value);
        }else{
            return ZCAST_FAIL;
        }
    }else{
        val->AddMember(rapidjson::Value(key, *alloc).Move(), value, *alloc);
    }
    return ZOK;
}

inline int z::json::RJson::GetDouble(const char *key, double &value){
    if(!key){
        // just get value, has no key
        if(val->IsDouble()){
            value = val->GetDouble();
        }else{
            return ZPARAM_INVALID;
        }
    }

    rapidjson::Value::ConstMemberIterator itr = val->FindMember(key);
    if (itr != val->MemberEnd() && itr->value.IsDouble()){
        value = itr->value.GetDouble();
        return ZOK;
    }
    //ZERRC(ZNOT_EXIST);
    return ZNOT_EXIST;
}

inline int z::json::RJson::SetBool(const char *key, bool value){
    if(!key){
        if(val){
            val->SetBool(value);
            return ZOK;
        }
        return ZPARAM_INVALID;
    }

    rapidjson::Value::MemberIterator itr = val->FindMember(key);
    if (itr != val->MemberEnd()){
        if(itr->value.IsBool()){
            itr->value.SetBool(value);
        }else{
            return ZCAST_FAIL;
        }
    }else{
        val->AddMember(rapidjson::Value(key, *alloc).Move(), value, *alloc);
    }
    return ZOK;
}

inline int z::json::RJson::GetBool(const char *key, bool &value){
    if(!key){
        // just get value, has no key
        if(val->IsBool()){
            value = val->GetBool();
        }else{
            return ZPARAM_INVALID;
        }
    }


    rapidjson::Value::ConstMemberIterator itr = val->FindMember(key);
    if (itr != val->MemberEnd() && itr->value.IsBool()){
        value = itr->value.GetBool();
        return ZOK;
    }
    //ZERRC(ZNOT_EXIST);
    return ZNOT_EXIST;
}

inline int z::json::RJson::GetMember(const char *key, RJson *rj){
    if(!key){
        //ZERRC(ZPARAM_INVALID);
        return ZPARAM_INVALID;
    }

    rapidjson::Value::MemberIterator itr = val->FindMember(key);
    if (itr != val->MemberEnd()){
        rj->val = &(itr->value);
        return ZOK;
    }
    //zerr_rpd("%s %s", key, zstrerr(ZNOT_EXIST));
    return ZNOT_EXIST;
}

inline int z::json::RJson::AddMember(const char *key, RJson *js){
    if(!key || !alloc){
        ZERRC(ZPARAM_INVALID);
        return ZPARAM_INVALID;
    }

    rapidjson::Value::MemberIterator itr = val->FindMember(key);
    if (itr != val->MemberEnd()){
        if(itr->value.IsObject() || itr->value.IsArray()){
            val->EraseMember(itr);
            val->AddMember(rapidjson::Value(key, *alloc).Move(), js->val->Move(), *alloc);
        }else{
            return ZCAST_FAIL;
        }
    }else{
        val->AddMember(rapidjson::Value(key, *alloc).Move(), js->val->Move(), *alloc);
    }
    return ZOK;
}

inline bool z::json::RJson::DelMember(const char *key){
    return val->RemoveMember(key);
}

inline int z::json::RJson::EachObject(CallbackEach fun, void *hint){
    int ret = ZOK;
    z::json::RJson jsValue(NULL);
    z::json::RJson jsName(NULL);
    for(rapidjson::Value::MemberIterator itr = val->MemberBegin(); itr != val->MemberEnd(); ++itr){
        jsValue.SetValue(itr->value);
        jsName.SetValue(itr->name);
        switch(jsValue.GetType()){
        case valNumber: // Number
        case valString: // String
        case valTrue: // True
        case valFalse: // False
        case valNull: // Null
            ret = fun(jsValue.GetType(), jsName, jsValue, hint);
            break;
        case valArray: // Array
            fun(statArrayBegin, jsName, jsValue, hint);
            ret = jsValue.EachArray(fun, hint);
            fun(statArrayEnd, jsName, jsValue, hint);
            break;
        case valObject: // Object
            fun(statObjectBegin, jsName, jsValue,hint);
            ret = jsValue.EachObject(fun, hint);
            fun(statObjectEnd, jsName, jsValue, hint);
            break;
        default:
            ret = ZNOT_SUPPORT;
            break;
        }
        if(ZOK != ret){
            break;
        }
    }
    ZERRCX(ret);
    return ret;
}

inline int z::json::RJson::EachArray(CallbackEach fun, void *hint){
    int ret = ZOK;
    z::json::RJson js(NULL);
    rapidjson::SizeType size = val->Size();
    for(rapidjson::SizeType idx = 0; idx<size; idx++){
        js.SetValue((*val)[idx]);
        if(ZOK != (ret = js.ForEach(fun, hint))){
            break;
        }
    }
    ZERRCX(ret);
    return ret;
}

inline int z::json::RJson::ForEach(CallbackEach fun, void *hint){
    int ret = ZOK;
    z::json::RJson js(NULL);
    z::json::RJson jsName(NULL);
    if(!val){return ZPARAM_INVALID;} // assert
    js.val = val;
    switch(GetType()){
    case valNumber: // Number
    case valString: // String
    case valTrue: // True
    case valFalse: // False
    case valNull: // Null
        ret = fun(GetType(), jsName, js, hint);
        break;
    case valArray: // Array
        fun(statArrayBegin, jsName, js, hint);
        ret = js.EachArray(fun, hint);
        fun(statArrayEnd, jsName, js, hint);
        break;
    case valObject: // Object
        fun(statObjectBegin, jsName, js, hint);
        ret = js.EachObject(fun, hint);
        fun(statObjectEnd, jsName, js,hint);
        break;
    default:
        ret = ZNOT_SUPPORT;
        break;
    }
    ZERRCX(ret);
    return ret;
}
#endif
