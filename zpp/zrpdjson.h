#ifndef _ZPP_RPDJSON_H_
#define _ZPP_RPDJSON_H_
/**@file zpp/rapidjson/zrpdjson.h
 * @brief rapidjson 封装类
 * @note
 *   1. RJson 只有一个Doc节点，必须以RJson()构造
 *   2. RJson 子节点必须域Doc节点同生命周期
 */
#include <rapidjson/document.h>
#include <zit/base/type.h>
#include <string>
#include <vector>

namespace z{
  class RJson{
  public:
    RJson(); // DOC
    RJson(rapidjson::Document::AllocatorType *alloc); // Value alloc==NULL GetMemver;
    //RJson(rapidjson::Type type, rapidjson::Document::AllocatorType *alloc); // Value
    ~RJson();

    // 加载与保存
    int LoadFile(const char *file);
    int SaveFile(const char *file);

    int LoadString(const char *file);
    int SaveString(std::string &str);

    void Swap(RJson *js){
      val->Swap(*js->val);
    }
    // 设置与获取
    rapidjson::Document::AllocatorType *GetAllocator();
    
    //int SetArray(const char *key, const Value& value); // call SetValue
    // for(SizeType i = 0; i < a.Size(); i++){ a[i].GetInt()}
    int GetMember(const char *key, RJson *js);//rapidjson::Value **ppv);
    int AddMember(const char *key, RJson *js);//const rapidjson::Value& value);
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

    // Array ; Value mast a array
    void SetArray(){val->SetArray();}
    void SetObject(){val->SetObject();}
    template< typename T >
      void PushBack(T &t){
      val->PushBack(t, *alloc);
    }
    void PopBack(){
      val->PopBack();
    }
    void SetValue(rapidjson::Value &v){val = &v;}
    void SetAllocator(rapidjson::Document::AllocatorType *alc){alloc = alc;}
    typedef void (*HandleEach)(RJson *js, void *hint);
    void ForEach(HandleEach fun, void *hint);

  public: // 辅助函数
    static void SetTrace(int flag); // 0-no trace 1-trace
    void Dump();
  protected:
    //RJson(const RJson&); //禁止拷贝构造
    //RJson(RJson *);
    static int traceFlag;
    static const char *traceTitle;
    //rapidjson::Document doc;
    bool needDel;
    rapidjson::Value *val;
    rapidjson::Document *doc;
    rapidjson::Document::AllocatorType *alloc;
  };

  //static const char* kTypeNames[] = { "Null", "False", "True", "Object", "Array", "String", "Number" };
  /*
    for (Value::ConstMemberIterator itr = document.MemberBegin();
    itr != document.MemberEnd(); ++itr)
    {
    printf("Type of member %s is %s\n",
    itr->name.GetString(), kTypeNames[itr->value.GetType()]);
    }
    
    Value::ConstMemberIterator itr = document.FindMember("hello");
    if (itr != document.MemberEnd())
    printf("%s\n", itr->value.GetString());
  */
  //class RJson;
  //typedef std::vector<RJson*> RJsons;

}
#endif
