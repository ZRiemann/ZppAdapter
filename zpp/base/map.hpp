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

#ifndef _ZPP_MAP_H_
#define _ZPP_MAP_H_

#ifndef ZNS_ZB
#define ZNS_ZB namespace z{
#define ZNS_ZE }
#endif

#include <zsi/base/type.h>
#include <zsi/base/error.h>

ZNS_ZB

template<typename T, class Key, class Value>
bool MapFind(T &t, const Key &k, Value &v){
    typename T::iterator it = t.find(k);
    if(it != t.end()){
        v = it->second;
        return true;
    }
    return false;
}

template<typename T, class Key, class Value>
zerr_t MapInsertDelOld(T &t, const Key &k, Value &v){
    typename T::iterator it = t.find(k);
    if(it != t.end()){
        delete it->second;
        it->second = v;
    }else{
        t[k] = v;
    }
    return ZEOK;
}

template<typename T, class Key>
zerr_t MapDel(T &t, const Key &k){
    typename T::iterator it = t.find(k);
    if(it != t.end()){
        delete it->second;
        t.erase(it);
        return ZEOK;
    }
    return ZENOT_EXIST;
}

template<typename T>
void MapDelClear(T &t){
    typename T::iterator it = t.begin();
    for(;it != t.end(); it++){
        delete it->second;
    }

    t.clear();
}

ZNS_ZE
#endif
