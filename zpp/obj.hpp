/**
 * MIT License
 *
 * Copyright (c) 2018 Z.Riemann
 * https://github.com/ZRiemann/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the Software), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM
 * , OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _ZPP_OBJ_HPP_
#define _ZPP_OBJ_HPP_
/**
 * @file zpp/obj.hpp
 */
#include <stdio.h>
#include <string>
#include <zsi/base/type.h>
#include <zsi/base/error.h>
#include <zsi/base/trace.h>
#include <zsi/base/atomic.h>

#ifndef NSB_Z
#define NSB_Z namespace z{
#define NSE_Z }
#endif

NSB_Z

#define DUMP_TOTAL 1

class Obj{
public:
    Obj()
        :type(0)
        ,id(0)
        ,state(0)
        ,ref(1){
        zatm_inc(&total);
#if DUMP_TOTAL
        zdbg("new obj, total<%d>", total);
#endif
    }
    virtual ~Obj(){
        zatm_dec(&total);
#if DUMP_TOTAL
        zdbg("del obj, total<%d>", total);
#endif
    }
    /* Object operate */
    virtual std::string &ToString(std::string &info){return info;}
    virtual zerr_t Clone(zop_arg){
        zatm_inc(&ref);
        return ZEOK;
    }
    virtual zerr_t Release(zop_arg){
        if(!zatm_dec(&ref)){
            delete this;
        }
        return ZEOK;
    }
    virtual zerr_t SerializeFrom(zop_arg){return ZEOK;}
    virtual zerr_t SerializeTo(zop_arg){return ZEOK;}
    virtual zerr_t Operate(zop_arg){return ZEOK;}

    /* Device operate */
    virtual zerr_t Init(zop_arg){return ZEOK;}
    virtual zerr_t Fini(zop_arg){return ZEOK;}
    virtual zerr_t Run(zop_arg){return ZEOK;}
    virtual zerr_t Stop(zop_arg){return ZEOK;}
public:
    ztype_t type; /** 对象类型 */
    zid_t id; /** 对象编号 */
protected:
    zatm32_t state; /** 对象状态 */
    zatm32_t ref; /** 对象引用计数 */
    static zatm32_t total; /** 对象总计数(辅助调试) */
};

template <typename O>
zerr_t ObjCreate(z::Obj** obj, void *hint){
    O *o = new(std::nothrow) O;
    if(!o){
        return ZEMEM_INSUFFICIENT;
    }
    if(NULL == (*obj = dynamic_cast<Obj*>(o))){
        return ZECAST_FAIL;
    }
    if(hint){
        return o->Serialize(NULL, NULL, hint);
    }
    return ZEOK;
}

#define REG_OBJ(otype, oname) case otype: return z::ObjCreate<oname>(obj, serial_hint)
#if 0 /* factory class sample */
class ObjFact{
public:
    static ObjFact *Inst(){
        static ObjFact of;
        return &of;
    }
    zerr_t Create(ztype_t type, Obj** obj, void *serial_hint){
        switch(type){
            REG_OBJ(0, sfx::Obj);
            // ... register more
        default:
            return ZENOT_EXIST;
        }
    }
};
#endif
NSE_Z

#endif
