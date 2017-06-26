#ifndef _ZPP_CRTP_OBJ_H_
#define _ZPP_CRTP_OBJ_H_
/**@file zpp/framework/crtp_obj.h
 * @brief CRTP 模式封顶层抽象框架，避免虚函数及虚函数表
 * @note
 *   简洁接口模式：基类必须被继承，使用的接口必须被实现，避免无限地归；
 * @history
 *   2017-06-24 Z.Riemann found testing
 */
#define ZTRACE_CRTP_OBJ 1 // 控制日志输出

#include <zit/base/error.h>
#include <zit/base/atomic.h>
#ifdef ZTRACE_CRTP_OBJ
#include <zit/base/trace.h>
#endif
#include <zpp/framework/types.h>
#include <map>

namespace z{
    namespace crtp{
        
        // abstract Archive definition
        template<typename T>
        class Archive{
        protected:
            Archive(){}
            
        public:
            int isLoad;
        public:
            int GetData(ZOP_ARG){return static_cast<T*>(this)->GetData(in, out, hint);}
            otype_t GetType(){return static_cast<T*>(this)->GetType();}
        };

        // abstract Object definition(include resource)
        template<typename T>
        class Object{
        protected:
            Object(){} // 必须被继承，实现基本接口
        public:
            otype_t otype;
            oid_t oid;
            ostat_t ostat;
        public:
            T *Cast(){return static_cast<T*>(this);} // Cast()->OtherOperate(ZPO_ARG);
            int Serialize(ZOP_ARG){return static_cast<T*>(this)->Serialize(in, out, hint);}
            int Clone(ZOP_ARG){return static_cast<T*>(this)->Clone(in, out, hint);}
            int Release(ZOP_ARG){return static_cast<T*>(this)->Release(in, out, hint);}
            int Operate(ZOP_ARG){return static_cast<T*>(this)->Operate(in, out, hint);}

        public:
            static int CreateObj(ZOP_ARG);
#if ZTRACE_CRTP_OBJ
            static oid_t cbj_cnt;
#endif
        };

        // ObjFactory
        class Factory{
        public:
            template<typename T>
            static Object<T>* CreateObj(otype_t otype);
            
            template<typename T>
            Object<T> *Creater(otype_t otype);
            static std::map<otype_t, zoperate> mapCreater;
        };
    }
}

#endif
