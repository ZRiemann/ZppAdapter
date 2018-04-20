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
/**
 * @file event2_core.cpp
 * @brief <A brief description of what this file is.>
 * @author Z.Riemann https://github.com/ZRiemann/
 * @date 2018-04-19 Z.Riemann found
 */
#include <stdio.h>
#include <zpp/event2/core.h>

ZNSB_LIBEV
ZNSB_LIBEV_CORE
EventBase::EventBase(){
    base = event_base_new();
    if(!base){
        zerrno(ZEMEM_INSUFFICIENT);
    }else{
        SupportedMethods();
        zdbg("%s", zstrerr(ZEOK));
    }
}
EventBase::~EventBase(){
    event_base_free(base);
#if ZLIBEV_TRACE
    zdbg("%s", zstrerr(ZEOK));
#endif
}

EventBase::EventBase(const char *avoid_method, const char *rquire_feature, int set_flag){
    base = NULL;
}

void EventBase::SupportedMethods(){
    int i;
    const char **methods = event_get_supported_methods();
    zinf("Starting Libevent %s.  Available methods are:",
         event_get_version());
    for (i=0; methods[i] != NULL; ++i) {
        ztrace_org("    %s\n", methods[i]);
    }
    zinf("Using Libevent with backend method %s.",
         event_base_get_method(base));
    int f = event_base_get_features(base);
    if ((f & EV_FEATURE_ET)){
        zinf("  Edge-triggered events are supported.");
    }
    if ((f & EV_FEATURE_O1)){
        zinf("  O(1) event notification is supported.");
    }
    if ((f & EV_FEATURE_FDS)){
        zinf("  All FD types are supported.");
    }
}

zerr_t EventBase::PriorityInit(int n){
    zerr_t ret = event_base_priority_init(base, n);
    zerrno(ret);
    return ret;
}

zerr_t EventBase::ReInit(){
#if ZLIBEV_TRACE
    zdbg("%s", zstrerr(ZEOK));
#endif
    return event_reinit(base);
}
void EventBase::DumpStatus(const char *fname){
    if(NULL == fname){
        fname = "event_base_status";
    }
    FILE *pf = fopen(fname, "w");
    if(pf){
        event_base_dump_events(base, pf);
    }else{
        zerrno(ZEPARAM_INVALID);
    }
}

Event *EventBase::EventNew(evutil_socket_t fd, short what, event_callback_fn cb, void *hint){
    Event *evt = NULL;
    struct event *ev = event_new(base, fd, what, cb, hint ? hint : event_self_cbarg());
    zerr_t err = ZEMEM_INSUFFICIENT;
    if(ev){
        evt = new Event(ev, true);
        if(evt){
            err = ZEOK;
        }
    }
#if ZLIBEV_TRACE
    zdbg("evt<%p> = evt_new(base:%p, fd:%d, what:%x, cb:%p, hint:%p) %s",
         evt, base, fd, what, cb, hint, zstrerr(err));
#endif
    zerrno(err);
    return evt;
}

zerr_t EventBase::EventAssign(Event *ev, evutil_socket_t fd, short what, event_callback_fn cb, void *hint){
#if ZLIBEV_TRACE
    zerr_t ret = event_assign(ev->ev, base, fd, what, cb, hint ? hint : event_self_cbarg());
    zdbg("event_assign(ev<%p>, base<%p>, fd<%d>, what<%x>, cb<%p>, hint<%p>) %s",
         ev->ev, base, fd, cb, hint, zstrerr(ret));
    return ret;
#else
    ev->need_free = false;
    return event_assign(ev->ev, base, fd, what, cb, hint ? hint : event_self_cbarg());
#endif
}

struct event *EventBase::GetRunningEvent(){
#if ZLIBEV_TRACE
    struct event *ev = event_base_get_running_event(base);
    zdbg("event_base_get_running_event(base:%p) ev:%p", base, ev);
    return ev;
#else
    return event_base_get_running_event(base);
#endif
}

Event::Event(struct event *evt, bool _need_free)
    :ev(evt)
    ,need_free(_need_free){
#if ZLIBEV_TRACE
    zdbg("Event(ev<%p>, need_free<%d>)", evt, _need_free);
#endif
}
Event::~Event(){
#if ZLIBEV_DEBUG
    /* event_debug_unassign(ev); // Cause bug */
#endif
    if(need_free){
        event_free(ev);
#if ZLIBEV_TRACE
        zdbg("event_free(ev<%p>)", ev);
#endif
    }
}

