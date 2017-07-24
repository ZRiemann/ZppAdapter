#ifndef _ZTST_RPD_JSON_HPP_
#define _ZTST_RPD_JSON_HPP_
#include "../zpp/json_rapid.hpp"
#include <string>

//static void HandleEachArrayValue(z::json::RJson *js, void *hint){
//    js->Dump();
//}

void tstjson_rapid(int argc, char **argv){
#if 0
    int ret = ZOK;
    zdbg("testing json_rapid.hpp wrpper...");
    z::json::RJson doc;
    z::json::RJson val(doc.GetAllocator()); // 叶节点
    z::json::RJson cms(doc.GetAllocator()); // 设备节点
    z::json::RJson self(doc.GetAllocator()); // 自身设备节点

    // 基本测试
    val.SetString(0, "aaa");
    val.SetString(0, "bbb");
  
    doc.SetObject();
  
    doc.SetString("String", "this is string value;");
    doc.SetInt("Int", 1);
    doc.SetUint("Uint", 2);
    doc.SetInt64("Int64", 3);
    doc.SetUint64("Uint64", 4);
    doc.SetDouble("Double", 3.14159265358979);
    doc.SetBool("BoolTrue", true);
    doc.SetBool("BoolFalse", false);

    int32_t i(0);
    uint32_t ui(0);
    int64_t i64(0);
    uint64_t ui64(0);
    std::string str;

    // 初始化数据项
    ZERRC(doc.GetInt("Int",i));
    ZERRC(doc.GetUint("Uint",ui));
    ZERRC(doc.GetInt64("Int64",i64));
    ZERRC(doc.GetUint64("Uint64",ui64));
    ZERRC(doc.GetString("String",str));
  
    zdbg("\nInt:%d \nUint:%u \nInt64:%lld \nUint64:%llu \nString:%s", i,ui,i64,ui64, str.c_str());
    doc.Dump();

    // 更新数据项
    doc.SetString("String", "this is string value xxx;");
    doc.SetInt("Int", 11);
    doc.SetUint("Uint", 22);
    doc.SetInt64("Int64", 33);
    doc.SetUint64("Uint64", 44);
    doc.SetDouble("Double", 3.14);
    doc.SetBool("BoolTrue", true);
    doc.SetBool("BoolFalse", false);

    doc.Dump();
    ZERRC(doc.GetInt("Int",i));
    ZERRC(doc.GetInt("Int",i));
    ZERRC(doc.GetUint("Uint",ui));
    ZERRC(doc.GetInt64("Int64",i64));
    ZERRC(doc.GetUint64("Uint64",ui64));
    ZERRC(doc.GetString("String",str));
  
    zdbg("\nInt:%d \nUint:%u \nInt64:%lld \nUint64:%llu \nString:%s", i,ui,i64,ui64, str.c_str());

    // 设备节点测试
    // 初始化设备节点
    cms.SetObject();
    cms.SetString("host", "192.168.48.128"); // or host name
    cms.SetUint("repPort", 5800); // default cms port 5800
    cms.Dump();
    // 挂载设备节点
    doc.AddMember("cms", &cms);
    zdbg("dump cms (check cms is swaped)");
    cms.Dump();

    // 初始化并挂载自身节点
    self.SetObject();
    // self.SetUint("id", 3); // 利用id标识本设备
    self.SetString("host", "192.168.48.128"); // 利用主机地址标识本设备
    // self.SetUint("repPort", 5810);
    //挂载自身节点
    doc.AddMember("self", &self);

    doc.Dump();

    // 删除设备节
    zdbg("Delete doc member<self>");
    doc.DelMember("self");
    zdbg("Delete doc member<cms>");
    doc.DelMember("cms");
    zdbg("dump doc now:");
    doc.Dump();

    // 重新设置cms
    int ival;
    z::json::RJson cms1(doc.GetAllocator());
    cms1.SetArray();
    ival = 0;
    cms1.PushBack(ival);
    ival = 1;
    cms1.PushBack(ival);
    doc.AddMember("cmsArray", &cms1);
    zdbg("dump doc.cmsArray");
    doc.Dump();

    // 重新设置cms
    double dbl;
    cms.SetArray();
    dbl = 3.1415926;
    cms.PushBack(dbl);
    dbl = 2.71828;
    cms.PushBack(dbl);
    doc.AddMember("doubleArray", &cms);
    zdbg("dump doc.doubleArray reuse cms value");
    doc.Dump();

    // 遍历数组
    z::json::RJson array(NULL);
    if(ZOK == (ret = doc.GetMember("doubleArray", &array))){
        zdbg("domp each array");
        array.ForEach(HandleEachArrayValue, &doc);
        zdbg("pop array");
        array.PopBack();
        //array.ForEach(NULL, &doc);
    }else{
        ZERRC(ret);
    }
#endif
}

