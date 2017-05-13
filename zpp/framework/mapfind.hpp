#ifndef _ZPP_MAPFIND_H_
#define _ZPP_MAPFIND_H_

template<typename T, class Key, class Value>
bool MapFind(T &t, const Key &k, Value &v){
  typename T::iterator it = t.find(k);
  if(it != t.end()){
    v = it->second;
    return true;
  }
  return false;
}

#endif
