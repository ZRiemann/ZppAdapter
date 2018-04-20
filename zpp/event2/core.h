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
#ifndef _ZEVENT2_CORE_H_
#define _ZEVENT2_CORE_H_

/**
 * @file core.h
 * @brief libevent 2.x core API wrapper
 * @author Z.Riemann https://github.com/ZRiemann/
 * @date 2018-04-20 Z.Riemann found
 *
 * @source https://github.com/libevent/libevent
 */
#include <zsi/base/trace.h>
#include <event2/event.h>
#include <event2/thread.h>

#ifndef ZNSB_LIBEV
#define ZNSB_LIBEV namespace z{ namespace ev2{
#define ZNSE_LIBEV }}
#define ZNSB_LIBEV_CORE namespace core {
#define ZNSE_LIBEV_CORE }
#define ZNSB_LIBEV_EXTRA namespace extra{
#define ZNSE_LIBEV_EXTRA }
#define ZLIBEV_TRACE 1
#define ZLIBEV_DEBUG 1
#endif

ZNSB_LIBEV

ZNSB_LIBEV_CORE
class Event;

class EventBase{
public:
    static void SetLogCb(event_log_cb cb){
        event_set_log_callback(cb);
#if ZLIBEV_TRACE
        zdbg("%s", zstrerr(ZEOK));
#endif
    }
    static void SetFatalCb(event_fatal_cb cb){
        event_set_fatal_callback(cb);
#if ZLIBEV_TRACE
        zdbg("%s", zstrerr(ZEOK));
#endif
    }
    static void SetMemFns(void *(*malloc_fn)(size_t sz),
                          void *(*realloc_fn)(void *, size_t),
                          void (*free_fn)(void *)){
        event_set_mem_functions(malloc_fn, realloc_fn, free_fn);
#if ZLIBEV_TRACE
        zdbg("%s", zstrerr(ZEOK));
#endif

    }
    static void EnableDebug(){
        event_enable_debug_mode();
#if ZLIBEV_TRACE
        zdbg("%s", zstrerr(ZEOK));
#endif
    }
    static void GlobalShutdown(){
        libevent_global_shutdown();
#if ZLIBEV_TRACE
        zdbg("%s", zstrerr(ZEOK));
#endif
    }
public:
    /** API wrapper */
    EventBase();
    ~EventBase();
    EventBase(const char *avoid_method, const char *rquire_feature, int set_flag);
    struct event_base *GetBase(){return base;} /** for extra API */
    void SupportedMethods();
    zerr_t PriorityInit(int n);
    zerr_t ReInit(); /** need reinit after fork() */
    zerr_t Loop(int flags){return event_base_loop(base, flags);} /** flags: EVLOOP_ONCE/NONBLOCK */
    zerr_t LoopContinue(){return event_base_loopcontinue(base);} /** continue loop */
    zerr_t Dispatch(){
#if ZLIBEV_TRACE
        zdbg("%s", zstrerr(ZEOK));
#endif
        return event_base_dispatch(base);
    }/** == Loop(0) */
    zerr_t Break(){return event_base_loopbreak(base);}
    zerr_t Exit(const struct timeval *tv){return event_base_loopexit(base, tv);}
    zbool_t GotExit(){return event_base_got_exit(base);}
    zbool_t GotBreak(){return event_base_got_break(base);}
    /** avoid system call */
    zerr_t CachedTimeOfDay(struct timeval *tv_out){
        return event_base_gettimeofday_cached(base, tv_out);
    }
    /** in long time cb, needs update cached time */
    zerr_t UpdateCacheTime(){return event_base_update_cache_time(base);}
    void DumpStatus(const char *fname = NULL);
    zerr_t ForEachEvent(event_base_foreach_event_cb fn, void *hint){
        return event_base_foreach_event(base, fn, hint);
    }

    /** Event
     * @param what EV_TIMEOUT/READ/WRITE/SIGNAL/PERSIST/ET
     * @cb void cb(evutil_socket_t fd, short what, void *arg)
     * @param hint NULL event_self_cbarg()
     *
     * @par Constructing signal events
     *      what: EV_SIGNAL|EV_PERSIST
     *      fd: SIG* ;SIGHUP/SIGINT/SIG
     *      only one event_base can monitor signal;
     */
    Event *EventNew(evutil_socket_t fd, short what, event_callback_fn cb, void *hint = NULL);
    zerr_t EventAssign(Event *, evutil_socket_t fd, short what, event_callback_fn cb, void *hint = NULL);
    struct event *GetRunningEvent(); /** Finding the currently running event */

    const struct timeval *InitCommmonTimeout(const struct timeval *duration); /** TODO: */

private:
    struct event_base *base;
};

class Event{
public:
    Event(struct event *ev, bool need_free);
    ~Event();
    struct event *GetEvent(){return ev;}
    zerr_t Add(struct timeval *tv){return event_add(ev, tv);}
    zerr_t Add(uint32_t ms); /** set event to pending status */
    zerr_t Del(); /** set non-pending or non-active */
    zerr_t RemoveTimer();
    zerr_t SetPriority(int priority);
    /* TODO: Inspecting event status */
    int GetPending(short what, uint32_t ms_out);
    evutil_socket_t GetFd();
    struct event_base *GetBase();
    short GetEvents();
    event_callback_fn GetCb();
    void *GetCbArg();
    int GetPriority();
    //void GetAssignment(...); // get all above fields
    void Active(int what, short ncals); /** TODO: Manually activating an event */
    
private:
    bool need_free;
    struct event *ev;
    friend EventBase;
};
ZNSE_LIBEV_CORE
ZNSE_LIBEV
#endif /*_ZEVENT2_CORE_H_*/
