#ifndef _ZPP_OBJECT_H_
#define _ZPP_OBJECT_H_
/**@file zpp/framework/object.h
 * @brief 顶层框架抽象类
 * @note
 * @history
 *   2017-05-02  Zwp   found
 */
#include <zit/base/error.h>
#include <zit/base/type.h>
#include <zit/base/atomic.h>
#include <vector>
#include <map>
#include <stdio.h>
#include <zpp/framework/types.h>

#define ZUSE_OBJ_CNT 1 // 启用对象计数功能，可辅助排查内存泄漏

#if ZUSE_OBJ_CNT
#include <zit/base/atomic.h>
#endif

namespace z{

    class Archive{
    public:
        int isLoad;
        Archive();
        virtual ~Archive();
        virtual int GetData(ZOP_ARG);
        virtual otype_t GetType();
    };

    class Object{
    public:
        otype_t type;
    public:
        Object();
        virtual ~Object();

        virtual int Serialize(ZOP_ARG);
        virtual int Clone(ZOP_ARG);
        virtual int Release(ZOP_ARG);
        virtual int Operate(ZOP_ARG);

    public:
        static int CreateObj(ZOP_ARG);
        static std::map<otype_t, zoperate>* pdyn_map;
#if ZUSE_OBJ_CNT
        static zatm32_t obj_cnt; // 辅助技术对象总数
#endif
    };

    template<typename T>
        struct RegObj{
            static int create(ZOP_ARG){
                int ret = ZOK;
                T *pa = 0;
                pa = new T;
                if(in){
                    ret = pa->Serialize(in, out, hint);
                }
                if(pa){
                    *out = (zvalue_t)pa;
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

        virtual int Init(ZOP_ARG);
        virtual int Fini(ZOP_ARG);
        virtual int Run(ZOP_ARG);
        virtual int Stop(ZOP_ARG);
    };
}
#endif
