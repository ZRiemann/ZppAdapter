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
#include <stdlib.h>
#include <string.h>
#include <zsi/base/error.h>
#include <zsi/base/trace.h>
#include <zsi/base/time.h>
#include <zsi/thread/thread.h>
#include <zsi/app/interactive.h>
#include <tests/tst_libevent.h>

using namespace z::ev2::core;

zerr_t tu_event2_base(zop_arg){
    printf("#-------------------------------------------------------------------------------\n"
           "# event2_base\t\t\t\t; Regress tests\n"
           "# event2_base echo_server <port>\t\t; Echo server\n"
           "# event2_base echo_client <host> <port> <repeats> <size>\t\t; Echo client\n"
        );

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
static void tc_event_base_regress();
static zthr_ret_t ZCALL proc_echo_cli(void*);
static zthr_ret_t ZCALL proc_echo_svr(void*);
zerr_t tc_event2_base(zop_arg){
    zitac_arg_t *arg = (zitac_arg_t*)in;
    if(1 == arg->argc){
        tc_event_base_regress();
    }else if(3 == arg->argc && 0 == strcmp("echo_server", arg->argv[1])){
        //"# event2_base echo_server <port>\t\t; Echo server\n"
        zthr_id_t tid;
        zthread_create(&tid, proc_echo_svr, in);
        zthread_detach(&tid);
    }else if(6 == arg->argc && 0 == strcmp("echo_client", arg->argv[1])){
        //"# event2_base echo_client <host> <port> <repeats> <size>\t\t; Echo client\n"
        zthr_id_t tid;
        zthread_create(&tid, proc_echo_cli, in);
        zthread_detach(&tid);

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
static void tc_event_base_regress(){
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
    printf("# event2_extra\t\t\t\t; Regress tests\n");
    return ZEOK;
}

static void tc_event2_extra_regress();
zerr_t tc_event2_extra(zop_arg){
    zitac_arg_t *arg = (zitac_arg_t*)in;
    if(1 == arg->argc){
        tc_event2_extra_regress();
    }else{
        zerrno(ZENOT_SUPPORT);
    }
    return ZEOK;
}

static void event2_extra_dump_uri(HttpUri *uri){
    char buf[512] = {0};
    zinf("join: %s", uri->Join(buf, 512) ? buf : "nil");
    zinf("scheme:\t%s", uri->GetScheme());
    zinf("host:\t%s", uri->GetHost());
    zinf("port:\t%d", uri->GetPort());
    zinf("path:\t%s", uri->GetPath());
    zinf("query:\t%s", uri->GetQuery());
    zinf("fragment:\t%s", uri->GetFragment());
}
static void tc_event2_extra_regress(){
    zinf("test z::ev2::extra::HttpUri(http://user:pwd@127.0.0.1:80/path1/path2?key=value#fragment)");
    HttpUri uri("http://user:pwd@127.0.0.1:80/path1/path2?key=value#fragment");
    event2_extra_dump_uri(&uri);
    zinf("test z::ev2::extra::HttpUri()");
    HttpUri uri0;
    event2_extra_dump_uri(&uri0);

    zinf("test z::ev2::extra::HttpUri::ParseQuery()");

    //const char *key = "key";
    //HttpUri::ParseQuery("http://user:pwd@127.0.0.1:80/path1/path2?key=value#fragment", &kv);
    //HttpKeyVal headers(&kv);
    //zinf("kv<%s, %s>",key, headers.Find(key));
}

#if 0
static zthr_ret_t ZCALL proc_demo(void* param){
    zitac_arg_t *arg = (zitac_arg_t*)param;

    zinf("thread[%d]<%04x> %s running...",zatm_inc(&g_threads), zthread_self(), arg->argv[1]);
    zsleepsec(10);
    zinf("thread<%04x> exit now. remain<%d>", zthread_self(), zatm_dec(&g_threads));
    return (zthr_ret_t)0;
}
#endif

static zthr_ret_t ZCALL proc_echo_cli(void* param){
    zitac_arg_t *arg = (zitac_arg_t*)param;

    zinf("thread[%d]<%04x> %s running...",zatm_inc(&g_threads), zthread_self(), arg->argv[1]);
    zsleepsec(10);
    zinf("thread<%04x> exit now. remain<%d>", zthread_self(), zatm_dec(&g_threads));
    return (zthr_ret_t)0;
}

/*=============================================================================
 * Echo Server
 */
static void echo_svr_read_cb(struct bufferevent *bev, void *ctx){
    /* This callback is invoked when there is data to read on bev.
     * just echo back message.
     */
    zinf("[tid:%d] echo back.", zthread_self());
    evbuffer_add_buffer(bufferevent_get_input(bev), bufferevent_get_input(bev));
}
static void echo_svr_event_cb(struct bufferevent *bev, short events, void *ctx){
    if (events & BEV_EVENT_ERROR){
        zerr("Error from bufferevent");
    }
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        zinf("[tid:%d] free bev<%p>", zthread_self(), bev);
        bufferevent_free(bev);
    }
}
static void accept_conn_cb(struct evconnlistener *listener,
                           evutil_socket_t fd, struct sockaddr *addrs, int socklen,
                           void *ctx){
    /* We got a new connection! Set up a bufferevent for it. */
    struct event_base *base = evconnlistener_get_base(listener);
    struct bufferevent *bev = bufferevent_socket_new(
        base, fd, BEV_OPT_CLOSE_ON_FREE);

    zinf("[tid:%d] accept connection bev<%p>", zthread_self(), bev);
    bufferevent_setcb(bev, echo_svr_read_cb, NULL, echo_svr_event_cb, NULL);

    bufferevent_enable(bev, EV_READ|EV_WRITE);
}
static void accept_error_cb(struct evconnlistener *listener, void *ctx){
    struct event_base *base = evconnlistener_get_base(listener);
    int err = EVUTIL_SOCKET_ERROR();
    zerr("[tid: %d] Got an error %d (%s) on the listener. Shutting down.\n",
         zthread_self(), err, evutil_socket_error_to_string(err));

    event_base_loopexit(base, NULL);
}

static zthr_ret_t ZCALL proc_echo_svr(void* param){
    zitac_arg_t *arg = (zitac_arg_t*)param;
    zinf("thread[%d]<%d> %s running...",zatm_inc(&g_threads), zthread_self(), arg->argv[1]);

    struct event_base *base;
    struct evconnlistener *listener;
    struct sockaddr_in sin;

    int port = 9876;

    port = atoi(arg->argv[2]);

    if (port<=0 || port>65535) {
        zerr("Invalid port");
        return (zthr_ret_t)1;
    }

    base = event_base_new();
    if (!base) {
        zerr("Couldn't open event base");
        return (zthr_ret_t)1;
    }

    /* Clear the sockaddr before using it, in case there are extra
     * platform-specific fields that can mess us up. */
    memset(&sin, 0, sizeof(sin));
    /* This is an INET address */
    sin.sin_family = AF_INET;
    /* Listen on 0.0.0.0 */
    sin.sin_addr.s_addr = htonl(0);
    /* Listen on the given port. */
    sin.sin_port = htons(port);

    listener = evconnlistener_new_bind(base, accept_conn_cb, NULL,
                                       LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1,
                                       (struct sockaddr*)&sin, sizeof(sin));
    if (!listener) {
        zerr("Couldn't create listener");
        return (zthr_ret_t)1;
    }
    evconnlistener_set_error_cb(listener, accept_error_cb);

    event_base_dispatch(base);

    zinf("thread<%d> exit now. remain<%d>", zthread_self(), zatm_dec(&g_threads));
    return (zthr_ret_t)0;
}
