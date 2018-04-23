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
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>

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

/**
 * @brief event2 struct bufferevent wrapper
 *
 * @par summary
 *      Consists: {
 *         fd:int,;
 *         ibuf:evbuffer,read_cb;
 *         obuf:evbuffer,write_cb;
 *      }
 *      Invokes its user-supplied callbacks when it has read or write enough data;
 * BufferEvent Types
 *   socket-based: event_*
 *   asynchronous-IO: IOCP windows only;
 *   filtering:: to compress or translate data;
 *   paired: Two bufferevents that transmit data to one another
 * Callbacks and watermarks
 *   Read low-water mark, 0
 *   Read high-water mark, -1
 *   Write low-water mark, 0
 *   Write hight-water mark, -1
 *   BEV_EVENT_READING
 *   BEV_EVENT_WRITING
 *   BEV_EVENT_ERROR
 *   BEV_EVENT_TIMEOUT
 *   BEV_EVENT_EOF
 *   BEV_EVENT_CONNECTED
 * Defered callbacks
 *   event_loop()
 * Option flags for bufferevents
 *   BEV_OPT_CLOSE_ON_FREE: When the bufferent is freed, close the underlying transport;
 *   BEV_OPT_THREADSAFE: Atomatically allocate locks for the bufferevent;
 *   BEV_OPT_DEFFER_CALLBACKS: Defer callbacks
 *   BEV_OPT_UNLOCK_CALLBACKS: Release lock when it's invoking your callbacks;
 * Working with socket-based bufferevents
 *   Creating a socket-based bufferevent
 *   Default a newly created bufferevent has writing enabled but not reading.
 */
#include <event2/bufferevent.h>
class BufferEvent{
public:
    /**
     * @param fd -1 bufferevent_setfd()/bufferevent_socket_connect()
     *        make sure fd is non-blocking mode.
     */
    BufferEvent(struct event_base *base, evutil_socket_t fd ,
                enum bufferevent_options options =
                (enum bufferevent_options)(BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE)){
        bev = bufferevent_socket_new(base, fd, options);
        if(!bev){
            zerrno(ZEFAIL);
        }
        need_free = false;
    }
    BufferEvent(struct bufferevent *underlying,
                bufferevent_filter_cb input_filter,
                bufferevent_filter_cb output_filter,
                int options,
                void (*free_context)(void *),
                void *ctx){
        bev = bufferevent_filter_new(underlying, input_filter,
                                     output_filter, options,
                                     free_context, ctx);
        if(!bev){
            zerrno(ZEFAIL);
        }
        need_free = false;
    }
    BufferEvent(struct bufferevent *_bev)
        :bev(_bev)
        ,need_free(false){ /* for callback function */
    }
    ~BufferEvent(){
        if(need_free){
            bufferevent_free(bev);
        }
    }
    struct bufferevent *GetBev(){
        return bev;
    }
    void SetFree(bool nf = true){
        need_free = nf;
    }
    zerr_t Connect(const struct sockaddr *addr, int sock_len){
        zerr_t ret = bufferevent_socket_connect(bev, addr, sock_len);
        zerrno(ret);
        return ret;
    }
    zerr_t Connect(const char *host, uint16_t port){
        zerr_t ret = bufferevent_socket_connect_hostname(bev,
                                                         NULL, AF_INET,
                                                         host, port);
        if(-1 == ret){
            ret = bufferevent_socket_get_dns_error(bev);
            zwar("Dns error code<%d:%s>", ret, ret ? evutil_gai_strerror(ret) : "NIL");
        }
        zerrno(ret);
        return ret;
    }
    /*
     * Generic bufferevent operations
     */
    void SetCb(bufferevent_data_cb reader, bufferevent_data_cb writer,
               bufferevent_event_cb eventer, void *arg){
        bufferevent_setcb(bev, reader, writer, eventer, arg);
    }
    /**
     * @param events EV_READ|EV_WRITE
     */
    void Enable(short events){bufferevent_enable(bev, events);}
    void Disable(short events){bufferevent_disable(bev, events);}
    short GetEnable(){bufferevent_get_enabled(bev);}
    void SetWaterMark(short events, size_t lowmark, size_t hightmark = 0){
        /* hight mark 0 = unlimited*/
        bufferevent_setwatermark(bev, events, lowmark, hightmark);
    }
    struct evbuffer *GetInput(){return bufferevent_get_input(bev);}
    struct evbuffer *GetOutput(){return bufferevent_get_output(bev);}
    zerr_t Write(void *data, size_t size){return bufferevent_write(bev, data, size);}
    zerr_t WriteBuffer(struct evbuffer *buf){return bufferevent_write_buffer(bev,buf);}
    size_t Read(void *data, size_t size){return bufferevent_read(bev, data, size);}
    zerr_t ReadBuffer(struct evbuffer *buf){return bufferevent_read_buffer(bev, buf);}
    zerr_t SetTimeouts(int read_ms, int write_ms){
        struct timeval read, write;
        read.tv_sec = read_ms/1000;
        read.tv_usec = (read_ms%1000)*1000;
        write.tv_sec = write_ms/1000;
        write.tv_usec = (write_ms%1000)*1000;
        return bufferevent_set_timeouts(bev, read_ms ? &read : NULL,
                                        write_ms ? &write : NULL);
    }
    zerr_t Flush(short iotype, enum bufferevent_flush_mode state){
        return bufferevent_flush(bev, iotype, state) == -1 ? ZEFAIL : ZEOK;
    }
    void Lock(){return bufferevent_lock(bev);} /** need BEV_OPT_THREADSAFE */
    void Unlock(){return bufferevent_unlock(bev);}

