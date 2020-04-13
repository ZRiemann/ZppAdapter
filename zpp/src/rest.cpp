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
#include <zsi/thread/thread.h>
#include <event2/buffer.h>
#include <zpp/event2/rest.h>

NSB_EV2
/******************************************************************************\
 * HTTP Request
 */
/*
 * tail queue access methods
 */
#define TAILQ_FIRST(head)       ((head)->tqh_first)
#define TAILQ_END(head)         NULL
#define TAILQ_NEXT(elm, field)      ((elm)->field.tqe_next)
#define TAILQ_LAST(head, headname)                          \
    (*(((struct headname *)((head)->tqh_last))->tqh_last))
/* XXX */
#define TAILQ_PREV(elm, headname, field)                        \
    (*(((struct headname *)((elm)->field.tqe_prev))->tqh_last))
#define TAILQ_EMPTY(head)                       \
    (TAILQ_FIRST(head) == TAILQ_END(head))

#define TAILQ_FOREACH(var, head, field)         \
    for((var) = TAILQ_FIRST(head);              \
        (var) != TAILQ_END(head);               \
        (var) = TAILQ_NEXT(var, field))

#define TAILQ_FOREACH_REVERSE(var, head, headname, field)   \
    for((var) = TAILQ_LAST(head, headname);                 \
        (var) != TAILQ_END(head);                           \
        (var) = TAILQ_PREV(var, headname, field))


static void done_cb(struct evhttp_request *req, void *arg){
    //zinf("request donw.");
    Request* request = (Request*)arg;
#if 0
    struct timeval tv = {1, 0};
    event_base_loopexit(request->base, &tv);
#else
    event_base_loopbreak(request->base);
#endif
}
static int header_cb(struct evhttp_request *req, void *arg){
    //zdbg("header_cb dump req:");
    //((Request*)arg)->Dump();
    return 0;
}

static void chunked_cb(struct evhttp_request *req, void *arg){
    Request *argreq = (Request*)arg;
    int len;
    //zdbg("chunk_cb dump req:");
    //argreq->Dump();
    struct evbuffer *input_buffer = evhttp_request_get_input_buffer(req);
    len = evbuffer_get_length(input_buffer);
    if(len){
        std::string cpy(len,'\0');
        evbuffer_copyout(input_buffer, (void*)cpy.data(), len);
        argreq->response.swap(cpy);
    }else{
        ztrace_org("input_buffer_len: %d\n", evbuffer_get_length(input_buffer));
    }
    argreq->errcode = evhttp_request_get_response_code(req);
    argreq->reason = evhttp_request_get_response_code_line(req);
}

static void error_cb(evhttp_request_error err, void *arg){
    const char *err_desc = "UNKNOWN";
    switch(err){
    case EVREQ_HTTP_TIMEOUT: err_desc = "EVREQ_HTTP_TIMEOUT"; break;
    case EVREQ_HTTP_EOF: err_desc = "EVREQ_HTTP_EOF"; break;
    case EVREQ_HTTP_INVALID_HEADER: err_desc = "EVREQ_HTTP_INVALID_HEADER"; break;
    case EVREQ_HTTP_BUFFER_ERROR: err_desc = "EVREQ_HTTP_BUFFER_ERROR"; break;
    case EVREQ_HTTP_REQUEST_CANCEL: err_desc = "EVREQ_HTTP_REQUEST_CANCEL"; break;
    case EVREQ_HTTP_DATA_TOO_LONG: err_desc = "EVREQ_HTTP_DATA_TOO_LONG"; break;
    default: break;
    }
    zerr("request error[%02x] %s", err, err_desc);
}

zerr_t Request::DumpKeyVals(struct evkeyvalq *kv){
    struct evkeyval *header;
    TAILQ_FOREACH(header, kv, next){
        ztrace_org("%s : %s\n", header->key, header->value);
    }
    return 0;
}

