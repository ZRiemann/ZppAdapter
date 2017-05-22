#ifndef _ZPP_MAPFIND_H_
#define _ZPP_MAPFIND_H_

namespace z{
  template<typename T, class Key, class Value>
  bool MapFind(T &t, const Key &k, Value &v){
    typename T::iterator it = t.find(k);
    if(it != t.end()){
      v = it->second;
      return true;
    }
    return false;
  }

  template<typename T>
  void MapClear(T &t){
    typename T::iterator it = t.begin();
    
    for(;it != t.end(); it++){
      delete it->second;
    }

    t.clear();
  }
}

#endif
