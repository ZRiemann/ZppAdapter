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

#ifndef _ZEV2_REST_H_
#define _ZEV2_REST_H_

#include <string.h>
#include <string>
#include <map>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/dns.h>
#include <event2/keyvalq_struct.h>
#include <event2/http.h>
#include <zsi/base/type.h>
#include <zsi/base/error.h>
#include <zsi/base/trace.h>
#include <zsi/thread/thread.h>

#ifndef NSB_EV2
#define NSB_EV2 namespace z{ namespace ev2{
#define NSE_EV2 }}
#endif /* NSB_EV2 */

NSB_EV2

#define REST_DUMP 0

/**
 * @brief http request and response class
 * @par RestClient
 *      preset request: uri & request
 *      response: errcode & reason & response
 * @par RestServer
 *      request from client
 *      response: response date;
 */
class Request{
public:
    Request()
        :req(NULL)
        ,base(NULL){}
    ~Request(){}

    void GetReply(int &err_code, std::string &desc, std::string &rep){
        err_code = errcode;
        desc.swap(reason);
        rep.swap(response);
    }

    /**
     * @brief rest server send reply
     */
    zerr_t SendReply(int error_code, const char *desc, std::string &reply){
        if(req){
            struct evbuffer *buf = evbuffer_new();
            struct evkeyvalq *out_headers = evhttp_request_get_output_headers(req);
            evhttp_add_header(out_headers, "Content-Type", "application/json");
            evbuffer_add(buf, reply.data(), reply.length());
            evhttp_send_reply(req, error_code, desc, buf);
            evbuffer_free(buf);
            return ZEOK;
        }
        return ZENOT_SUPPORT;
    }

    zerr_t SendError(int error_code, const char *desc){
        if(req){
            evhttp_send_error(req, error_code, desc);
            return ZEOK;
        }
        return ZENOT_SUPPORT;
    }

    zerr_t SendOk(std::string &reply){
        if(req){
            struct evbuffer *buf = evbuffer_new();
            evbuffer_add(buf, reply.data(), reply.length());
            evhttp_send_reply(req, HTTP_OK, "OK", buf);
            evbuffer_free(buf);
            return ZEOK;
        }
        return ZENOT_SUPPORT;
    }

    /**
     * @brief check patch
     * @retval 0 equal
     * @retval 1 equal and has subpatch
     * @retval -1 not equal
     */
    int IsResource(const char *path){
        int uri_len = strlen(uri);
        int path_len = strlen(path);
        int i = 0;
        if(uri_len < path_len){
            //zdbg("uri_len:%d < path_len:%d", uri_len, path_len);
            return -1;
        }else if(uri_len == path_len){
            for(i = 0; i < uri_len; ++i){
                if(uri[i] != path[i]){
                    //zdbg("not equal");
                    return -1;
                }
            }
            //zdbg("equal");
            return 0;
        }else{
            for(i = 0; i < path_len; ++i){
                if(uri[i] != path[i]){
                    zdbg("not-equal");
                    return -1;
                }
            }
            //zdbg("equal + %c", uri[path_len]);
            return uri[path_len] == '/' ? 1 : 0;
        }
        //zdbg("bad compare");
        return -1;
    }
    zerr_t Dump();
    zerr_t DumpKeyVals(struct evkeyvalq *kv);
    zerr_t ParseReq();
    zerr_t AddHeader(const char *key, const char *val);
    zerr_t AddOutBuffer(std::string &data){
        if(data.size()){
            struct evbuffer *out_buf = evhttp_request_get_output_buffer(req);
            return evbuffer_add(out_buf, data.data(), data.length());
        }
        return ZENOT_EXIST;
    }
    zerr_t MakeRequest(evhttp_connection *conn){
        return evhttp_make_request(conn, req, cmd, uri);
    }
public:
    /* internal */
    struct evhttp_request *req;
    struct event_base *base;
    /* request */
    evhttp_cmd_type cmd; /** EVHTTP_REQ_GET/POST/HEAD/PUT/DELETE/... */
    evhttp_request_kind kind; /** EVHTTP_REQUEST, EVHTTP_RESPONSE */
    const char *uri; /** resource url */
    std::map<std::string, std::string> query; /* query key-value */
    std::string request; /** request data */
    /* response */
    int errcode; /** http error code, 200 403 ...*/
    std::string reason; /** error description */
    std::string response; /** response data, http body. empty means no body */
};
/**
 * @brief REST API Server
 * @par Sample
 *      uns::vms::RestSvr rest;
 *      rest.Run(8989);
 *      wait stop signal...
 *      rest.Stop();
 * @par User definition APIs
 *      class RestSvr : public sfx::ev2::RestServer{
 *        // REWRITE  OnApiGeneral according sample code
 *        virtual zerr_t OnApiGeneral(sfx::ev2::Request &req);
 *        // define more APIs
 *        zerr_t OnApiPathxxx(sfx::ev2::Request &req);
 *      };
 */
class RestServer{
public:
    RestServer(){
        base = event_base_new();
        http = evhttp_new(base);
        is_run = false;
    }
    virtual ~RestServer(){
        evhttp_free(http);
        event_base_free(base);
    }
    zerr_t Run(uint16_t port);
    zerr_t Stop();
    void Proc();
    /**
     * @brief Sub class rewrite the sample
     * zerr_t OnApixxx(Request &req);
     */
    virtual zerr_t OnApiGeneral(Request &req){
        if(0 == req.IsResource("/api/v1/test")){
            return OnApiTest(req);
        }
        zerrno(ZENOT_SUPPORT);
        return req.SendError(HTTP_NOTIMPLEMENTED, "http-error-not-implemented");
    }
    /*
     * zerr_t OnApixxx(Request &req);
     */
    zerr_t OnApiTest(Request &req){
        std::string ok("default ok");
        switch(req.cmd){
        case EVHTTP_REQ_GET:
            ok = "query ok: ";
            ok += req.uri;
            break;
        case EVHTTP_REQ_POST:
            ok = "create :";
            ok += req.request;
            break;
        case EVHTTP_REQ_PUT:
            ok = "update ok:";
            ok += req.uri;
            break;
        case EVHTTP_REQ_DELETE:
            ok = "delete ok:";
            ok += req.uri;
            break;
        default:
            break;
        }
        return req.SendOk(ok);
    }
protected:
    struct event_base *base;
    struct evhttp *http;
    uint16_t port;
    bool is_run;
    zthr_t tid;
};
/**
 * @brief REST API Client
 * @par Sample
 *      sfx::ev2::RestClient cli;
 *      cli.Connect("127.0.0.1", 8989);
 *      req.uri = "/api/v1/test";
 *      req.request = "{type:1,name:aaa}";
 *      cli.Add(req);
 */
class RestClient{
public:
    RestClient();
    ~RestClient();
    zerr_t Connect(const char *address, uint16_t port);
    zerr_t Disconnect();

    /**
     * @brief Add resource
     * @param req [in]  repset req.uri/req.request
     *            [out] reply req.errcode/req.reason/req.response
     */
    zerr_t Add(Request &req);
    zerr_t Get(Request &req);
    zerr_t Update(Request &req);
    zerr_t Del(Request &req);

public:
    zerr_t DispatchJson(Request &req, evhttp_cmd_type cmd);
public:
    event_base *base;
    evdns_base *dns;
    evhttp_connection *conn;
};

NSE_EV2

#endif /* _ZPP_HTTP_HPP_ */