    static zerr_t Pair(struct event_base *base, int options, struct bufferevent *pair[2]){
        return bufferevent_pair_new(base, options, pair);
    }
public:
    zerr_t Assign(struct event_base *base){return bufferevent_base_set(base, bev);}
    struct event_base *GetBase(){return bufferevent_get_base(bev);}
    zerr_t SetPriority(int pri){return bufferevent_priority_set(bev, pri);}
    int GetPriority(){return bufferevent_get_priority(bev);}
    void GetCb(bufferevent_data_cb *readcb_ptr, bufferevent_data_cb *writecb_ptr,
               bufferevent_event_cb *eventcb_ptr, void **cbarg_ptr){
        bufferevent_getcb(bev, readcb_ptr, writecb_ptr, eventcb_ptr, cbarg_ptr);
    }
    /*
     * int bufferevent_set_max_single_read(struct bufferevent *bev, size_t size);
     *int bufferevent_set_max_single_write(struct bufferevent *bev, size_t size);
     *ev_ssize_t bufferevent_get_max_single_read(struct bufferevent *bev);
     *ev_ssize_t bufferevent_get_max_single_write(struct bufferevent *bev);
     *void ev_token_bucket_cfg_free(struct ev_token_bucket_cfg *cfg);
     *int bufferevent_set_rate_limit(struct bufferevent *bev,
     *    struct ev_token_bucket_cfg *cfg);
     */
private:
    struct bufferevent *bev;
    bool need_free;
};


class EvBuffer{
public:
    EvBuffer(){
        buf = evbuffer_new();
        if(!buf){
            zerrno(ZEFAIL);
        }
    }
    ~EvBuffer(){
        if(need_free){
            evbuffer_free(buf);
        }
    }
    evbuffer *GetBuf(){return buf;}
    void SetFree(bool nf = true){need_free = nf;}

    zerr_t enable_locking(void *lock){return evbuffer_enable_locking(buf, lock);}
    void Lock(){evbuffer_lock(buf);}
    void Unlock(){evbuffer_unlock(buf);}
    size_t GetLength(){return evbuffer_get_length(buf);}
    size_t GetContigousSpace(){return evbuffer_get_contiguous_space(buf);}

    zerr_t Add(const void *data, size_t datalen){return evbuffer_add(buf, data, datalen);}
    zerr_t Add(::evbuffer *src){return evbuffer_add_buffer(buf, src);}
    zerr_t Remove(::evbuffer *dst, size_t datlen){
        return evbuffer_remove_buffer(buf,dst,datlen);
    }
    //int evbuffer_add_printf(struct evbuffer *buf, const char *fmt, ...)
    //int evbuffer_add_vprintf(struct evbuffer *buf, const char *fmt, va_list ap)
    zerr_t Expand(size_t datalen){return evbuffer_expand(buf, datalen);}

