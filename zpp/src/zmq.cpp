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
 * @file zmq.cpp
 * @brief ZeroMQ cpp wrapper implement
 * @author Zwp
 * @date 2017-10-24
 */
#include <zsi/base/time.h>
#include <zsi/base/trace.h>
#include <zpp/zmq.hpp>

ZNS_ZMQB

zptr_t Socket::s_ctx;
zatm32_t Socket::total;

zerr_t Socket::InitCtx(){
    if(NULL == s_ctx){
        s_ctx = zmq_ctx_new();
        /* set linger timeout 0 */
        zdbg("set linger timeout 0");
        zmq_ctx_set(s_ctx, ZMQ_BLOCKY, false);
    }
}

zerr_t Socket::FiniCtx(){
    zdbg("Begin destroy ZeroMQ context...");
    if(-1 == zmq_ctx_destroy(s_ctx)){
        zmq_error();
    }
    zdbg("Destroy ZeroMQ context down.");
}

zerr_t Socket::SetCtx(zptr_t ctx){
    zdbg("set zmq context<%p>", ctx);
    s_ctx = ctx;
}

Socket::Socket(int type){
    if(NULL == (sock = zmq_socket(s_ctx, type))){
        zmq_error();
    }
    zatm_inc(&total);
    zdbg("sock<%p type: %d total:%d>",
                sock, type, total);
}

Socket::~Socket(){
    for(int i = 0; i < conns.size(); ++i){
        delete conns[i];
        zdbg("delete conn_info[%d]", i);
    }
    zatm_dec(&total);
    zdbg("close sock<%p total:%d>",
                sock, total);
    if(sock && (-1 == zmq_close(sock))){
        zmq_error();
    }
}

zerr_t Socket::Close(){
    if(-1 == zmq_close(sock)){
        zmq_error();
    }
    zdbg("close socket<%p>", sock);
    sock = NULL;
}
zerr_t Socket::Bind(const char *endpoint){
    if(-1 == zmq_bind(sock, endpoint)){
        zmq_error();
        zdbg("bind<endp:%s> FAILED", endpoint);
        return ZEFAIL;
    }
    zdbg("bind<endp:%s>", endpoint);
    return ZEOK;
}

zerr_t Socket::Unbind(const char *endpoint){
    if(-1 == zmq_unbind(sock, endpoint)){
        zmq_error();
        zdbg("unbind<endp:%s> FAILED", endpoint);
        return ZEFAIL;
    }
    zdbg("unbind<endp:%s>", endpoint);
    return ZEOK;
}
zerr_t Socket::Connect(const char *endpoint){
    if(-1 == zmq_connect(sock, endpoint)){
        zmq_error();
        return ZEFAIL;
    }else{
        Conn(endpoint, NULL, 0);
        zdbg("connect<endp:%s>", endpoint);
    }
    return ZEOK;
}

zerr_t Socket::Connect(const char *endpoint, const char *id, int len){
    if(-1 == zmq_connect(sock, endpoint)){
        zmq_error();
        return ZEFAIL;
    }else{
        Conn(endpoint, id, len);
        ztrace_org("connect<endp:%s id: ", endpoint);
        ztrace_bin(id, len);
        ztrace_org(">\n");
    }
    return ZEOK;
}

zerr_t Socket::Disconnect(const char *endpoint){
    if(-1 == zmq_disconnect(sock, endpoint)){
        zmq_error();
        return ZEFAIL;
    }else{
        Disconn(endpoint, NULL, 0);
        zdbg("disconnect<endp:%s>", endpoint);
    }
    return ZEOK;
}

zerr_t Socket::GetOpt(int option_name, void *option_value, size_t *option_len){
    if(-1 == zmq_getsockopt(sock, option_name, option_value, option_len)){
        zmq_error();
        return ZEFAIL;
    }
    return ZEOK;
}
zerr_t Socket::SetOpt(int option_name, const void *option_value, size_t option_len){
    if(-1 == zmq_setsockopt(sock, option_name, option_value, option_len)){
        zmq_error();
        return ZEFAIL;
    }
    return ZEOK;
}

zerr_t Socket::SetHwm(int is_send, int hwm){
    return SetOpt(is_send ? ZMQ_SNDHWM : ZMQ_RCVHWM, &hwm, sizeof(hwm));
}

zerr_t Socket::SetLinger(int linger){
    return SetOpt(ZMQ_LINGER, &linger, sizeof(linger));
}

zerr_t Socket::SetId(const char *id, int len){
    std::string str(id, len);
    zdbg("set socket<id: %s len: %d>", str.c_str(), str.size());
    return SetOpt(ZMQ_IDENTITY, id, len);
}

zerr_t Socket::SetId(std::string &id){
    zdbg("set socket<id: %s len: %d>", id.c_str(), id.size());
    return SetOpt(ZMQ_IDENTITY, id.data(), id.size());
}

zerr_t Socket::LazyPiratReq(Message &req, Message &rep, int timeout_ms, int trys){
    int ret = ZETIMEOUT;
    int rc;

    do{
        zmq_pollitem_t items[]={{sock, 0, ZMQ_POLLIN, 0}};
        /* read old date first */
        rc = zmq_poll(items, 1, 0);
        if(rc > 0){
            Message msg;
            if( ZEOK != (ret = Recv(msg))){
                break;
            }
        }
        if(ZEOK != (ret = Send(req))){
            break;
        }

        rc = zmq_poll(items, 1, timeout_ms);
        if(rc > 0){
            /* reply */
            ret = Recv(rep);
            break;
        }else if(0 == rc){
            /* timeout */
        }else{
            zmq_error();
        }
        Reconnect();
    }while(--trys);
    zerrno(ret);
    return ret;
}

