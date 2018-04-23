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
#ifndef _ZEVENT2_EXTRA_H_
#define _ZEVENT2_EXTRA_H_

/**
 * @file extra.h
 * @brief libevent 2.x extra API wrapper
 * @author Z.Riemann https://github.com/ZRiemann/
 * @date 2018-04-20 Z.Riemann found
 */
#include <zsi/base/type.h>
#include <zsi/base/error.h>
#include "core.h"
#include <event2/http.h>

ZNSB_LIBEV
ZNSB_LIBEV_EXTRA
class HttpServer{
public:
    HttpServer(struct event_base *base){http = evhttp_new(base);}
    ~HttpServer(){evhttp_free(http);}

    /** Bind an HTTP server on the specified address and port,
     *  Can be called muitiple times to band to multiple different ports.
     */
    zerr_t Bind(const char *address, ev_uint16_t port){return evhttp_bind_socket(http, address, port);}
    /**
     * Makes an HTTP server accept connections on the specified socket.
     * @param fd a socket fd that is ready for accepting connections
     * @return 0 on success, -1 on failure.
     */
    zerr_t Accept(evutil_socket_t fd){
        return evhttp_accept_socket(http, fd);
    }

    /**
     * With handle APIs
     */
    /**
     * Like Bind(), but returns a handle for referencing the socket.
     */
    struct evhttp_bound_socket *BindWithHandle(const char *address, ev_uint16_t port){
        return evhttp_bind_socket_with_handle(http, address, port);
    }
    struct evhttp_bound_socket *AcceptWithHandle(evutil_socket_t fd){
        return evhttp_accept_socket_with_handle(http, fd);
    }
    /**
     * The most low-level evhttp_bind/accept method: takes an evconnlistener
     */
    struct evhttp_bound_socket *BindListener(struct evconnlistener *listener){
        return evhttp_bind_listener(http, listener);
    }
    struct evconnlistener *GetListener(struct evhttp_bound_socket *bound){
        /** Get listener used to implement a bound socket */
        return evhttp_bound_socket_get_listener(bound);
    }
    /* typedef void evhttp_bound_socket_foreach_fn(struct evhttp_bound_socket *, void *); */
    void ForEachBound(evhttp_bound_socket_foreach_fn *fn, void *arg){
        evhttp_foreach_bound_socket(http, fn, arg);
    }
    void DelAcceptSocket(struct evhttp_bound_socket *bound){
        /** Stop accepting connections on the specified socket */
        evhttp_del_accept_socket(http, bound);
    }
    evutil_socket_t GetFd(struct evhttp_bound_socket *bound){
        return evhttp_bound_socket_get_fd(bound);
    }

    /**
     * XXX Document.
     */
    void SetMaxHeadersSize(ev_ssize_t max){
        evhttp_set_max_headers_size(http, max);
    }
    void SetMaxBodySize(ev_ssize_t max){
        evhttp_set_max_body_size(http, max);
    }
    void SetDevaultContentType(const char *content_type){
        evhttp_set_default_content_type(http, content_type);
    }
    /**
     * If not supported they will generate a "405 Method not allowed" response.
     * By default this includes the following methods: GET, POST, HEAD, PUT, DELETE
     * @param methods bit mask constructed from evhttp_cmd_type values
     */
    void SetAllowedMethods(ev_uint16_t methods){
        evhttp_set_allowed_methods(http, methods);
    }
    zerr_t SetCb(const char *path, void (*cb)(struct evhttp_request *, void *), void *arg){
        zerr_t ret = evhttp_set_cb(http, path, cb, arg);
        if(-1 == ret){
            ret = ZE_EXIST;
        }else if(-2 == ret){
            ret = ZEFAIL;
        }
        zerrno(ret);
        return ret;
    }
    zerr_t DelCb(const char *path){
        return evhttp_del_cb(http, path);
    }
    void SetGenCb(void (*cb)(struct evhttp_request *, void*), void *arg){
        /* GenericCb Set a callback for all request that
         * are not caught by specific callbacks */
        evhttp_set_gencb(http, cb, arg);
    }
    void SetBevCb(struct bufferevent *(*cb)(struct event_base *, void *), void *arg){
        /**
         * For example:
         * to make this evhttp object use SSL bufferevents rather than unencrypted
         * New bufferevents must be allocated with no fd set on them.
         */
        evhttp_set_bevcb(http, cb, arg);
    }
    void SetNewReqCb(int (*cb)(struct evhttp_request *, void *), void *arg){
        /**
         * Set a callback which allows the user to note or throttle incoming requests.
         */
        evhttp_set_newreqcb(http, cb, arg);
    }
    zerr_t AddVhost(const char *pattern, struct evhttp *vhost){
        return evhttp_add_virtual_host(http, pattern, vhost);
    }
    zerr_t RemVhost(struct evhttp *vhost){
        return evhttp_remove_virtual_host(http, vhost);
    }
    zerr_t AddSvrAlias(const char *alias){
        return evhttp_add_server_alias(http, alias);
    }
    zerr_t RemSvrAlias(const char *alias){
        return evhttp_remove_server_alias(http, alias);
    }
    void SetTimeout(int secs){ /* Set the timeout for an HTTP request */
        evhttp_set_timeout(http, secs);
        /* evhttp_set_timeout_tv(http, (timeval*)tv)*/
    }
    zerr_t SetFlags(int flags/* EVHTTP_SERVER_* */){
        return evhttp_set_flags(http, flags);
    }
private:
    struct evhttp *http;
};

