#ifndef _Z_OBJ_HPP_
#define _Z_OBJ_HPP_

#include <zsi/base/type.h>
#include <zsi/base/error.h>

#define NS_ZB namespace z{
#define NS_ZE }

NS_ZB

class Object{
public:
    virtual ~Object(){}
    virtual zerr_t Operate(Object *obj){return ZEOK;}
};

class Resource : public Object{
public:
    virtual ~Resource(){}
    /* Persistent interface */
    virtual zerr_t SerializeTo(Object *obj){return ZEOK;}
    virtual zerr_t LoadFrom(Object *obj){return ZEOK;}
    /* reference interface */
    virtual int32_t IncRef(){return 0;}
    virtual void Release(){/*if(0 == DecRef()) delete this;*/}
};

class Device : public Resource{
public:
    virtual ~Device(){}
    virtual zerr_t Init(Object *obj){return ZEOK;}
    virtual zerr_t Run(Object *obj){return ZEOK;}
    virtual zerr_t Stop(Object *obj){return ZEOK;}
    virtual zerr_t Fini(Object *obj){return ZEOK;}
};
NS_ZE

#endif