    zerr_t Prepend(const void *data, size_t size){return evbuffer_prepend(buf, data, size);}
    zerr_t Prepend(struct evbuffer *src){return evbuffer_prepend_buffer(buf, src);}
    unsigned char *PullUp(ev_ssize_t size){return evbuffer_pullup(buf, size);}
    zerr_t Drain(size_t len){return evbuffer_drain(buf, len);}
    zerr_t Remove(void *data, size_t datlen){return evbuffer_remove(buf, data, datlen);}
    ev_ssize_t Copy(void *data, size_t datlen){return evbuffer_copyout(buf, data, datlen);}
    int Peek(ev_ssize_t len, struct evbuffer_ptr *start_at, struct evbuffer_iovec *vec_out,
             int n_vec){ return evbuffer_peek(buf, len, start_at, vec_out, n_vec);}
    char *ReadLn(size_t *read, enum evbuffer_eol_style style = EVBUFFER_EOL_CRLF){
        return evbuffer_readln(buf, read, style);
    }
    /*
     * struct evbuffer_ptr evbuffer_search(struct evbuffer *buffer,
     *    const char *what, size_t len, const struct evbuffer_ptr *start);
     *struct evbuffer_ptr evbuffer_search_range(struct evbuffer *buffer,
     *    const char *what, size_t len, const struct evbuffer_ptr *start,
     *    const struct evbuffer_ptr *end);
     *struct evbuffer_ptr evbuffer_search_eol(struct evbuffer *buffer,
     *    struct evbuffer_ptr *start, size_t *eol_len_out,
     *    enum evbuffer_eol_style eol_style);
     */
    /* 0 copy
     * int evbuffer_reserve_space(struct evbuffer *buf, ev_ssize_t size,
     *    struct evbuffer_iovec *vec, int n_vecs);
     *int evbuffer_commit_space(struct evbuffer *buf,
     *    struct evbuffer_iovec *vec, int n_vecs);
     *typedef void (*evbuffer_ref_cleanup_cb)(const void *data,
     *    size_t datalen, void *extra);
     *int evbuffer_add_reference(struct evbuffer *outbuf,
     *    const void *data, size_t datlen,
     *    evbuffer_ref_cleanup_cb cleanupfn, void *extra);
     */
    /* Network IO with evbuffers
     * int evbuffer_write(struct evbuffer *buffer, evutil_socket_t fd);
     *int evbuffer_write_atmost(struct evbuffer *buffer, evutil_socket_t fd,
     *        ev_ssize_t howmuch);
     *int evbuffer_read(struct evbuffer *buffer, evutil_socket_t fd, int howmuch);
     */
    /* Evbuffers and callbacks
     *struct evbuffer_cb_entry;
     *struct evbuffer_cb_entry *evbuffer_add_cb(struct evbuffer *buffer,
     *    evbuffer_cb_func cb, void *cbarg);
     */
    /* Adding a file to an evbuffer
     * int evbuffer_add_file(struct evbuffer *output, int fd, ev_off_t offset,
     *    size_t length);
     * Fine-grained control with file segments
     * struct evbuffer_file_segment *evbuffer_file_segment_new(
     *        int fd, ev_off_t offset, ev_off_t length, unsigned flags);
     *void evbuffer_file_segment_free(struct evbuffer_file_segment *seg);
     *int evbuffer_add_file_segment(struct evbuffer *buf,
     *    struct evbuffer_file_segment *seg, ev_off_t offset, ev_off_t length);
     */
    /*
     * Adding an evbuffer to another by reference
     * int evbuffer_add_buffer_reference(struct evbuffer *outbuf,struct evbuffer *inbuf);
     */
    zerr_t Freeze(int at_front){return evbuffer_freeze(buf, at_front);}
    zerr_t Unfreeze(int at_front){return evbuffer_unfreeze(buf, at_front);}
private:
    evbuffer *buf;
    bool need_free;
};

/**
 * @brief Gives you a way to listen for and accept incomming TCP connections
 */
class Listener{
public:
    /**
     * @param flags LEV_OPT_LEAVE_SOCKETS_BLOCKING
     *              LEV_OPT_CLOSE_ON_FREE
     *              LEV_OPT_CLOSE_ON_EXEC
     *              LEV_OPT_REUSABLE
     *              LEV_OPT_THREADSAFE
     */
    Listener(struct event_base *base, evconnlistener_cb cb, void *ptr,
             unsigned flags, int backlog, evutil_socket_t fd){
        listener = evconnlistener_new(base, cb, ptr, flags, backlog, fd);
        if(!listener){
            zerrno(ZEFAIL);
        }
    }
    Listener(struct event_base *base, evconnlistener_cb cb, void *ptr,
             unsigned flags, int backlog, const struct sockaddr *sa, int salen){
        listener = evconnlistener_new_bind(base, cb, ptr, flags, backlog, sa, salen);
        if(!listener){
            zerrno(ZEFAIL);
        }
    }
    ~Listener(){
        if(listener){
            evconnlistener_free(listener);
        }
    }