zerr_t Socket::Reconnect(){
    zerr_t ret = ZEOK;
    int size = conns.size();

    if(size){
        ConnInfos endp = conns;
        zmq_close(sock);
        sock = zmq_socket(s_ctx, sock_type);
        if(NULL == sock){
            zmq_error();
            return ZEFAIL;
        }else{
            SetLinger(0);
        }

        conns.clear(); // avoid mulit push
        do{
            Connect(endp[size-1]->endp.c_str());
            delete endp[size-1];
        }while(--size);
        zsleepsec(1); // wait connection establish
    }
    zerrno(ret);
    return ret;
}

zerr_t Socket::SetConnStat(int stat, void *id, int len){
    zerr_t ret = ZEOK;
    std::string str((char*)id, len);
    for(int i = 0; i < conns.size(); ++i){
        if(0 == conns[i]->id.compare(str)){
            conns[i]->stat = stat;
            ret = ZEOK;
            break;
        }
    }
    zerrno(ret);
    return ret;
}
zerr_t Socket::SetConnStat(int stat, const char *endp){
    zerr_t ret = ZENOT_EXIST;
    std::string str(endp);
    for(int i = 0; i < conns.size(); ++i){
        if(0 == conns[i]->endp.compare(str)){
            conns[i]->stat = stat;
            ret = ZEOK;
            break;
        }
    }
    zerrno(ret);
    return ret;
}

zerr_t Socket::Conn(const char *endpoint, const char *id, int len){
    zerr_t ret = ZEOK;
    bool exist = false;
    for(int i = 0; i < conns.size(); ++i){
        if(0 == conns[i]->endp.compare(endpoint)){
            exist = true;
            break;
        }
    }
    if(!exist){
        ConnInfo *cnn = new(std::nothrow) ConnInfo(endpoint);
        if(!cnn){
            ret = ZEMEM_INSUFFICIENT;
            zerrno(ret);
        }else{
            if(id){
                cnn->id.assign(id, len);
            }
            conns.push_back(cnn);
        }
    }
    return ret;
}

zerr_t Socket::Disconn(const char *endpoint, const char *id, int len){
    zerr_t ret = ZEOK;
    for(int i = 0; i < conns.size(); ++i){
        if(0 == conns[i]->endp.compare(endpoint)){
            delete conns[i];
            conns.erase(conns.begin()+i);
            break;
        }
    }
}

zerr_t Socket::SubAll(){
    return SetOpt(ZMQ_SUBSCRIBE, "", 0);
}

zerr_t Socket::Subscribe(const char *filter, int len){
    return SetOpt(ZMQ_SUBSCRIBE, filter, len);
}

zerr_t Socket::Unsubscribe(const char *filter, int len){
    return SetOpt(ZMQ_UNSUBSCRIBE, filter, len);
}

zerr_t Socket::Monitor(const char *endpoint, int events){
    zdbg("monitor<endp: %s>", endpoint);
    return zmq_socket_monitor(sock, endpoint, events);
}

void Socket::DumpEvent(uint16_t event, uint32_t fd, const char *addr){
    switch(event){
    case ZMQ_EVENT_CONNECTED:
        zdbg("connected<fd: %d addr: %s>\n", fd,
                    addr ? addr : "...");
        break;
    case ZMQ_EVENT_CONNECT_DELAYED:
        zdbg("delayed<fd: %d addr: %s>\n", fd,
                    addr ? addr : "...");
        break;
    case ZMQ_EVENT_CONNECT_RETRIED:
        zdbg("connect_retried<fd: %d addr: %s>\n", fd,
                    addr ? addr : "...");
        break;
    case ZMQ_EVENT_LISTENING:
        zdbg("listening<fd: %d addr: %s>\n", fd,
                    addr ? addr : "...");
        break;
    case ZMQ_EVENT_BIND_FAILED:
        zdbg("<bind_failed<fd: %d addr: %s>\n", fd,
                    addr ? addr : "...");
        break;
    case ZMQ_EVENT_ACCEPTED:
        zdbg("accepted<fd: %d addr: %s>\n", fd,
                    addr ? addr : "...");
        break;
    case ZMQ_EVENT_ACCEPT_FAILED:
        zdbg("accept_faile<fd: %d addr: %s>\n", fd,
                    addr ? addr : "...");
        break;
    case ZMQ_EVENT_CLOSED:
        zdbg("closed<fd: %d addr: %s>\n", fd,
                    addr ? addr : "...");
        break;
    case ZMQ_EVENT_CLOSE_FAILED:
        zdbg("close_failed<fd: %d addr: %s>\n", fd,
                    addr ? addr : "...");
        break;
    case ZMQ_EVENT_DISCONNECTED:
        zdbg("disconnected<fd: %d addr: %s>\n", fd,
                    addr ? addr : "...");
        break;
    case ZMQ_EVENT_MONITOR_STOPPED:
        zdbg("monitor_stoped<fd: %d addr: %s>\n", fd,
                    addr ? addr : "...");
        break;
    }
}

ZNS_ZMQE