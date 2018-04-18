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

#ifndef _ZMSG_QUEUE_HPP_
#define _ZMSG_QUEUE_HPP_
/**
 * @file msg_queue.hpp
 * @brief 通用线程安全消息队列
 * @author Z.Riemann
 * @date 2017-10-28 found
 * @date 2017-11-15 添加队列缓存区功能
 */
#include <stdlib.h>
#include <queue>
#include <zsi/base/type.h>
#include <zsi/base/error.h>
#include <zsi/thread/semaphore.h>
#include <zsi/thread/atomic.h>
#include <zsi/adapter/spin.hpp>

#ifndef ZNS_ZB
#define ZNS_ZB namespace z{
#define ZNS_ZE }
#endif

ZNS_ZB

template<class T>
class MsgQueue{
public:
    MsgQueue(){
        zsem_init(&sem, 0);
        zspin_init(&spin_in);
        zspin_init(&spin_out);
        que_in = new(std::nothrow) std::queue<T>;
        que_out = new(std::nothrow) std::queue<T>;
    }
    ~MsgQueue(){
        delete que_in;
        delete que_out;
        zspin_fini(&spin_out);
        zspin_fini(&spin_in);
        zsem_fini(&sem);
    }
    /* producer - customer mode */
    void Push(T t){
        SpinLock lck(&spin_in);
        que_in->push(t);
        if(1 == que_in->size()){
            zsem_post(&sem);
        }
    }

    err_t Pop(T &t, int timeout_ms = 1000){
        err_t ret = ZEOK;

        SpinLock lck(&spin_out);

        if(que_out->empty()){
            if(ZEOK == (ret = zsem_wait(&sem, timeout_ms))){
                que_out = zatmc_xchg_ptr(&que_in, que_out);
            }
        }

        if(ZEOK == ret){
            t = que_out->front();
            que_out->pop();
        }
        return ret;
    }
protected:
    std::queue<T> *que_in;
    std::queue<T> *que_out;
    spin_t spin_in;
    spin_t spin_out;
    sem_t sem;
};

template<typename T>
class MsgPoolBase{
public:
    err_t Alloc(T *&t){
        t = new(std::nothrow) T;
        return t ? ZEOK : ZEMEM_INSUFFICIENT;
    }
    void Free(T *&t){
        delete t;
        t = NULL;
    }
};

/**
 * @brief 环形缓冲区(线程安全)
 */
template<typename T>
class MsgPoolRing{
public:
    /** 消息内存缓冲区功能 */
    MsgPoolRing(int _size){
        _size <<= 1;
        while(_size){
            capacity = _size;
            _size &= _size-1;
        }
        buf = (T**)calloc(sizeof(T*), capacity);
        mask = capacity - 1;
        pos = 0;
        size = 0;
        zspin_init(&spin);
    }

    ~MsgPoolRing(){
        for(int i = 0; i < size; ++i){
            delete buf[(pos + i) & mask];
        }
        free(buf);
        zspin_fini(&spin);
    }

    void PreAlloc(){
        if(0 == pos && size == 0){
            for(int i = 0; i < capacity; ++i){
                buf[i] = new(std::nothrow) T;
                if(buf[i]){
                    size = i;
                }else{
                    break;
                }
            }
        }
    }
    err_t Alloc(T *&t){
        zspin_lock(&spin);
        if(size > 0){
            t = buf[pos & mask];
            ++pos;
            --size;
            zspin_unlock(&spin);
        }else{
            zspin_unlock(&spin);
            t = new(std::nothrow) T;
            return t ? ZEOK : ZEMEM_INSUFFICIENT;
        }
        return ZEOK;
    }

    void Free(T *&t){
        zspin_lock(&spin);
        if(size < capacity){
            t->Release(ZOPNULL);
            buf[(pos + size) & mask] = t;
            ++size;
            zspin_unlock(&spin);
        }else{
            zspin_unlock(&spin);
            delete t;
        }
        t = 0;
    }
protected:
    T **buf;
    spin_t spin;
    int capacity;
    int mask;
    int pos;
    int size;
};

#ZNS_ZE
#endif