    zerr_t Disable(){return evconnlistener_disable(listener);}
    zerr_t Enable(){return evconnlistener_enable(listener);}
public:
    void SetCb(evconnlistener_cb cb, void *arg){evconnlistener_set_cb(listener, cb, arg);}
    void SetErrorCb(evconnlistener_errorcb errorcb){
        evconnlistener_set_error_cb(listener, errorcb);
    }
    evutil_socket_t GetFd(){return evconnlistener_get_fd(listener);}
    struct event_base *GetBase(){return evconnlistener_get_base(listener);}
    
private:
    evconnlistener *listener;
};

ZNSE_LIBEV_CORE
ZNSE_LIBEV
#endif /*_ZEVENT2_CORE_H_*/

#if 0 /* samples*/
/**
 * bufferevent sample
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

// Example: Trivial HTTP v0 client.
/* Don't actually copy this code: it is a poor way to implement an
   HTTP client.  Have a look at evhttp instead.
*/
#include <event2/dns.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/event.h>

#include <stdio.h>

void readcb(struct bufferevent *bev, void *ptr)
{
    char buf[1024];
    int n;
    struct evbuffer *input = bufferevent_get_input(bev);
    while ((n = evbuffer_remove(input, buf, sizeof(buf))) > 0) {
        fwrite(buf, 1, n, stdout);
    }
}

void eventcb(struct bufferevent *bev, short events, void *ptr)
{
    if (events & BEV_EVENT_CONNECTED) {
         printf("Connect okay.\n");
    } else if (events & (BEV_EVENT_ERROR|BEV_EVENT_EOF)) {
         struct event_base *base = ptr;
         if (events & BEV_EVENT_ERROR) {
                 int err = bufferevent_socket_get_dns_error(bev);
                 if (err)
                         printf("DNS error: %s\n", evutil_gai_strerror(err));
         }
         printf("Closing\n");
         bufferevent_free(bev);
         event_base_loopexit(base, NULL);
    }
}

int main(int argc, char **argv)
{
    struct event_base *base;
    struct evdns_base *dns_base;
    struct bufferevent *bev;

    if (argc != 3) {
        printf("Trivial HTTP 0.x client\n"
               "Syntax: %s [hostname] [resource]\n"
               "Example: %s www.google.com /\n",argv[0],argv[0]);
        return 1;
    }

    base = event_base_new();
    dns_base = evdns_base_new(base, 1);

    bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, readcb, NULL, eventcb, base);
    bufferevent_enable(bev, EV_READ|EV_WRITE);
    evbuffer_add_printf(bufferevent_get_output(bev), "GET %s\r\n", argv[2]);
    bufferevent_socket_connect_hostname(
        bev, dns_base, AF_UNSPEC, argv[1], 80);
    event_base_dispatch(base);
    return 0;
}

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>

#include <stdlib.h>
#include <errno.h>
#include <string.h>

struct info {
    const char *name;
    size_t total_drained;
};

void read_callback(struct bufferevent *bev, void *ctx)
{
    struct info *inf = ctx;
    struct evbuffer *input = bufferevent_get_input(bev);
    size_t len = evbuffer_get_length(input);
    if (len) {
        inf->total_drained += len;
        evbuffer_drain(input, len);
        printf("Drained %lu bytes from %s\n",
             (unsigned long) len, inf->name);
    }
}

