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
 * @file tst_libevent.cpp
 * @brief <A brief description of what this file is.>
 * @author Z.Riemann https://github.com/ZRiemann/
 * @date 2018-04-19 Z.Riemann found
 *
 * @zmake.app zpptst;
 */
#include <stdio.h>
#include <string.h>
#include <zsi/base/error.h>
#include <zsi/base/trace.h>
#include <zsi/app/interactive.h>
#include <tests/tst_libevent.h>

using namespace z::ev2::core;

zerr_t tu_event_base(zop_arg){
    printf("#-------------------------------------------------------------------------------\n"
           "# event_base\n");

    return ZEOK;
}

static void ev_log_cb(int level, const char *msg){
    switch(level){
    case 0: zdbg("%s", msg);break;
    case 1: zmsg("%s", msg);break;
    case 2: zwar("%s", msg);break;
    case 3: zerr("%s", msg);break;
    default: zwar("%s", msg);break;
    }
}
static void tc_event_base_default();
zerr_t tc_event_base(zop_arg){
    zitac_arg_t *arg = (zitac_arg_t*)in;
    if(1 == arg->argc){
        tc_event_base_default();
    }else{
        zerrno(ZENOT_SUPPORT);
    }
    return ZEOK;
}

static void timer_cb(evutil_socket_t fd, short what, void *arg){
    static int cnt = 0;
    zinf("timer_cb called %d times so far.", ++cnt);
    if(cnt > 2){
        Event ev((struct event*)arg, false);
        ev.Del();
    }
}
static void tc_event_base_default(){
#if ZLIBEV_DEBUG
    EventBase::EnableDebug();
#endif
    EventBase::SetLogCb(ev_log_cb);
    EventBase base;
    Event *ev = base.EventNew(-1, EV_PERSIST, timer_cb);
    ev->Add(1000);
    //ev->Del();
    base.Dispatch();
    zinf("dispatch done.");
    delete ev;
}

/******************************************************************************\
 * event2_extra
 */
#include <zpp/event2/extra.h>

using namespace z::ev2::extra;

zerr_t tu_event2_extra(zop_arg){
    printf("# event2_extra\n");
    return ZEOK;
}

static void tc_event2_extra_default();
zerr_t tc_event2_extra(zop_arg){
    zitac_arg_t *arg = (zitac_arg_t*)in;
    if(1 == arg->argc){
        tc_event2_extra_default();
    }else{
        zerrno(ZENOT_SUPPORT);
    }
    return ZEOK;
}

static void event2_extra_dump_uri(HttpUri *uri){
    char buf[512] = {0};
    zinf("join: %s", uri->Join(buf, 512) ? buf : "nil");
}
static void tc_event2_extra_default(){
    zinf("test z::ev2::extra::HttpUri(http://user:pwd@127.0.0.1:80/path1/path2?key=value#fragment)");
    HttpUri uri("http://user:pwd@127.0.0.1:80/path1/path2?key=value#fragment");
    event2_extra_dump_uri(&uri);
    zinf("test z::ev2::extra::HttpUri()");
    HttpUri uri0;
    event2_extra_dump_uri(&uri0);
    
}