zerr_t Request::Dump(){
#if REST_DUMP
    if(req){
        const char *str_cmd = "UNKNOWN";
        int len;
        cmd = evhttp_request_get_command(req);
        switch(cmd){
        case EVHTTP_REQ_GET: str_cmd = "GET"; break;
        case EVHTTP_REQ_POST: str_cmd = "POST"; break;
        case EVHTTP_REQ_HEAD: str_cmd = "HEAD"; break;
        case EVHTTP_REQ_PUT: str_cmd = "PUT"; break;
        case EVHTTP_REQ_DELETE: str_cmd = "DELETE"; break;
        case EVHTTP_REQ_OPTIONS: str_cmd = "OPTIONS"; break;
        case EVHTTP_REQ_TRACE: str_cmd = "TRACE"; break;
        case EVHTTP_REQ_CONNECT: str_cmd = "CONNECT"; break;
        case EVHTTP_REQ_PATCH:str_cmd = "PATCH"; break;
        default: break;
        }
        zinf("dump request:");
        ztrace_org("uri:\t%s\n", evhttp_request_get_uri(req));
        /* dump query key-values */
        std::map<std::string, std::string>::iterator it;
        for(it = query.begin(); it != query.end(); ++it){
            ztrace_org("query<key:%s value:%s>\n",
                 it->first.c_str(),
                 it->second.c_str());
        }

        ztrace_org("cmd:\t0x%02x (%s)\n", cmd, str_cmd);
        ztrace_org("rep_code:\t%d\n", evhttp_request_get_response_code(req));
        ztrace_org("rep_code_line:\t%s\n", evhttp_request_get_response_code_line(req));
        ztrace_org("host:\t%s\n", evhttp_request_get_host(req));

        struct evkeyvalq *input_headers = evhttp_request_get_input_headers(req);
        ztrace_org("input_headers:\n");
        DumpKeyVals(input_headers);
        struct evkeyvalq *output_headers = evhttp_request_get_output_headers(req);
        ztrace_org("output_headers:\n");
        DumpKeyVals(output_headers);
        struct evbuffer *input_buffer = evhttp_request_get_input_buffer(req);
        len = evbuffer_get_length(input_buffer);
        if(len){
            std::string cpy(len,'\0');
            evbuffer_copyout(input_buffer, (void*)cpy.data(), len);
            ztrace_org("input_buffer_len: %d\n%s\n",
                 evbuffer_get_length(input_buffer),
                 cpy.c_str());
        }else{
            ztrace_org("input_buffer_len: %d\n", evbuffer_get_length(input_buffer));
        }
        struct evbuffer *output_buffer = evhttp_request_get_output_buffer(req);
        len = evbuffer_get_length(output_buffer);
        if(len){
            std::string cpy(len,'\0');
            evbuffer_copyout(output_buffer, (void*)cpy.data(), len);
            ztrace_org("output_buffer_len: %d\n%s\n",
                 evbuffer_get_length(output_buffer),
                 cpy.c_str());
        }else{
            ztrace_org("output_buffer_len: %d\n", evbuffer_get_length(output_buffer));
        }
    }
#endif
    return ZEOK;
}

zerr_t Request::ParseReq(){
    cmd = evhttp_request_get_command(req);
    uri = evhttp_request_get_uri(req);
    // init query value
    std::string key;
    std::string value;
    struct evkeyvalq kv;
    if(0 == evhttp_parse_query(uri, &kv)){
        struct evkeyval *header;
        TAILQ_FOREACH(header, &kv, next){
            ztrace_org("%s : %s\n", header->key, header->value);
            key = header->key;
            value = header->value;
            query[key] = value;
        }
    }else{
        zerrno(ZECAST_FAIL);
    }


    // zdbg("parse uri<%p>:%s", uri, uri);
    struct evbuffer *input_buffer = evhttp_request_get_input_buffer(req);
    int len = evbuffer_get_length(input_buffer);
    if(len){
        request.resize(len);
        if(-1 == evbuffer_copyout(input_buffer, (void*)request.data(), len)){
            zerrno(ZEFAIL);
            return ZEFAIL;
        }
    }
    return ZEOK;
}
zerr_t Request::AddHeader(const char *key, const char *val){
    if(!req){
        req = evhttp_request_new(done_cb, (zptr_t)this);
    }
    struct evkeyvalq *out_headers = evhttp_request_get_output_headers(req);
    if(NULL != evhttp_find_header(out_headers, key)){
        evhttp_remove_header(out_headers, key);
    }
    return evhttp_add_header(out_headers, key, val);
}
/******************************************************************************\
 * Rest Server
 */
static zthr_ret_t ZCALL proc_rest(void* param){
    RestServer *rest = (RestServer*)param;
    rest->Proc();
    return (zthr_ret_t)0;
}

static void rest_gencb(struct evhttp_request *req, void *hint){
    Request request;
    request.req = req;
    request.ParseReq();
    //zinf("rest gencb:");
    request.Dump();
    if(ZEOK != ((RestServer*)hint)->OnApiGeneral(request)){
        request.SendError(HTTP_INTERNAL, "http-internal-error");
    }
}

zerr_t RestServer::Run(uint16_t _port){
    if(!is_run){
        port = _port;
        is_run = true;
        return zthread_create(&tid, proc_rest, (zptr_t)this);
    }
    return ZE_EXIST;
}