/**
 * @brief HTTP request wrapper
 *
 * @par message structure
 *      HTTP/1.1 200 OK             ; start line
 *      Contect-type: text/plain    ; header
 *      Content-length: 19          ;
 *                                  ; \r\n
 *      Hi! I'm a message!          ; body
 *                                  ; \r\n
 *                                  ; \r\n
 * @par request message
 *      <method> <request-URL> <version>
 *      <headers>
 *
 *      <entity-body>
 *
 * @par response message
 *      <version> <status> <reason-phrase>
 *      <headers>
 *
 *      <entity-body>
 *
 * @par general headers
 *      Accept-Encoding/Language/Charset/Ranges
 *      Range: bytes=1000-2000 / 1000- /1000
 * @par response headers
 *      Cache-Control:no-store/no-cache/mast-revalidate
 */
class HttpRequest{
public:
    HttpRequest(struct evhttp_request *_req, bool _free=true)
        :req(_req), need_free(_free){}
    ~HttpRequest(){
        if(need_free){
            evhttp_request_free(req);
        }
    }

    /**
     * Request/Response functionality
     */
    void SendError(int error, const char *reson){
        evhttp_send_error(req, error, reson);
    }
    void SentReply(int code, const char *reason,
                   struct evbuffer *databuf){
        evhttp_send_reply(req, code, reason, databuf);
    }

    /**
     * Low-level response interface, for streaming/chunked replies
     */
    void SendReplyStart(int code, const char *reason){
        evhttp_send_reply_start(req, code, reason);
    }
    void SendReplyChunk(struct evbuffer *databuf){
        evhttp_send_reply_chunk(req, databuf);
    }
    void SendReplyChunkWithCb(struct evbuffer *buf, void (*cb)(struct evhttp_connection *, void *), void *arg){
        evhttp_send_reply_chunk_with_cb(req, buf, cb, arg);
    }
    void SendReplyEnd(){
        evhttp_send_reply_end(req);
    }

    /**
     * Client
     */
    HttpRequest(void (*cb)(struct evhttp_request *, void *), void *arg){
        req = evhttp_request_new(cb, arg);
    }
    void SetChunkCb(void (*cb)(struct evhttp_request *, void *)){
        evhttp_request_set_chunked_cb(req, cb);
    }
    void SetHeaderCb(int (*cb)(struct evhttp_request *, void *)){
        /* It allows analyzing the header and possibly closing the connection
         * by returning a value < 0.
         */
        evhttp_request_set_header_cb(req, cb);
    }
    void SetErrorCb(void (*cb)(enum evhttp_request_error, void *)){
        evhttp_request_set_error_cb(req, cb);
    }
    void SetOnCompleteCb(void(*cb)(struct evhttp_request *, void *), void *arg){
        evhttp_request_set_on_complete_cb(req, cb, arg);
    }

    void Own(){
        /** Takes ownership of the request object
         *
         * Can be used in a request callback to keep onto the request until
         * evhttp_request_free() is explicitly called by the user.
         */
        evhttp_request_own(req);
    }
    bool IsOwned(){
        return evhttp_request_is_owned(req) == 1 ? true : false;
    }
    struct evhttp_connection *GetConnection(){
        return evhttp_request_get_connection(req);
    }

    void Cancel(){
        need_free = false;
        evhttp_cancel_request(req);
    }

    /**
     * URI RFC3986
     */
    const char *GetURI(){
        return evhttp_request_get_uri(req);
    }
    const struct evhttp_uri *GetURIx(){
        return evhttp_request_get_evhttp_uri(req);
    }
    enum evhttp_cmd_type GetCmd(){
        return evhttp_request_get_command(req);
    }
    int GetResponseCode(){
        return evhttp_request_get_response_code(req);
    }
    const char *GetResponseCodeLine(){
        return evhttp_request_get_response_code_line(req);
    }

