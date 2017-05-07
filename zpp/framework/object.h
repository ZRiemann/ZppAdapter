#ifndef _ZPP_OBJECT_H_
#define _ZPP_OBJECT_H_
/**@file zpp/framework/object.h
 * @brief 顶层框架抽象类
 * @note
 * @history
 *   2017-05-02  Zwp   found
 */

#include <zit/base/type.h>
#include <vector>
#include <map>

namespace z{

  class Archive{
  public:
    Archive();
    virtual ~Archive();
    int GetData(zvalue_t in, zvalue_t *out, zvalue_t hint);
  };
  
  class Object{
  public:
    int type;
  public:
    Object();
    virtual ~Object();

    virtual int Serialize(zvalue_t in, zvalue_t *out, zvalue_t hint);
    virtual int Clone(zvalue_t in, zvalue_t *out, zvalue_t hint);
    virtual int Release(zvalue_t in, zvalue_t *out, zvalue_t hint);
    virtual int Operate(zvalue_t in, zvalue_t *out, zvalue_t hint);
  };
  typedef Object Item;
  typedef std::vector<Item*> Items;
  typedef Items::iterator ItemsIt;
  typedef Items::const_iterator ItemsCit;
  typedef std::map<int, Items> ItemMap;
  
  class Resource : public Object{
  public:
    int id;
    Items items;
    //ItemMap itemsMap;
  };
  typedef std::vector<Resource*> Resources;
  typedef Resources::iterator ResIt;
  typedef Resources::const_iterator ResCit;

  class Device : public Resource{
  public:
    Device();
    virtual ~Device();

    virtual int Init(zvalue_t in, zvalue_t *out, zvalue_t hint);
    virtual int Fini(zvalue_t in, zvalue_t *out, zvalue_t hint);
    virtual int Run(zvalue_t in, zvalue_t *out, zvalue_t hint);
    virtual int Stop(zvalue_t in, zvalue_t *out, zvalue_t hint);   
  };
}
#endif