zerr_t Event::Add(uint32_t ms){
    struct timeval tv = {ms/1000, (ms%1000)*1000};
#if ZLIBEV_TRACE
    zerr_t ret = event_add(ev, &tv);
    zdbg("event_add(ev<%p>, ms<%d>) %s", ev, ms, zstrerr(ret));
    return ret;
#else
    return event_add(ev, &tv);
#endif
}

zerr_t Event::Del(){
#if ZLIBEV_TRACE
    zerr_t ret = event_del(ev);
    zdbg("event_del(ev<%p>) %s", ev, zstrerr(ret));
    return ret;
#else
   return event_del(ev);
#endif
}

zerr_t Event::RemoveTimer(){
#if ZLIBEV_TRACE
    zerr_t ret = event_remove_timer(ev);
    zdbg("event_remove_timer(ev<%p>) %s", ev, zstrerr(ret));
    return ret;
#else
   return event_remove_timer(ev);
#endif
}
zerr_t Event::SetPriority(int priority){
#if ZLIBEV_TRACE
    zerr_t ret = event_priority_set(ev, priority);
    zdbg("event_priority_set(ev<%p>, prio<%d>) %s", ev, priority, zstrerr(ret));
    return ret;
#else
   return event_priority_set(ev, priority);
#endif
}

ZNSE_LIBEV_CORE
ZNSE_LIBEV

#if 0
/* Change the callback and callback_arg of 'ev', which must not be
 * pending. */
int replace_callback(struct event *ev, event_callback_fn new_callback,
    void *new_callback_arg)
{
    struct event_base *base;
    evutil_socket_t fd;
    short events;

    int pending;

    pending = event_pending(ev, EV_READ|EV_WRITE|EV_SIGNAL|EV_TIMEOUT,
                            NULL);
    if (pending) {
        /* We want to catch this here so that we do not re-assign a
         * pending event.  That would be very very bad. */
        fprintf(stderr,
                "Error! replace_callback called on a pending event!\n");
        return -1;
    }

    event_get_assignment(ev, &base, &fd, &events,
                         NULL /* ignore old callback */ ,
                         NULL /* ignore old callback argument */);

    event_assign(ev, base, fd, events, new_callback, new_callback_arg);
    return 0;
}

/* buffer event */
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <sys/socket.h>
#include <string.h>

void eventcb(struct bufferevent *bev, short events, void *ptr)
{
    if (events & BEV_EVENT_CONNECTED) {
         /* We're connected to 127.0.0.1:8080.   Ordinarily we'd do
            something here, like start reading or writing. */
    } else if (events & BEV_EVENT_ERROR) {
         /* An error occured while connecting. */
    }
}

int main_loop(void)
{
    struct event_base *base;
    struct bufferevent *bev;
    struct sockaddr_in sin;

    base = event_base_new();

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(0x7f000001); /* 127.0.0.1 */
    sin.sin_port = htons(8080); /* Port 8080 */

    bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

    bufferevent_setcb(bev, NULL, NULL, eventcb, NULL);

    if (bufferevent_socket_connect(bev,
        (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        /* Error starting connection */
        bufferevent_free(bev);
        return -1;
    }

    event_base_dispatch(base);
    return 0;
}
/*******************************************************************************/
/* Don't actually copy this code: it is a poor way to implement an
   HTTP client.  Have a look at evhttp instead.
*/
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <sys/socket.h>
#include <string.h>

void eventcb(struct bufferevent *bev, short events, void *ptr)
{
    if (events & BEV_EVENT_CONNECTED) {
         /* We're connected to 127.0.0.1:8080.   Ordinarily we'd do
            something here, like start reading or writing. */
    } else if (events & BEV_EVENT_ERROR) {
         /* An error occured while connecting. */
    }
}

int main_loop(void)
{
    struct event_base *base;
    struct bufferevent *bev;
    struct sockaddr_in sin;

    base = event_base_new();

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(0x7f000001); /* 127.0.0.1 */
    sin.sin_port = htons(8080); /* Port 8080 */

    bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

    bufferevent_setcb(bev, NULL, NULL, eventcb, NULL);

    if (bufferevent_socket_connect(bev,
        (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        /* Error starting connection */
        bufferevent_free(bev);
        return -1;
    }

    event_base_dispatch(base);
    return 0;
}

#endif