    /* input/output headers */
    struct evkeyvalq *GetInputHeaders(){
        return evhttp_request_get_input_headers(req);
    }
    struct evkeyvalq *GetOutputHeaders(){
        return evhttp_request_get_output_headers(req);
    }
    struct evbuffer *GetInputBuffer(){
        return evhttp_request_get_input_buffer(req);
    }
    struct evbuffer *GetOutputBuffer(){
        return evhttp_request_get_output_buffer(req);
    }
    const char *GetHost(){
        return evhttp_request_get_host(req);
    }
private:
    bool need_free;
    struct evhttp_request *req;
};

/**
 * @brief wrapper Uniform Resource Identifier
 *
 * @par URI include URL(URLocation)/URN()
 *
 * @par URL
 *      URI-Reference                         relative-refs
 *      scheme://[[userinfo]@]foo.com[:port]]/[path][?query][#fragment]
 *      relative-refs
 *      <scheme>://<user>:<pwd>@<host>:<prot>/<path>;<params>?<query>#<frag>
 *      http    ://user:pwd@127.0.0.1:80/api;params?key=value#fragment
 *      https
 *                            ;路径参数
 *      ftp://prep.edu/pub/gnu;type=d
 *      http://www.joes.com/hammers;sale=false/index.html;graphics=true
 *      ?查询组件
 *      http://www.joes.com/inventory-check.cgi?item=12731&color=blue&size=large
 *      #片段(引用资源的部分，浏览器获取整个但显示片段部分)，从drills片段出开始显示
 *      http://www.joes-hardward.com/tools.html#drills
 */
class HttpUri{
public:
    HttpUri(){
        uri = evhttp_uri_new();
    }
    HttpUri(const char *source_uri, unsigned flags = 0/*EVHTTP_URI_NONCONFORMANT*/){
        uri = evhttp_uri_parse_with_flags(source_uri, flags);
        if(NULL == uri){
            zerrno(ZEPARAM_INVALID);
        }
    }
    ~HttpUri(){
        evhttp_uri_free(uri);
    }

    char *Join(char *buf, size_t limit){
        return evhttp_uri_join(uri, buf, limit);
    }
    void SetFlags(unsigned flags){
        evhttp_uri_set_flags(uri, flags);
    }
    /**
     * Set URI attributes
     */
    zerr_t SetScheme(const char *scheme){
        return evhttp_uri_set_scheme(uri, scheme);
    }
    zerr_t SetUserInfo(const char *userinfo){
        return evhttp_uri_set_userinfo(uri, userinfo);
    }
    zerr_t SetHost(const char *host){
        return evhttp_uri_set_host(uri, host);
    }
    zerr_t SetPort(int port){
        return evhttp_uri_set_port(uri, port);
    }
    zerr_t SetPath(const char *path){
        return evhttp_uri_set_path(uri, path);
    }
    zerr_t SetQuery(const char *query){
        return evhttp_uri_set_query(uri, query);
    }
    zerr_t SetFragment(const char *fragment){
        return evhttp_uri_set_fragment(uri, fragment);
    }
    /**
     * Get URI attributes
     */
    const char *GetScheme(){
        return evhttp_uri_get_scheme(uri);
    }
    const char *GetUserInfo(){
        return evhttp_uri_get_userinfo(uri);
    }
    /**
     * Return the host part of an evhttp_uri, or NULL if it has no host set.
     * The host may either be a regular hostname (conforming to the RFC 3986
     * "regname" production), or an IPv4 address, or the empty string, or a
     * bracketed IPv6 address, or a bracketed 'IP-Future' address.
     *
     * Note that having a NULL host means that the URI has no authority
     * section, but having an empty-string host means that the URI has an
     * authority section with no host part.  For example,
     * "mailto:user@example.com" has a host of NULL, but "file:///etc/motd"
     * has a host of "".
     */
    const char *GetHost(){
        return evhttp_uri_get_host(uri);
    }
    int GetPort(){
        return evhttp_uri_get_port(uri);
    }
    const char *GetPath(){
        return evhttp_uri_get_path(uri);
    }
    const char *GetQuery(){
        /** Return the query part of an evhttp_uri (excluding the leading "?"), or
         * NULL if it has no query set */
        return evhttp_uri_get_query(uri);
    }
    const char *GetFragment(){
        /** Return the fragment part of an evhttp_uri (excluding the leading "#"),
         * or NULL if it has no fragment set */
        return evhttp_uri_get_fragment(uri);
    }
public:
    /* Miscellaneous utility functions
     * THE RETURN STRING MAST BE FREED BY THE CALLER
     */
    static char *Encode(const char *str){
        return evhttp_encode_uri(str);
    }
    static char *Encodex(const char *str, ev_ssize_t size,
                            int space_to_plus){
        return evhttp_uriencode(str, size, space_to_plus);
    }
    static char *Decode(const char *uri){
        return evhttp_decode_uri(uri);
    }
    static char *Decodex(const char *uri, int decode_plus, size_t *size_out){
        return evhttp_uridecode(uri, decode_plus, size_out);
    }
    /**
     * Escape HTML character entities in a string.
     *
     * Replaces <, >, ", ' and & with &lt;, &gt;, &quot;,
     * &#039; and &amp; correspondingly.
     *
     * The returned string needs to be freed by the caller.
     *
     * @param html an unescaped HTML string
     * @return an escaped HTML string or NULL on error
     */
    static char *HtmlEscape(const char *html){
        return evhttp_htmlescape(html);
    }
private:
    struct evhttp_uri *uri;
};
/**
 * HTTP key value data
 */
