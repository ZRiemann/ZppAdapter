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

#ifndef _ZPP_OBJECT_H_
#define _ZPP_OBJECT_H_
/**
 * @file object.hpp
 * @brief CRTP 模式封顶层抽象框架，避免虚函数及虚函数表
 * @history
 *   2017-06-24 Z.Riemann found
 */
#include <stdio.h>
#include <zsi/base/type.h>
#include <zsi/base/error.h>
#include <zsi/base/atomic.h>

#ifndef ZNS_ZB
#define ZNS_ZB namespace z{
#define ZNS_ZE }
#endif

ZNS_ZB

class Obj;
/**
 * @brief 对象顶层抽象
 *
 * @par 避免虚函数及虚函数表，提高函数效率及内存利用率
 */
template<typename T>
class Object{
public:
    /**
     * @获取统一封装对象指针
     */
    Obj *Uniform();
    /**
     * @brief 获取对象描述信息
     */
    zerr_t ToString(std::string &info){
        return static_cast<T*>(this)->ToStringImp(info);
    }

    /**
     * @brief 克隆对象
     */
    zerr_t Clone(zop_arg){
        return static_cast<T*>(this)->CloneImp(in, out, hint);
    }

    /**
     * @brief 释放对象
     */
    zerr_t Release(zop_arg){
        return static_cast<T*>(this)->ReleaseImp(in, out, hint);
    }

    /**
     * @brief 序列化对象
     */
    zerr_t Serialize(zop_arg){
        return static_cast<T*>(this)->SerializeImp(in, out, hint);
    }

    /**
     * @brief 操作对象
     */
    zerr_t Operate(zop_arg){
        return static_cast<T*>(this)->OperateImp(in, out, hint);
    }

    /* 设备操作 */
    zerr_t Init(zop_arg){
        return static_cast<T*>(this)->InitImp(in, out, hint);
    }

    zerr_t Fini(zop_arg){
        return static_cast<T*>(this)->FiniImp(in, out, hint);
    }

    zerr_t Run(zop_arg){
        return static_cast<T*>(this)->RunImp(in, out, hint);
    }

    zerr_t Stop(zop_arg){
        return static_cast<T*>(this)->StopImp(in, out, hint);
    }

public:
    ztype_t type; /** 对象类型 */
    zid_t id; /** 对象编号 */

protected:
    zatm32_t status; /** 对象状态 */
    zatm32_t ref_cnt; /** 对象引用计数 */
    static zatm32_t total; /** 对象总计数(辅助调试) */
protected:
    Object()
        :type(0)
        ,id(0)
        ,status(0)
        ,ref_cnt(1){}
    ~Object(){}
    /* 默认接口实现 */
    zerr_t ToStringImp(std::string &info){
        char buf[128];
        sprintf(buf, "Objetc<type:%x, id:%d, status:%d, ref:%d>",
                type, id, status, ref_cnt);
        info = buf;
        printf("%s\n", buf);
        return ZEOK;
    }
    zerr_t CloneImp(zop_arg){
        *out = (zptr_t)this;
        return 2 > zatm_inc(&ref_cnt) ? ZEOK : ZEFAIL;
    }
    zerr_t ReleaseImp(zop_arg){
        if(!zatm_dec(&ref_cnt)){
            /* 如果有额外资源释放，需要重写该函数　*/
            delete this;
        }
        return ZEOK;
    }
    zerr_t SerializeImp(zop_arg){
        printf("%s\n", zstrerr(ZENOT_SUPPORT));
        return ZENOT_SUPPORT;
    }
    zerr_t OperateImp(zop_arg){
        printf("%s\n", zstrerr(ZENOT_SUPPORT));
        return ZENOT_SUPPORT;
    }
    zerr_t InitImp(zop_arg){
        printf("%s\n", zstrerr(ZENOT_SUPPORT));
        return ZENOT_SUPPORT;
    }
    zerr_t FiniImp(zop_arg){
        printf("%s\n", zstrerr(ZENOT_SUPPORT));
        return ZENOT_SUPPORT;
    }
    zerr_t RunImp(zop_arg){
        printf("%s\n", zstrerr(ZENOT_SUPPORT));
        return ZENOT_SUPPORT;
    }
    zerr_t StopImp(zop_arg){
        printf("%s\n", zstrerr(ZENOT_SUPPORT));
        return ZENOT_SUPPORT;
    }
};

/**
 * @brief 统一对象封装
 *
 * @note 可以将各类对象存储到同一容器中
 * @note 源对象生命周期，类似智能指针
 */
class Obj : public Object<Obj>{
public:
    Obj()
        :obj(NULL)
        ,operate(NULL)
        ,clone(NULL)
        ,release(NULL)
        ,serialize(NULL)
        ,init(NULL)
        ,fini(NULL)
        ,run(NULL)
        ,stop(NULL)
        ,to_string(NULL){}
    ~Obj(){
        (obj->*release)(zop_null);
    }
    typedef zerr_t (Obj::*oto_str)(std::string& info);
    typedef zerr_t (Obj::*ofn)(zop_arg);
    template<typename O>
    void Init(O *o){
        obj = reinterpret_cast<Obj*>(o);
        operate = (ofn)&O::Operate;
        clone = (ofn)&O::Clone;
        release = (ofn)&O::Release;
        serialize = (ofn)&O::Serialize;
        init = (ofn)&O::Init;
        fini = (ofn)&O::Fini;
        run = (ofn)&O::Run;
        stop = (ofn)&O::Stop;
        to_string = (oto_str)&O::ToString;
    }
    template<typename O>
    O *Cast(){
        return reinterpret_cast<O*>(obj);
    }
    zerr_t OperateImp(zop_arg){
        return (obj->*operate)(in, out, hint);
    }
    zerr_t CloneImp(zop_arg){
        return (obj->*clone)(in, out, hint);
    }
    zerr_t ReleaseImp(zop_arg){
        return (obj->*release)(in, out, hint);
    }
    zerr_t ToStringImp(std::string &info){
        return (obj->*to_string)(info);
    }
    zerr_t SerializeImp(zop_arg){
        return (obj->*serialize)(in, out, hint);
    }
protected:
    Obj *obj;
    ofn operate;
    ofn clone;
    ofn release;
    ofn serialize;
    ofn init;
    ofn fini;
    ofn run;
    ofn stop;
    oto_str to_string;
};

template<typename T>
Obj *Object<T>::Uniform(){
    Obj *obj = new(std::nothrow) Obj;
    if(obj){
        obj->Init(this);
    }else{
        zerrno(ZEMEM_INSUFFICIENT);
    }
    return obj;
}

ZNS_ZE

#endif