void tstrpdjson(int argc, char **argv){
    zdbg("testing rapid jsoin wrapper...");
#if 0
    z::RJson doc; // 根节点
    z::RJson val(doc.GetAllocator()); // 叶节点
    z::RJson cms(doc.GetAllocator()); // 设备节点
    z::RJson self(doc.GetAllocator()); // 自身设备节点

    // 基本测试
    val.SetString(0, "aaa");
    val.SetString(0, "bbb");
  
    doc.SetObject();
  
    doc.SetString("String", "this is string value;");
    doc.SetInt("Int", 1);
    doc.SetUint("Uint", 2);
    doc.SetInt64("Int64", 3);
    doc.SetUint64("Uint64", 4);
    doc.SetDouble("Double", 3.14159265358979);
    doc.SetBool("BoolTrue", true);
    doc.SetBool("BoolFalse", false);

    int32_t i(0);
    uint32_t ui(0);
    int64_t i64(0);
    uint64_t ui64(0);
    std::string str;

    // 初始化数据项
    ZERRC(doc.GetInt("Int",i));
    ZERRC(doc.GetUint("Uint",ui));
    ZERRC(doc.GetInt64("Int64",i64));
    ZERRC(doc.GetUint64("Uint64",ui64));
    ZERRC(doc.GetString("String",str));
  
    zdbg("\nInt:%d \nUint:%u \nInt64:%lld \nUint64:%llu \nString:%s", i,ui,i64,ui64, str.c_str());
    doc.Dump();

    // 更新数据项
    doc.SetString("String", "this is string value xxx;");
    doc.SetInt("Int", 11);
    doc.SetUint("Uint", 22);
    doc.SetInt64("Int64", 33);
    doc.SetUint64("Uint64", 44);
    doc.SetDouble("Double", 3.14);
    doc.SetBool("BoolTrue", true);
    doc.SetBool("BoolFalse", false);

    doc.Dump();
    ZERRC(doc.GetInt("Int",i));
    ZERRC(doc.GetInt("Int",i));
    ZERRC(doc.GetUint("Uint",ui));
    ZERRC(doc.GetInt64("Int64",i64));
    ZERRC(doc.GetUint64("Uint64",ui64));
    ZERRC(doc.GetString("String",str));
  
    zdbg("\nInt:%d \nUint:%u \nInt64:%lld \nUint64:%llu \nString:%s", i,ui,i64,ui64, str.c_str());

    // 设备节点测试
    // 初始化设备节点
    cms.SetObject();
    cms.SetString("host", "192.168.48.128"); // or host name
    cms.SetUint("repPort", 5800); // default cms port 5800
    cms.Dump();
    // 挂载设备节点
    doc.AddMember("cms", &cms);

    // 初始化并挂载自身节点
    self.SetObject();
    // self.SetUint("id", 3); // 利用id标识本设备
    self.SetString("host", "192.168.48.128"); // 利用主机地址标识本设备
    // self.SetUint("repPort", 5810);
    //挂载自身节点
    doc.AddMember("self", &self);

    doc.Dump();

    // 删除设备节
#endif
}

#endif