class HttpKeyVal{
public:
    HttpKeyVal(struct evkeyvalq *kvq)
        :kv(kvq){}
    ~HttpKeyVal(){}

    const char *FindHeader(const char *key){
        return evhttp_find_header(kv, key);
    }
    zerr_t RemHeader(const char *key){
        return evhttp_remove_header(kv, key);
    }
    zerr_t AddHeader(const char *key, const char *value){
        return evhttp_add_header(kv, key, value);
    }
    void ClearHeaders(){
        evhttp_clear_headers(kv);
    }
private:
    struct evkeyvalq *kv;
};

class HttpClient{
public:
    HttpClient(struct event_base *base, struct evdns_base *dnsbase, struct bufferevent *bev,
               const char *address, ev_uint16_t port){
        conn = evhttp_connection_base_bufferevent_new(base, dnsbase, bev, address, port);
    }
    HttpClient(struct event_base *base, struct evdns_base *dnsbase,
               const char *address, ev_uint16_t port){
        conn = evhttp_connection_base_new(base, dnsbase, address, port);
    }
    ~HttpClient(){
        evhttp_connection_free(conn);
    }

    struct bufferevent *GetBufferevent(){
        return evhttp_connection_get_bufferevent(conn);
    }
    /** struct evhttp *evhttp_connection_get_server(conn)*/
    void SetFamily(int familly){
        /* Set family hint for DNS request. */
        evhttp_connection_set_family(conn, familly);
    }
    zerr_t SetFlags(int flags){
        /* EVHTTP_CON_*(REUSE_CONNECTED_ADDR/READ_ON_WRITE_ERROR/LINGERING_CLOSE
         * PUBLIC_FLAGS_END) */
        evhttp_connection_set_flags(conn, flags);
    }
    struct event_base *GetBase(){
        return evhttp_connection_get_base(conn);
    }
    void SetMaxHeadersSize(ev_ssize_t max){
        evhttp_connection_set_max_headers_size(conn, max);
    }
    void SetMaxBodySize(ev_ssize_t max){
        evhttp_connection_set_max_body_size(conn, max);
    }

    void FreeOnCompletion(){
        /* Can be used to tell libevent to free the connection object after
         * the last request has completed or failed.
         */
        evhttp_connection_free_on_completion(conn);
    }
    void SetLocalAddress(const char *address){
        /** Sets the ip address from which http connection are make */
        evhttp_connection_set_local_address(conn, address);
    }
    void SetLocalPort(ev_uint16_t port){
        evhttp_connection_set_local_port(conn, port);
    }
    void SetTimeout(int secs){
        evhttp_connection_set_timeout(conn, secs);
    }
    void SetRetries(int retry_max, int secs){
        struct timeval tv = {secs, 0};
        evhttp_connection_set_retries(conn, retry_max);
        evhttp_connection_set_initial_retry_tv(conn, &tv);
    }
    void SetCloseCb(void (*cb)(struct evhttp_connection *, void *), void *arg){
        evhttp_connection_set_closecb(conn, cb, arg);
    }
    void GetPeer(char **address, ev_uint16_t *port){
        evhttp_connection_get_peer(conn, address, port);
    }
    const struct sockaddr *GetAddr(){
        return evhttp_connection_get_addr(conn);
    }

    zerr_t MakeRequest(struct evhttp_request *req, enum evhttp_cmd_type type, const char *uri){
        return evhttp_make_request(conn, req, type, uri);
    }
private:
    struct evhttp_connection *conn;
};
ZNSE_LIBEV_EXTRA
ZNSE_LIBEV
#endif /*_ZEVENT2_EXTRA_H_*/
