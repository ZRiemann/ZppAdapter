#ifndef _ZPP_OBJECT_H_
#define _ZPP_OBJECT_H_
/**
 * @file zpp/base/object.h
 * @brief 顶层框架抽象类
 * @note
 * @history
 *   2017-05-02  Zwp   found
 */
#include <zsi/base/error.h>
#include <zsi/base/type.h>
#include <zsi/base/atomic.h>
#include <vector>
#include <map>
#include <stdio.h>
#include <zpp/base/types.h>

#define ZUSE_OBJ_CNT 1 // 启用对象计数功能，可辅助排查内存泄漏

#if ZUSE_OBJ_CNT
#include <zsi/base/atomic.h>
#endif

#ifndef ZNS_ZB
#define ZNS_ZB namespace z{
#define ZNS_ZE }
#endif

ZNS_ZB

class Archive{
public:
    int isLoad;
    Archive();
    virtual ~Archive();
    virtual int GetData(zop_arg);
    virtual otype_t GetType();
};

class Object{
public:
    otype_t type;
public:
    Object();
    virtual ~Object();

    virtual int Serialize(zop_arg);
    virtual int Clone(zop_arg);
    virtual int Release(zop_arg);
    virtual int Operate(zop_arg);

public:
    static int CreateObj(zop_arg);
    static std::map<otype_t, zoperate>* pdyn_map;
#if ZUSE_OBJ_CNT
    static zatm32_t obj_cnt; // 辅助技术对象总数
#endif
};

template<typename T>
struct RegObj{
    static int create(zop_arg){
        int ret = ZOK;
        T *pa = 0;
        pa = new(std::nothrow) T;
        if(!pa){
            return ZMEM_INSUFFICIENT;
        }
        *out = (zvalue_t)pa;
        if(in){
            ret = pa->Serialize(in, out, hint);
        }
        return ret;
    }
    // 手动注册
    static void RegCreater(otype_t type, zoperate creater){
        if(!creater || !type){
            return;
        }
        if(!Object::pdyn_map){
            Object::pdyn_map = new std::map<otype_t , zoperate>;	// 由系统释放
        }
        (*Object::pdyn_map)[type] = creater;
    }
    // 自动注册
    RegObj(){
        if(!Object::pdyn_map){
            Object::pdyn_map = new std::map<otype_t , zoperate>;	// 由系统释放
        }
        T *pa = 0;
        pa = new T;
        if(pa){
            (*Object::pdyn_map)[pa->z::Object::type] = create;
            printf("register object<%08x> creater<%p>\n", pa->z::Object::type, create);
            delete pa;
        }else{
            printf("register failed.\n");
        }
    }
};
#define ZDECLARE_OBJ(class_type, class_name) static z::RegObj<class_name> Obj##class_type
#define ZIMPLEMENT_OBJ(class_type, class_name) z::RegObj<class_name> class_name::Obj##class_type

typedef Object Item;
typedef std::vector<Item*> Items;
typedef Items::iterator ItemsIt;
typedef Items::const_iterator ItemsCit;
typedef std::map<z::oid_t, Item*> Mitem; // <oid, obj*>

class Resource : public Object{
public:
    Resource();
    ~Resource();
    oid_t id;
    ostat_t status; // 资源状态
    //Items items;
    Mitem items;
};

typedef std::vector<Resource*> Resources;
typedef Resources::iterator ResIt;
typedef Resources::const_iterator ResCit;

class Device : public Resource{
public:
    Device();
    virtual ~Device();

    virtual int Init(zop_arg);
    virtual int Fini(zop_arg);
    virtual int Run(zop_arg);
    virtual int Stop(zop_arg);
};
ZNS_ZE
#endif
