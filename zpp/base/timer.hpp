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

#ifndef _ZTIMER_HPP_
#define _ZTIMER_HPP_
/**
 * @copyright (C) 2017 RuiTing Tech.Co., Ltd
 * @file timer.hpp
 * @brief 数据同步计时器
 * @author Zwp
 * @date 2017-11-17
 */
#include <set>
#include <map>
#include <zsi/base/type.h>
#include <zsi/base/error.h>
#include <zsi/base/time.h>
#include <zsi/thread/spin.h>
#include <zsi/thread/semaphore.h>

#ifndef ZNS_ZB
#define ZNS_ZB namespace z{
#define ZNS_ZE }
#endif

ZNS_ZB
/**
 * @brief 同步超时计时器
 *
 * @pra 应用场景
 *      1) 多任务同步请求应答超时计时器
 *      SyncTimer<int, reply_t> timer;
 *
 *      request(){
 *        reply_t *rep = 0; post request;
 *        if(ZEOK == timer.Wait(1, &rep)){
 *          parse reply
 *        }else{ timeout }
 *      }
 *
 *      call_back(int key, reply_t *rep){
 *        if(ZEOK != Post(key, rep)){
 8          release rep;
 *        }
 *      }
 * @note
 *       同步超时采用信号量，精确到毫秒超时；
 */
template<typename Key, typename Value>
class SyncTimer{
public:
    typedef struct sync_out_s{
        sync_out_s(Value **_out){
            out = _out;
        }
        sem_t sem;
        Value **out;
    }out_t;

    typedef std::map<Key, out_t*> sem_map_t;

    SyncTimer(){
        zspin_init(&spin);
    }

    ~SyncTimer(){
        zspin_fini(&spin);
    }
    err_t Push(Key &key, out_t *out){
        err_t ret = ZEOK;

        zspin_lock(&spin);
        if(sem_map.find(key) == sem_map.end()){
            zsem_init(&out->sem, 0);
            sem_map[key] = out;
            zspin_unlock(&spin);
        }else{
            zspin_unlock(&spin);
            ret = ZEEXIST;
            return ret;
        }
        return ret;
    }

    err_t Wait(Key &key, out_t *out, int timeout_ms){
        err_t ret = ZEOK;

        ret = zsem_wait(&out->sem, timeout_ms);
        zspin_lock(&spin);
        sem_map.erase(key);
        zspin_unlock(&spin);

        zsem_fini(&out->sem);
        return ret;
    }

    err_t Pop(Key &key, Value *in){
        err_t ret = ZENOT_EXIST;

        typename sem_map_t::iterator it;

        zspin_lock(&spin);
        it = sem_map.find(key);
        if(it != sem_map.end()){
            *(it->second->out) = in;
            zsem_post(&(it->second->sem));
            ret = ZEOK;
        }
        zspin_unlock(&spin);

        return ret;
    }
protected:
    sem_map_t sem_map;
    spin_t spin;
};

/**
 * @brief 异步计时器，精确度由用户实现决定
 *
 * @par demo
 *      AsyncTimer timer;
 *
 *      request(){
 *        timer.Post(key, timeout);
 *      }
 *
 *      call_back(){
 *        if(ZEOK == timer.Ack(key, value)){
 *          parse reply...
 *        }ele{ release value }
 *      }
 *
 *      timer(){
 *        Check();
 *      }
 */

template<typename Key>
class AsyncTimer{
public:
    typedef void (*Timeout)(Key key, ctx_t hint);
    typedef std::pair<timestamp_t, Key> ptk_t;
    typedef std::set< ptk_t > timestamp_set_t;

public:
    AsyncTimer(){
        zspin_init(&spin);
    }
    ~AsyncTimer(){
        zspin_fini(&spin);
    }

    err_t Push(Key &key, int timeout_ms){
        zspin_lock(&spin);
        ts_set.insert(std::make_pair(ztimestamp_ms() +
                                     timeout_ms,
                                     key));
        zspin_unlock(&spin);
        return ZEOK;
    }

    err_t Pop(Key &key){
        err_t ret = ZENOT_EXIST;
        zspin_lock(&spin);
        typename timestamp_set_t::iterator it;

        for(it = ts_set.begin(); it != ts_set.end();){
            if(it->second == key){
                ret = ZEOK;
                ts_set.erase(it++);
                break; /* fast find */
            }else{
                ++it;
            }
        }

        zspin_unlock(&spin);
        return ret;
    }

    err_t Timer(Timeout fn_timeout, ctx_t hint){
        err_t ret = ZENOT_EXIST;
        zspin_lock(&spin);
        timestamp_t now = ztimestamp_ms();
        typename timestamp_set_t::iterator it;
        for(it = ts_set.begin(); it != ts_set.end();){
            if(it->first <=  now){
                /* time out task */
                fn_timeout(it->second, hint);
                ts_set.erase(it++);
            }else{
                /* ++it;
                 * not time out items, compare down.
                 */
                break;
            }
        }
        zspin_unlock(&spin);
        return ZEOK;
    }

protected:
    timestamp_set_t ts_set;
    spin_t spin;
};
ZNS_ZE
#endif
