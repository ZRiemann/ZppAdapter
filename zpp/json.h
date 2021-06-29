#ifndef __ZPP_JSON_H__
#define __ZPP_JSON_H__

#include <string>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/filereadstream.h>


namespace zpp{
class json{
public:
    json(){
        doc = new rapidjson::Document;
        alloc = &(doc->GetAllocator());
        pv = doc;
    }

    json(json &j)
        :doc(nullptr)
        ,alloc(j.alloc){
        pv = &v;
    }

    ~json(){
        if(doc != nullptr){
            delete doc;
        }
    }

    ErrorCode load_file(const std::string &fname){
        ErrorCode err = ERR_PARAM_INVALID;
        FILE* fp = NULL;
        do{
            if(nullptr == doc){
                break;
            }
#if ZSYS_WINDOWS
            fp = fopen(fname.c_str(), "rb");
#else
            fp = fopen(fname.c_str(), "r");
#endif
            if(!fp){
                IMS_ERR("Can not open: %s", fname.c_str());
                break;
            }
            char readBuffer[4096];
            rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
            doc->ParseStream(is);
            err = ERR_OK;
        }while(0);
        if(fp){
            fclose(fp);
        }
        return err;
    }

    ErrorCode load_string(const char *str, size_t len = 0){
        if(nullptr == doc){
            return ERR_NOT_SUPPORT;
        }
        if(len){
            rapidjson::MemoryStream mem(str, len);
            doc->ParseStream(mem);
        }else{
            doc->Parse(str);
        }
        return ERR_OK;
    }

    const char *to_string(rapidjson::StringBuffer &buffer, bool is_pretty = false){
        if(is_pretty){
            rapidjson::PrettyWriter<rapidjson::StringBuffer>writer(buffer);
            pv->Accept(writer);
        }else{
            rapidjson::Writer<rapidjson::StringBuffer>writer(buffer);
            pv->Accept(writer);
        }
        return (char*)buffer.GetString();
    }

    ErrorCode set_int(const std::string& key, int value){
        if(!pv->IsObject()){
            pv->SetObject();
        }
        rapidjson::Value::ConstMemberIterator itr = pv->FindMember(key.c_str());
        if (itr != pv->MemberEnd()){
            pv->EraseMember(itr);
        }
        pv->AddMember(rapidjson::Value(key.c_str(), *alloc).Move(), value, *alloc);
        return ERR_OK;
    }

    ErrorCode get_int(const std::string& key, int &value){
        rapidjson::Value::ConstMemberIterator itr = pv->FindMember(key.c_str());
        if (itr != pv->MemberEnd() && itr->value.IsInt()){
            value = itr->value.GetInt();
            return ERR_OK;
        }

        return ERR_NOT_EXIST;
    }

    ErrorCode set_string(const std::string &key, const std::string &value){
        rapidjson::Value::ConstMemberIterator itr = pv->FindMember(key.c_str());
        if (itr != pv->MemberEnd()){
            pv->EraseMember(itr);
        }
        pv->AddMember(rapidjson::Value(key.c_str(), *alloc).Move(),
                      rapidjson::Value(value.c_str(), *alloc).Move(),
                      *alloc);
        return ERR_OK;
    }

    ErrorCode get_string(const std::string &key, std::string &str){
        rapidjson::Value::ConstMemberIterator itr = pv->FindMember(key.c_str());
        if (itr != pv->MemberEnd() && itr->value.IsString()){
            str.assign(itr->value.GetString(), itr->value.GetStringLength());
            return ERR_OK;
        }
        return ERR_NOT_EXIST;
    }

    ErrorCode set_member(const std::string &key, json &value){
        if(value.v.IsNull()){
            return ERR_NOT_EXIST;
        }
        if(!pv->IsObject()){
            pv->SetObject();
        }
        rapidjson::Value::MemberIterator itr = pv->FindMember(key.c_str());
        if (itr != pv->MemberEnd()){
            pv->EraseMember(itr);
        }
        pv->AddMember(rapidjson::Value(key.c_str(), *alloc).Move(),
                       value.pv->Move(), *alloc);
        return ERR_OK;
    }

    ErrorCode get_member(const std::string &key, json &value){
        if(!pv->IsObject()){
            return ERR_NOT_SUPPORT;
        }
        rapidjson::Value::MemberIterator itr = pv->FindMember(key.c_str());
        if (itr != pv->MemberEnd()){
            value.pv = &(itr->value);
            return ERR_OK;
        }
        return ERR_NOT_EXIST;
    }

    template<typename T> ErrorCode push_back(T t){
        if(!pv->IsArray()){
            if(pv->IsNull()){
                pv->SetArray();
            }else{
                return ERR_NOT_SUPPORT;
            }
        }
        pv->PushBack(t, *alloc);
        return ERR_OK;
    }

    ErrorCode push_json(json &js){
        if(!pv->IsArray()){
            if(pv->IsNull()){
                pv->SetArray();
            }else{
                return ERR_NOT_SUPPORT;
            }
        }
        pv->PushBack((rapidjson::Value&)*(js.pv), *alloc);
        return ERR_OK;
    }

    size_t array_size(){
        if(!pv->IsArray()){
            return 0;
        }
        return (size_t)pv->Size();
    }
    ErrorCode at(json &js, size_t pos){
        if(!pv->IsArray()){
            return ERR_NOT_SUPPORT;
        }
        js.pv = &(*pv)[pos];
        return ERR_OK;
    }

    bool is_int(){
        return pv->IsInt();
    }

    int get_int(){
        return pv->GetInt();
    }

    bool is_object(){
        return pv->IsObject();
    }

    bool is_float(){
        return pv->IsFloat();
    }

    float get_float(){
        return pv->GetFloat();
    }
public:
    rapidjson::Document *doc;
    rapidjson::Document::AllocatorType *alloc;
    rapidjson::Value v;
    rapidjson::Value *pv;
};
}

#endif