void event_callback(struct bufferevent *bev, short events, void *ctx)
{
    struct info *inf = ctx;
    struct evbuffer *input = bufferevent_get_input(bev);
    int finished = 0;

    if (events & BEV_EVENT_EOF) {
        size_t len = evbuffer_get_length(input);
        printf("Got a close from %s.  We drained %lu bytes from it, "
            "and have %lu left.\n", inf->name,
            (unsigned long)inf->total_drained, (unsigned long)len);
        finished = 1;
    }
    if (events & BEV_EVENT_ERROR) {
        printf("Got an error from %s: %s\n",
            inf->name, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
        finished = 1;
    }
    if (finished) {
        free(ctx);
        bufferevent_free(bev);
    }
}

struct bufferevent *setup_bufferevent(void)
{
    struct bufferevent *b1 = NULL;
    struct info *info1;

    info1 = malloc(sizeof(struct info));
    info1->name = "buffer 1";
    info1->total_drained = 0;

    /* ... Here we should set up the bufferevent and make sure it gets
       connected... */

    /* Trigger the read callback only whenever there is at least 128 bytes
       of data in the buffer. */
    bufferevent_setwatermark(b1, EV_READ, 128, 0);

    bufferevent_setcb(b1, read_callback, NULL, event_callback, info1);

    bufferevent_enable(b1, EV_READ); /* Start reading. */
    return b1;
}

// Example
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <ctype.h>

void
read_callback_uppercase(struct bufferevent *bev, void *ctx)
{
        /* This callback removes the data from bev's input buffer 128
           bytes at a time, uppercases it, and starts sending it
           back.

           (Watch out!  In practice, you shouldn't use toupper to implement
           a network protocol, unless you know for a fact that the current
           locale is the one you want to be using.)
         */

        char tmp[128];
        size_t n;
        int i;
        while (1) {
                n = bufferevent_read(bev, tmp, sizeof(tmp));
                if (n <= 0)
                        break; /* No more data. */
                for (i=0; i<n; ++i)
                        tmp[i] = toupper(tmp[i]);
                bufferevent_write(bev, tmp, n);
        }
}

struct proxy_info {
        struct bufferevent *other_bev;
};
void
read_callback_proxy(struct bufferevent *bev, void *ctx)
{
        /* You might use a function like this if you're implementing
           a simple proxy: it will take data from one connection (on
           bev), and write it to another, copying as little as
           possible. */
        struct proxy_info *inf = ctx;

        bufferevent_read_buffer(bev,
            bufferevent_get_output(inf->other_bev));
}

struct count {
        unsigned long last_fib[2];
};

void
write_callback_fibonacci(struct bufferevent *bev, void *ctx)
{
        /* Here's a callback that adds some Fibonacci numbers to the
           output buffer of bev.  It stops once we have added 1k of
           data; once this data is drained, we'll add more. */
        struct count *c = ctx;

        struct evbuffer *tmp = evbuffer_new();
        while (evbuffer_get_length(tmp) < 1024) {
                 unsigned long next = c->last_fib[0] + c->last_fib[1];
                 c->last_fib[0] = c->last_fib[1];
                 c->last_fib[1] = next;

                 evbuffer_add_printf(tmp, "%lu", next);
        }

        /* Now we add the whole contents of tmp to bev. */
        bufferevent_write_buffer(bev, tmp);

        /* We don't need tmp any longer. */
        evbuffer_free(tmp);
}

//Example code: an echo server.
//Example
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <arpa/inet.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

static void
echo_read_cb(struct bufferevent *bev, void *ctx)
{
        /* This callback is invoked when there is data to read on bev. */
        struct evbuffer *input = bufferevent_get_input(bev);
        struct evbuffer *output = bufferevent_get_output(bev);

        /* Copy all the data from the input buffer to the output buffer. */
        evbuffer_add_buffer(output, input);
}

static void
echo_event_cb(struct bufferevent *bev, short events, void *ctx)
{
        if (events & BEV_EVENT_ERROR)
                perror("Error from bufferevent");
        if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
                bufferevent_free(bev);
        }
}

static void
accept_conn_cb(struct evconnlistener *listener,
    evutil_socket_t fd, struct sockaddr *address, int socklen,
    void *ctx)
{
        /* We got a new connection! Set up a bufferevent for it. */
        struct event_base *base = evconnlistener_get_base(listener);
        struct bufferevent *bev = bufferevent_socket_new(
                base, fd, BEV_OPT_CLOSE_ON_FREE);

        bufferevent_setcb(bev, echo_read_cb, NULL, echo_event_cb, NULL);

        bufferevent_enable(bev, EV_READ|EV_WRITE);
}

static void
accept_error_cb(struct evconnlistener *listener, void *ctx)
{
        struct event_base *base = evconnlistener_get_base(listener);
        int err = EVUTIL_SOCKET_ERROR();
        fprintf(stderr, "Got an error %d (%s) on the listener. "
                "Shutting down.\n", err, evutil_socket_error_to_string(err));

        event_base_loopexit(base, NULL);
}

int
main(int argc, char **argv)
{
        struct event_base *base;
        struct evconnlistener *listener;
        struct sockaddr_in sin;

        int port = 9876;

        if (argc > 1) {
                port = atoi(argv[1]);
        }
        if (port<=0 || port>65535) {
                puts("Invalid port");
                return 1;
        }

        base = event_base_new();
        if (!base) {
                puts("Couldn't open event base");
                return 1;
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
                perror("Couldn't create listener");
                return 1;
        }
        evconnlistener_set_error_cb(listener, accept_error_cb);

        event_base_dispatch(base);
        return 0;
}
#endif /* samples */