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
 * zmake.app zpptst;
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
    libevent_global_shutdown();
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
    printf("# event2_extra http-server <port>\t\t; HTTP server\n");
    printf("# event2_extra http-client <host> <port> <mathod> <path> [data]\t\t; HTTP client\n");
    return ZEOK;
}
static zthr_ret_t ZCALL proc_http_svr(void* param);
static void tc_event2_extra_regress();
zerr_t tc_event2_extra(zop_arg){
    zitac_arg_t *arg = (zitac_arg_t*)in;
    if(1 == arg->argc){
        tc_event2_extra_regress();
    }else if(arg->argc == 3 && 0 == strcmp("http-server", arg->argv[1])){
        //printf("# event2_extra http-server <port> <path ...>\t\t; HTTP server\n");
        zthr_id_t tid;
        zthread_create(&tid, proc_http_svr, in);
        zthread_detach(&tid);
    }else if(arg->argc > 6 && 0 == strcmp("http-client", arg->argv[1])){
        //printf("# event2_extra http-client <host> <port> <mathod> <path> [data]\t\t; HTTP client\n");
        zerrno(ZENOT_EXIST);
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

/*=============================================================================
 * Echo client
 */
typedef struct echo_cli_ctx_s{
    uint32_t cnt;
    uint32_t repeats;
}echo_cli_t;

static void echo_cli_read_cb(struct bufferevent *bev, void *ctx){
    /* This callback is invoked when there is data to read on bev.
     * just echo back message.
     */
    uint32_t cnt;
    int i;
    echo_cli_t *cli = (echo_cli_t*)ctx;
    struct evbuffer *input = bufferevent_get_input(bev);
    size_t len = evbuffer_get_length(input);
#if 1
    if(len){
        struct evbuffer_ptr pos;
        evbuffer_ptr_set(input, &pos, 0, EVBUFFER_PTR_SET);
        for(i = 0; i < len; i += sizeof(cnt)){
            pos.pos = i;
            evbuffer_copyout_from(input, &pos, (void*)&cnt, sizeof(cnt));
            zinf("pos[%d]=%d", i/sizeof(cnt), cnt);
            evbuffer_ptr_set(input, &pos, sizeof(cnt), EVBUFFER_PTR_ADD);
        }
        evbuffer_drain(input, len);
    }
    zinf("[tid:%d] recv len<%d>", zthread_self(), len);

    if(cnt >= cli->repeats){
        bufferevent_disable(bev, EV_WRITE);

        struct event_base *base = bufferevent_get_base(bev);
        zinf("[tid:%d] repeats done. <base:%p>", zthread_self(), base);
        event_base_loopexit(base, NULL);
    }else{
        //++cli->cnt;
        bufferevent_enable(bev, EV_WRITE | EV_READ);
    }
#else
    if(cli->cnt < cli->repeats){
        zinf("[tid:%d] client repeated[%d].", zthread_self(), ++(*(uint32_t*)ctx));
        evbuffer_add_buffer(bufferevent_get_output(bev), bufferevent_get_input(bev));
    }
#endif
}

static void echo_cli_write_cb(struct bufferevent *bev, void *ctx){
    //zsleepsec(1);
    if(g_stop){
        struct event_base *base = bufferevent_get_base(bev);
        zinf("[tid:%d] global stop flag setting and stop listener<base:%p>",
             zthread_self(), base);
        event_base_loopexit(base, NULL);
    }else{
        echo_cli_t *cli = (echo_cli_t*)ctx;
        ++cli->cnt;
        if(cli->cnt > 10){
            bufferevent_disable(bev, EV_WRITE);
        }else{
            zinf("[tid:%d] send[%d].", zthread_self(), cli->cnt);
            bufferevent_write(bev, (void*)&(cli->cnt), sizeof(uint32_t));
        }
    }
}

static void echo_cli_event_cb(struct bufferevent *bev, short events, void *ctx){
    if (events & BEV_EVENT_ERROR){
        zerr("Error from bufferevent");
        struct event_base *base = bufferevent_get_base(bev);
        event_base_loopexit(base, NULL);
    }
    if(events & BEV_EVENT_CONNECTED){
        size_t low = 0xffff;
        size_t high = 0xffff;
        bufferevent_getwatermark(bev, EV_WRITE, &low, &high);
        zinf("[tid:%d] bev<%p> connect down watermark[%d, %d]", zthread_self(), bev, low, high);
    }
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        zinf("[tid:%d] free bev<%p>", zthread_self(), bev);
        if(g_stop){
            struct event_base *base = bufferevent_get_base(bev);
            zinf("global stop flag setting and stop listener<base:%p>", base);
            event_base_loopexit(base, NULL);
        }
        //bufferevent_free(bev);
    }
}

static zthr_ret_t ZCALL proc_echo_cli(void* param){
    zitac_arg_t *arg = (zitac_arg_t*)param;
    echo_cli_t ctx = {0};
    zinf("thread[%d]<%d> %s running...",zatm_inc(&g_threads), zthread_self(), arg->argv[1]);
#if 1
    struct event_base *base = event_base_new();
    struct evdns_base *dns_base = evdns_base_new(base, 1);
    struct bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

    bufferevent_setcb(bev, echo_cli_read_cb, echo_cli_write_cb, echo_cli_event_cb, (void*)&ctx);
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    ++ctx.cnt;
    zinf("[tid:%d] send[%d].", zthread_self(), ctx.cnt);
    bufferevent_write(bev, &ctx.cnt, sizeof(uint32_t));

    bufferevent_socket_connect_hostname(bev, dns_base, AF_INET, arg->argv[2], atoi(arg->argv[3]));
    ctx.repeats = atoi(arg->argv[4]);
    event_base_dispatch(base);
    bufferevent_free(bev);
    event_base_free(base);
#else
    EventBase base;
    BufferEvent bev(base.GetBase());
    bev.SetFree(true);
    bev.SetCb(echo_cli_read_cb, echo_cli_write_cb, echo_cli_event_cb, (void*)&ctx);
    // event2_base echo_client <host> <port> <repeats> <size>
    bev.Connect(arg->argv[2], atoi(arg->argv[3]));
    bev.Enable(EV_READ|EV_WRITE);
    base.Dispatch();
#endif
    zinf("thread<%d> exit now. remain<%d>", zthread_self(), zatm_dec(&g_threads));
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
    evbuffer_add_buffer(bufferevent_get_output(bev), bufferevent_get_input(bev));
}
static void echo_svr_event_cb(struct bufferevent *bev, short events, void *ctx){
    if (events & BEV_EVENT_ERROR){
        zerr("Error from bufferevent");
    }
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        zinf("[tid:%d] free bev<%p>", zthread_self(), bev);
        if(g_stop){
            struct event_base *base = bufferevent_get_base(bev);
            zinf("global stop flag setting and stop listener<base:%p>", base);
            event_base_loopexit(base, NULL);
        }
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

    evconnlistener_free(listener);
    event_base_free(base);
    zinf("thread<%d> exit now. remain<%d>", zthread_self(), zatm_dec(&g_threads));
    return (zthr_ret_t)0;
}

/******************************************************************************
 * HTTP Server
 */
#define HTTP_SIGNATURE "event2-http-server-test"
static void http_gencb(struct evhttp_request *req, void *hint){
    evhttp_send_error(req, HTTP_NOTFOUND,
                      "/v1\n"
                      "/v1/test\n");
}
static void http_cb_v1(struct evhttp_request *req, void *hint){
    //HTTP header
    evhttp_add_header(evhttp_request_get_output_headers(req), "Server", HTTP_SIGNATURE);
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "text/plain; charset=UTF-8");
    evhttp_add_header(evhttp_request_get_output_headers(req), "Connection", "close");
    //body
    struct evbuffer *buf;
    buf = evbuffer_new();
    evbuffer_add_printf(buf, "tid:%d v1 !\n", zthread_self());
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}
static void http_cb_v1_test(struct evhttp_request *req, void *hint){
    //HTTP header
    evhttp_add_header(evhttp_request_get_output_headers(req), "Server", HTTP_SIGNATURE);
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "text/plain; charset=UTF-8");
    evhttp_add_header(evhttp_request_get_output_headers(req), "Connection", "close");
    //body
    struct evbuffer *buf;
    buf = evbuffer_new();
    evbuffer_add_printf(buf, "tid:%d v1/test!\n", zthread_self());
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}
#if 0
static void http_errcb(enum evhttp_request_error errcode, void *hint){
    const char *err_desc = "unknown";
    switch(errcode){
    case EVREQ_HTTP_TIMEOUT: err_desc = "http-timeout"; break;
    case EVREQ_HTTP_EOF: err_desc = "http-eof"; break;
    case EVREQ_HTTP_INVALID_HEADER: err_desc = "http-invalid-header"; break;
    case EVREQ_HTTP_BUFFER_ERROR: err_desc = "http-buffer-error"; break;
    case EVREQ_HTTP_REQUEST_CANCEL: err_desc = "http-request-cancel"; break;
    case EVREQ_HTTP_DATA_TOO_LONG: err_desc = "http-data-too-long"; break;
    default:
    break;
    }
    zerr("http error: %s", err_desc);
}
#endif
static zthr_ret_t ZCALL proc_http_svr(void* param){
    zitac_arg_t *arg = (zitac_arg_t*)param;
    zinf("thread[%d]<%d> %s running...",zatm_inc(&g_threads), zthread_self(), arg->argv[1]);

    struct event_base *base;
    struct evhttp *http;

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
    http = evhttp_new(base);
    if(evhttp_bind_socket(http, "0.0.0.0", port)){
        zerr("Couldn't bind socket at 0.0.0.0:%d", port);
        return (zthr_ret_t)1;
    }else{
        zinf("http server: 0.0.0.0:%d", port);
    }
    evhttp_set_cb(http, "/v1", http_cb_v1, NULL);
    evhttp_set_cb(http, "/v1/test", http_cb_v1_test, NULL);
    evhttp_set_gencb(http, http_gencb, NULL);

    evhttp_set_timeout(http, 3);

    event_base_dispatch(base);

    evhttp_free(http);
    event_base_free(base);
    zinf("thread<%d> exit now. remain<%d>", zthread_self(), zatm_dec(&g_threads));
    return (zthr_ret_t)0;
}