zerr_t RestServer::Stop(){
    if(is_run){
        RestClient cli;
        Request req;

        is_run = false;
        req.request = "cmd-stop";
        req.uri="/api";
        cli.Connect("127.0.0.1", 8989);
        cli.Del(req);
        return zthread_join(&tid);
    }
    return ZENOT_EXIST;
}
void RestServer::Proc(){
    zmsg("rest server thread[%d] running...", zthread_self());
    evhttp_set_gencb(http, rest_gencb, (zptr_t)this);
    if(evhttp_bind_socket(http, "0.0.0.0", port)){
        zerr("Couldn't bind socket at 0.0.0.0:%d", port);
    }else{
        zinf("rest server lintening 0.0.0.0:%d", port);
    }
    zmsg("dispatch base<%p>", base);
    event_base_dispatch(base);
    zmsg("rest server thread[%d] exit now.", zthread_self());
}

/******************************************************************************\
 * Rest client
 */
RestClient::RestClient(){
    base = event_base_new();
    dns = evdns_base_new(base, EVDNS_BASE_INITIALIZE_NAMESERVERS |
                         EVDNS_BASE_DISABLE_WHEN_INACTIVE);
    conn = NULL;
}

RestClient::~RestClient(){
    evdns_base_free(dns, 0);
    event_base_free(base);
}

zerr_t RestClient::Connect(const char *address, uint16_t port){
    if(!conn){
        conn = evhttp_connection_base_bufferevent_new(base, dns, NULL, address, port);
        evhttp_connection_set_timeout(conn, 5/* sec */);
        return ZEOK;
    }
    return ZE_EXIST;
}

zerr_t RestClient::Disconnect(){
    if(conn){
        evhttp_connection_free(conn);
        conn = NULL;
        return ZEOK;
    }
    return ZENOT_EXIST;
}

zerr_t RestClient::Add(Request &req){
    return DispatchJson(req, EVHTTP_REQ_POST);
}
zerr_t RestClient::Get(Request &req){
    return DispatchJson(req, EVHTTP_REQ_GET);
}
zerr_t RestClient::Update(Request &req){
    return DispatchJson(req, EVHTTP_REQ_PUT);
}
zerr_t RestClient::Del(Request &req){
    return DispatchJson(req, EVHTTP_REQ_DELETE);
}

zerr_t RestClient::DispatchJson(Request &req, evhttp_cmd_type cmd){
    const char *str_cmd = "UNKNOWN";
    switch(cmd){
    case EVHTTP_REQ_GET: str_cmd = "GET"; break;
    case EVHTTP_REQ_POST: str_cmd = "POST"; break;
    case EVHTTP_REQ_HEAD: str_cmd = "HEAD"; break;
    case EVHTTP_REQ_PUT: str_cmd = "PUT"; break;
    case EVHTTP_REQ_DELETE: str_cmd = "DELETE"; break;
    case EVHTTP_REQ_OPTIONS: str_cmd = "OPTIONS"; break;
    case EVHTTP_REQ_TRACE: str_cmd = "TRACE"; break;
    case EVHTTP_REQ_CONNECT: str_cmd = "CONNECT"; break;
    case EVHTTP_REQ_PATCH:str_cmd = "PATCH"; break;
    default: break;
    }

    req.cmd = cmd;
    req.base = base;
    if(!req.req){
        if(NULL != (req.req = evhttp_request_new(done_cb, (zptr_t)&req))){
            evhttp_request_set_header_cb(req.req, header_cb);
            evhttp_request_set_chunked_cb(req.req, chunked_cb);
            evhttp_request_set_error_cb(req.req, error_cb);
        }else{
            return ZEMEM_INSUFFICIENT;
        }
    }

    zmsg("\n%s-Begin:%s\nrequest:%s", str_cmd, req.uri, req.request.c_str());
    req.MakeRequest(conn);
    req.AddHeader("Content-Type", "application/json");
    req.AddHeader("cache-control", "no-cache");
    req.AddHeader("User-Agent", "zpp/restful");
    req.AddHeader("Accept", "application/json");
    req.AddHeader("Connection", "keep-alive");
    req.AddOutBuffer(req.request);

    event_base_dispatch(base);
    // evhttp_cancel_request(req.req);
    // evhttp_request_free(req.req);
    req.req = NULL;
    req.base = NULL;
    zmsg("\n%s-Done:\n"
         "code: %d\n"
         "desc: %s\n"
         "reply: %s\n",
         str_cmd, req.errcode, req.reason.c_str(), req.response.c_str());
    return ZEOK;

}
NSE_EV2