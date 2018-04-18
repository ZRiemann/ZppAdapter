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

#ifndef _ZPP_ZMQ_HPP_
#define _ZPP_ZMQ_HPP_
/**
 * @file zmq.hpp
 * @brief ZeroMQ cpp wrapper
 * @author Zwp
 * @date 2017-10-24
 */
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <string>
#include <zsi/base/type.h>
#include <zsi/base/error.h>
#include <zsi/base/trace.h>
#include <zsi/base/atomic.h>
#define ZMQ_BUILD_DRAFT_API
#include <zmq.h>

//#define zmq_error() zerr("%s", zmq_strerror(zmq_errno()))
#define ENABLE_DUMP 1

#define ZNS_ZMQB namespace z{ namespace zmq{
#define ZNS_ZMQE }}

ZNS_ZMQB
/**
 * @brief ZeroMQ 消息类封装
 */
class Message{
public:
    Message():need_close(true){
    }
    ~Message(){
        /* NOT necessary after a successful zmq_msg_send().*/
        if(need_close && (-1 == zmq_msg_close(&msg))){
            zmq_error();
        }
    }
    err_t Init(){
        if(-1 == zmq_msg_init(&msg)){
            zmq_error();
            return ZEFAIL;
        }
        return ZEOK;
    }
    err_t Init(size_t size){
        if(-1 == zmq_msg_init_size(&msg, size)){
            zmq_error();
            return ZEFAIL;
        }
        return ZEOK;
    }
    err_t Init(const void *data, size_t size){
        if(-1 == zmq_msg_init_size(&msg, size)){
            zmq_error();
            return ZEFAIL;
        }else{
            memcpy(zmq_msg_data(&msg), data, size);
        }
        return ZEOK;
    }
    err_t Init(void *data, size_t size, zmq_free_fn *ffn, void *hint){
        if(-1 == zmq_msg_init_data(&msg, data, size, ffn, hint)){
            zmq_error();
            return ZEFAIL;
        }
        return ZEOK;
    }
    err_t Init(std::string &str){
        if(-1 == zmq_msg_init_size(&msg, str.size())){
            zmq_error();
            return ZEFAIL;
        }else{
            memcpy(zmq_msg_data(&msg), str.data(), str.size());
        }
        return ZEOK;
    }
    err_t Init(zmq_msg_t &src_msg){
        /* copy the message */
        zmq_msg_init(&msg);
        if(-1 == zmq_msg_copy(&msg, &src_msg)){
            zmq_error();
            return ZEFAIL;
        }
        return ZEOK;
    }
    err_t Init(zmq_msg_t *src_msg){
        if(src_msg){
            msg = *src_msg; /* not copy */
        }
        return ZEOK;
    }
    err_t Move(Message &src){
        if(-1 == zmq_msg_move(&msg, &src.msg)){
            zmq_error();
            return ZEFAIL;
        }
        return ZEOK;
    }

    err_t Copy(Message &src){
        if(-1 == zmq_msg_copy(&msg, &src.msg)){
            zmq_error();
            return ZEFAIL;
        }
        return ZEOK;
    }
    int IsMore(){
        return zmq_msg_more(&msg);
    }

    void *Data(){
        return zmq_msg_data(&msg);
    }

    size_t Size(){
        return zmq_msg_size(&msg);
    }

    void Dump(bool is_recv = true){
#if ENABLE_DUMP
        size_t len = 0;
        len = zmq_msg_size(&msg);
        zdbg("%s<len: %d data: ", is_recv ? "recv" : "send", len);
        if(len < 256){
            ztrace_bin((char*)zmq_msg_data(&msg), len);
        }
        ztrace_org(">\n");
#endif
    }
    /*
      zmq_msg_t GetMsg(){
      //need_close = false;
      return msg;
      }
    */
    int GetFd(){
        return zmq_msg_get(&msg, ZMQ_SRCFD);
    }
    bool need_close;
protected:
    zmq_msg_t msg;
    friend class Socket;
    friend class RoutePath;
    friend class MsgStack;
};

typedef std::vector<zmq_msg_t> Msgs;
class MsgStack{
public:
    MsgStack()
        :status(0)
        ,fd(-1){
    }
    ~MsgStack(){
        if(!(status & 1)){
            while(!msgs.empty()){
                zmq_msg_close(&msgs.back());
                msgs.pop_back();
            }
        }
    }
    err_t Push(const void *data, size_t len,
               zmq_free_fn fn = NULL, void *hint = NULL){
        err_t ret = ZEOK;
        zmq_msg_t msg;

        if(fn){
            ret = data ? zmq_msg_init_data(&msg, (void*)data, len, fn, hint) :
                ZEPARAM_INVALID;
        }else{
            if(data){
                ret = zmq_msg_init_size(&msg, len);
                if(0 == ret){
                    memcpy(zmq_msg_data(&msg), data, len);
                }
            }else if(len){
                ret = zmq_msg_init_size(&msg, len);
            }else{
                ret = zmq_msg_init(&msg);
            }
        }

        if(0 == ret){
            msgs.push_back(msg);
        }
        return ret;
    }

    err_t Push(std::string &str){
        err_t ret = ZEOK;
        zmq_msg_t msg;
        ret = zmq_msg_init_size(&msg, str.size());
        if(0 == ret){
            memcpy(zmq_msg_data(&msg), str.data(), str.size());
            msgs.push_back(msg);
        }
        return ret;
    }
    /* 非拷贝压栈 */
    err_t Push(zmq_msg_t &msg){
        msgs.push_back(msg);
        return ZEOK;
    }
    /* 拷贝压栈 */
    err_t Push(zmq_msg_t *msg){
        zmq_msg_t msg_copy;
        zmq_msg_init(&msg_copy);
        zmq_msg_copy(&msg_copy, msg);
        msgs.push_back(msg_copy);
        return ZEOK;
    }

    err_t Push(Message &msg){
        zmq_msg_t msg_copy;
        zmq_msg_init(&msg_copy);
        zmq_msg_copy(&msg_copy, &msg.msg);
        msgs.push_back(msg_copy);
        return ZEOK;
    }

    err_t Push(MsgStack &msg){
        zmq_msg_t msg_copy;
        for(size_t i = 0; i < msg.msgs.size(); ++i){
            zmq_msg_init(&msg_copy);
            zmq_msg_copy(&msg_copy, &msg.msgs[i]);
            msgs.push_back(msg_copy);
        }
        return ZEOK;
    }

    err_t Push(){
        /* push 0 length message */
        zmq_msg_t msg;
        zmq_msg_init(&msg);
        msgs.push_back(msg);
        return ZEOK;
    }

    err_t ReplaceBack(void *data, size_t len,
                      zmq_free_fn fn = NULL, void *hint = NULL){
        err_t ret = ZEOK;
        zmq_msg_t msg;

        if(fn){
            ret = data ? zmq_msg_init_data(&msg, data, len, fn, hint) :
                ZEPARAM_INVALID;
        }else{
            if(data){
                ret = zmq_msg_init_size(&msg, len);
                if(0 == ret){
                    memcpy(zmq_msg_data(&msg), data, len);
                }
            }else if(len){
                ret = zmq_msg_init_size(&msg, len);
            }else{
                ret = zmq_msg_init(&msg);
            }
        }

        if(0 == ret){
            zmq_msg_close(&msgs.back());
            msgs.pop_back();
            msgs.push_back(msg);
        }
        return ret;
    }

    err_t Pop(zmq_msg_t &msg){
        if(msgs.empty()){
            return ZENOT_EXIST;
        }
        msg = msgs.back();
        msgs.pop_back();
        return ZEOK;
    }

    err_t Erase(int idx){
        msgs.erase(msgs.begin()+idx);
        return ZEOK;
    }
    void Dump(bool is_recv = true){
#if ENABLE_DUMP
        size_t len = 0;

        zdbg("%s<size:%d> :", is_recv ? "recv" : "send", msgs.size());
        for(int i = 0; i < msgs.size(); ++i){
            len = zmq_msg_size(&msgs[i]);
            ztrace_org("msg<len:%d data: ", len);
            if(len < 256){
                ztrace_bin((char*)zmq_msg_data(&msgs[i]), len);
            }
            ztrace_org(">\n");
        }
#endif
    }

    err_t At(int idx, Message &msg){
        err_t ret = ZEOK;
        try{
            msg.Init(msgs.at(idx));
        }catch(...){ret = ZENOT_EXIST;}
        return ret;
    }

    zmq_msg_t *At(int idx){
        return &msgs[idx];
    }

    err_t At(int idx, std::string &msg){
        msg.assign((char*)zmq_msg_data(&msgs[idx]), zmq_msg_size(&msgs[idx]));
        return ZEOK;
    }

    err_t Copy(MsgStack &msg_stack){
        for(size_t i = 0; i < msg_stack.msgs.size(); ++i){
            zmq_msg_t msg;
            zmq_msg_init(&msg);
            zmq_msg_copy(&msg, &msg_stack.msgs[i]);
            msgs.push_back(msg);
        }
        return ZEOK;
    }
    size_t Size(){
        return msgs.size();
    }

    Msgs *GetMsgs(){
        return &msgs;
    }

    void SetClose(bool need){
        need ? (status &= (~1)) : (status |= 1);
    }

    void SetOnline(bool online){
        online ? (status &= (~2)) : (status |= 2);
    }

    bool IsOnline(){
        return !(status & 2);
    }

    int GetFd(){
        return fd;
    }
    void SetFd(int _fd){
        fd = _fd;
    }

    int *GetStatus(){
        return &status;
    }

    void Rotate(){
        /* 旋转数组 */
        zmq_msg_t msg;
        int hops = msgs.size() - 1;
        for(int i = 0; i < hops; ++i, --hops){
            msg = msgs[i];
            msgs[i] = msgs[hops];
            msgs[hops] = msg;
        }
    }
protected:
    Msgs msgs; /** 用户消息 */
    int status; /**
                 * 状态 1- 0:need close 1: not
                 * 在线 2- 0:online 1:offline
                 * 作用域 0xc0 0/1/2/3
                 * 注册/注销 5 0x10
                 */
    int fd; /** 描述符 */
    friend class Socket;
    friend class RoutePath;
};

/**
 * @brief 路由路径
 */
class RoutePath{
public:
    RoutePath()
        :need_close(true)
        ,flag(0){
    }

    ~RoutePath(){
        CloseMsg(true, true, true);
    }

    /** 添加用户数据 */
    err_t PushMsg(Message &src_msg){
        /* 效率考虑，用户必须保证src_msg合法*/
        zmq_msg_t msg;
        zmq_msg_init(&msg);
        zmq_msg_copy(&msg, &src_msg.msg);
        msgs[1].push_back(msg);
        return ZEOK;
    }

    err_t PushMsg(MsgStack &msg_stack){
        for(size_t i = 0; i < msg_stack.msgs.size(); ++i){
            zmq_msg_t msg;
            zmq_msg_init(&msg);
            zmq_msg_copy(&msg, &msg_stack.msgs[i]);
            msgs[1].push_back(msg);
        }
        return ZEOK;
    }

    err_t PushMsg(std::string &str_msg){
        zmq_msg_t msg;
        if(-1 == zmq_msg_init_size(&msg, str_msg.size())){
            zmq_error();
            return ZEFAIL;
        }else{
            memcpy(zmq_msg_data(&msg), str_msg.data(), str_msg.size());
        }
        msgs[1].push_back(msg);
        return ZEOK;
    }
    /** 添加目标路由 */
    err_t PushDest(Message &src_msg){
        zmq_msg_t msg;
        zmq_msg_init(&msg);
        zmq_msg_copy(&msg, &src_msg.msg);
        msgs[2].push_back(msg);
        return ZEOK;
    }

    err_t PushDest(std::string &str_msg){
        zmq_msg_t msg;
        if(-1 == zmq_msg_init_size(&msg, str_msg.size())){
            zmq_error();
            return ZEFAIL;
        }else{
            memcpy(zmq_msg_data(&msg), str_msg.data(), str_msg.size());
        }
        msgs[2].push_back(msg);
        return ZEOK;
    }

    err_t PushDest(MsgStack &route){
        route.status = 1; /* not close */
        size_t size = route.Size();
        size_t i = 0;
        for(i = 0; i < size; ++i){
            msgs[2].push_back(route.msgs[i]);
        }
        return ZEOK;
    }

    err_t DestBack(zmq_msg_t &msg){
        if(msgs[2].empty()){
            return ZENOT_EXIST;
        }

        zmq_msg_init(&msg);
        zmq_msg_copy(&msg, &msgs[2].back());
        return ZEOK;
    }

    err_t SwapDest(){
        zmq_msg_t msg;
        size_t size = msgs[2].size();
        msg = msgs[2].back();
        msgs[2][size - 1] = msgs[2][size - 2];
        msgs[2][size - 2] = msg;
        return ZEOK;
    }
    err_t PopDest(Message &msg){
        if(msgs[2].empty()){
            return ZENOT_EXIST;
        }

        msg.msg = msgs[2].back();
        msgs[2].pop_back();
        return ZEOK;
    }

    /** 添加源路由 */
    err_t PushSrc(Message &src_msg){
        zmq_msg_t msg;
        zmq_msg_init(&msg);
        zmq_msg_copy(&msg, &src_msg.msg);
        msgs[0].push_back(msg);
        return ZEOK;
    }

    err_t PushSrc(std::string &src_msg){
        Message src;
        src.Init(src_msg);
        return PushSrc(src);
    }

    /** 获取用户数据*/
    err_t GetMsg(int i0, int i1, ptr_t *data, int *len){
        if(i0 < 0 || i0 > 2 || !data || !len){
            return ZEPARAM_INVALID;
        }
        if(i1 < 0 || msgs[i0].size() <= i1){
            return ZEPARAM_INVALID;
        }
        *data = zmq_msg_data(&msgs[i0][i1]);
        *len = zmq_msg_size(&msgs[i0][i1]);
        return ZEOK;
    }

    err_t GetUserData(ptr_t *data, int *len, int index){
        *data = zmq_msg_data(&msgs[1][index]);
        *len = zmq_msg_size(&msgs[1][index]);
        return ZEOK;
    }

    err_t GetUserData(std::string &data, int index){
        data.assign((char*)zmq_msg_data(&msgs[1][index]),
                    zmq_msg_size(&msgs[1][index]));
        return ZEOK;
    }

    err_t SetUserData(std::string &data, int index){
        zmq_msg_t msg;
        if(-1 == zmq_msg_init_size(&msg, data.size())){
            zmq_error();
            return ZEFAIL;
        }else{
            memcpy(zmq_msg_data(&msg), data.data(), data.size());
        }

        zmq_msg_move(&msgs[1][index], &msg);
        return ZEOK;
    }

    err_t SetUserData(ptr_t data, int len, int index){
        zmq_msg_t msg;
        if(-1 == zmq_msg_init_size(&msg, len)){
            zmq_errno();
            return ZEFAIL;
        }else{
            memcpy(zmq_msg_data(&msg), data, len);
        }
        zmq_msg_move(&msgs[1][index], &msg);
        return ZEOK;
    }

    err_t MoveUserData(RoutePath *route, int index){
        zmq_msg_move(&msgs[1][index], &route->msgs[1][index]);
    }
    /** 获取路由控制数据 */
    err_t GetRouteCtl(ptr_t *ctl, int *len){
        *ctl = zmq_msg_data(&msgs[1][0]);
        *len = zmq_msg_size(&msgs[1][0]);
        return ZEOK;
    }
    err_t GetRouteCmd(ptr_t *ctl, int *len){
        *ctl = zmq_msg_data(&msgs[1][1]);
        *len = zmq_msg_size(&msgs[1][1]);
    }
    /** 交换消息 */
    err_t SwapMsg(Message &src_msg, Msgs &msgs, int idx){
        zmq_msg_t msg = msgs[idx];
        msgs[idx] = src_msg.msg;
        src_msg.msg = msg;
        return ZEOK;
    }
    err_t SwapMsg(Msgs &msgs, int idx
                  , Msgs &msgs1, int idx1){
        zmq_msg_t msg = msgs[idx];
        msgs[idx] = msgs1[idx1];
        msgs1[idx1] = msg;
        return ZEOK;
    }
    void SwapMsg(Msgs &msgs, Msgs &msgs1){
        msgs.swap(msgs1);
    }
    /** 交换路由路径 */
    err_t SwapRoute(MsgStack &msg_stack){
        msg_stack.msgs.swap(msgs[2]);
    }

    err_t SwapData(Message &src_msg){
        zmq_msg_t msg = msgs[1][1];
        msgs[1][1] = src_msg.msg;
        src_msg.msg = msg;
        return ZEOK;
    }

    /** 旋转消息 */
    err_t RotateRoute(int hops){
        if(0 == hops){
            return ZEOK;
        }
        /** 关闭多余路由地在 */
        while(msgs[2].size() > (hops+1)){
            zmq_msg_close(&msgs[2].back());
            msgs[2].pop_back();
        }
        /* 旋转路由地址 */
        zmq_msg_t msg;
        for(int i = 0; i < hops; ++i, --hops){
            msg = msgs[2][i];
            msgs[2][i] = msgs[2][hops - 1];
            msgs[2][hops - 1] = msg;
        }
        return ZEOK;
    }

    /** 关闭指定消息 */
    err_t CloseMsg(bool src, bool msg, bool dest){
        if(!need_close){
            msgs[0].clear();
            msgs[1].clear();
            msgs[2].clear();
            return ZEOK;
        }
        if(src){
            while(!msgs[0].empty()){
                zmq_msg_close(&msgs[0].back());
                msgs[0].pop_back();
            }
        }
        if(msg){
            while(!msgs[1].empty()){
                zmq_msg_close(&msgs[1].back());
                msgs[1].pop_back();
            }
        }
        if(dest){
            while(!msgs[2].empty()){
                zmq_msg_close(&msgs[2].back());
                msgs[2].pop_back();
            }
        }
        return ZEOK;
    }

    err_t InitRoute(size_t ctl_size){
        zmq_msg_t ctl;
        if(-1 == zmq_msg_init_size(&ctl, ctl_size)){
            return ZEMEM_INSUFFICIENT;
        }
        msgs[1].push_back(ctl);
        return ZEOK;
    }

    err_t Release(ZOPARG){
        flag = 0;
        need_close = true;
        CloseMsg(true, true, true);
    }

    void Dump(bool is_recv = false){
#if ENABLE_DUMP
        size_t len = 0;
        size_t i = 0;
        zdbg("%s<src:%d msg:%d dest:%d>", is_recv
                    ? "recv" : "send", msgs[0].size()
                    , msgs[1].size(), msgs[2].size());
        for(int j = 0; j < 3; ++j){
            for(i = 0; i < msgs[j].size(); ++i){
                len = zmq_msg_size(&msgs[j][i]);
                ztrace_org("msg<len:%d data: ", len);
                if(len < 256){
                    ztrace_bin((char*)zmq_msg_data(&msgs[j][i]), len);
                }
                ztrace_org(">\n");
            }
            if(j != 2){
                ztrace_org("msg<len:0 data: >\n");
            }
        }
#endif
    }

    bool IsLastHop(byte_t cur_hop){
        return (0 == (msgs[2].size() - cur_hop));
    }

    err_t DestId(std::string &dest_id){
        size_t size = msgs[2].size();

        if(size == 1){
            zmq_msg_t msg = msgs[2][0];
            dest_id.assign((char*)zmq_msg_data(&msg),
                           zmq_msg_size(&msg));
            return ZEOK;
        }else if(size > 1){
            zmq_msg_t msg = msgs[2][size - 1];
            dest_id.assign((char*)zmq_msg_data(&msg),
                           zmq_msg_size(&msg));
            return ZEEXIST; /* 中转路由 */
        }

        return ZEPARAM_INVALID;
    }

    err_t CalcRoute(byte_t &hops){
        if(hops < msgs[2].size()){
            zmq_msg_t msg = msgs[0][0];
            msgs[0][0] = msgs[2][hops];
            msgs[2][hops] = msg;
            ++hops;
        }else if(hops == msgs[2].size()){
            /* 标记目标路由器 */
            return ZEOF;
        }else if(hops > msgs[2].size()){
            return ZEPARAM_INVALID;
        }
        return ZEOK;
    }

    err_t CalcRoute(byte_t &hops, bool &is_out){
        /* 支持单向路由器 */
        size_t size = msgs[2].size();
        if(hops < size){
            zmq_msg_t msg = msgs[0][0];
            msgs[0][0] = msgs[2][hops];
            msgs[2][hops] = msg;
            ++hops;
            if(hops == size){
                is_out = false;
            }else{
                char *dest = (char*)zmq_msg_data(&msgs[0][0]);
                size_t len = zmq_msg_size(&msgs[0][0]);
                is_out = ('*' == dest[len - 1]) ? false : true;
            }
        }else if(hops == size){
            /* 标记目标路由器 */
            return ZEOF;
        }else{ /*(hops > size) */
            return ZEPARAM_INVALID;
        }
        return ZEOK;
    }

    zmq_msg_t CopySrc(){
        zmq_msg_t msg;
        zmq_msg_init(&msg);
        zmq_msg_copy(&msg, &(msgs[0][0]));
        return msg;
    }

    err_t GetSrcId(std::string &id){
        id.assign((char*)zmq_msg_data(&msgs[0][0]),
                  zmq_msg_size(&msgs[0][0]));
    }

    int GetFd(){
        return zmq_msg_get(&msgs[1][0], ZMQ_SRCFD);
    }

    err_t SwapSrc(Message &message){
        zmq_msg_t msg = msgs[0][0];
        msgs[0][0] = message.msg;
        message.msg = msg;
        return ZEOK;
    }

    err_t Copy(RoutePath &route){
        zmq_msg_t msg;
        for(int i = 0; i < 3; i++){
            for(size_t j = 0; j < route.msgs[i].size(); ++j){
                zmq_msg_init(&msg);
                zmq_msg_copy(&msg, &(route.msgs[i][j]));
                msgs[i].push_back(msg);
            }
        }
        return ZEOK;
    }

    err_t CopyRoute(RoutePath &route){
        size_t size = route.msgs[2].size();
        size_t i = 0;
        zmq_msg_t msg;
        if(msgs[2].size() || (0 == size)){
            return ZEPARAM_INVALID;
        }
        for(i = 0; i < size; ++i){
            zmq_msg_init(&msg);
            zmq_msg_copy(&msg, &(route.msgs[2][i]));
            msgs[2].push_back(msg);
        }
        return ZEOK;
    }

    err_t Copy(MsgStack &ms, int index, int begin_pos = 0, int end_pos = 0){
        int i = begin_pos;

        if(end_pos == 0){
            end_pos = msgs[index].size();
        }

        for(; i < end_pos; ++i){
            zmq_msg_t msg;
            zmq_msg_init(&msg);
            zmq_msg_copy(&msg, &(msgs[index][i]));
            ms.Push(msg);
        }
        return ZEOK;
    }

    err_t GetSrcId(char **id, int *len){
        *id = NULL;
        *len = NULL;
        if(msgs[2].size()){
            *id = (char*)zmq_msg_data(&msgs[2][0]);
            *len = zmq_msg_size(&msgs[2][0]);
            return ZEOK;
        }
        return ZENOT_EXIST;
    }
    uint32_t GetFlag(){
        return flag;
    }
    void SetFlag(uint32_t _flag){
        flag = _flag;
    }
protected:
    bool need_close; /** 标记是否需要关闭消息 */
    uint32_t flag; /** 用户自定义标记
                    *  0x01 0- not copy 1 - copy
                    */
    /* 效率优化 */
    Msgs msgs[3]; /** 0- 源路径
                   *  1- 用户数据
                   *  2- 目标路径
                   */
    friend class Socket;
};

struct ConnInfo{
    ConnInfo(std::string ep)
        :stat(0){
        endp.assign(ep);
        time(&last_recv);
    }
    std::string endp; /** 连接端口 */
    std::string id; /** 对端ID */
    int stat; /** 接口状态（用户自定义） */
    time_t last_recv; /** 最近接收数据时间戳 */
};

typedef std::vector<ConnInfo*> ConnInfos;

/**
 * @brief ZeroMq 套接字封装
 */
class Socket{
public:
    /**
     * @brief 初始化zmq环境
     */
    static err_t InitCtx();

    /**
     * @brief 释放zmq环境
     */
    static err_t FiniCtx();

    /**
     * @brief 设置环境
     * @note 动态库共用进程环境
     */
    static err_t SetCtx(ctx_t ctx);

    static ctx_t GetCtx(){
        return s_ctx;
    }
public:
    /** API 封装 */
    Socket(int type);
    ~Socket();

    err_t Close(); /** 控制生命周期 */
    err_t Bind(const char *endpoint);
    err_t Unbind(const char *endpoint);
    err_t Connect(const char *endpoint);
    err_t Connect(const char *endpoint, const char *id, int len);
    err_t Disconnect(const char *endpoint);
    err_t GetOpt(int option_name, void *option_value, size_t *option_len);
    err_t SetOpt(int option_name, const void *option_value, size_t option_len);
    err_t SetHwm(int is_send, int hwm);
    err_t SetLinger(int linger);
    err_t SetId(const char *id, int len);
    err_t SetId(std::string &id);
    err_t SubAll();
    err_t Subscribe(const char *filter, int len);
    err_t Unsubscribe(const char *filter, int len);

    int IsMore(){
        int more;
        size_t len;
        len = sizeof(more);
        if(-1 == zmq_getsockopt(sock, ZMQ_RCVMORE, &more, &len)){
            more = -1;
        }
        return more;
    }

    err_t Send(Message &msg, int flags = 0){
        msg.Dump(false);
        if(-1 == zmq_msg_send(&msg.msg, sock, flags)){
            zmq_error();
            return ZEFAIL;
        }else{
            msg.need_close = false;
        }
        return ZEOK;
    }

    err_t Recv(Message &msg, int flags = 0){
        msg.Init();
        if(-1 == zmq_msg_recv(&msg.msg, sock, flags)){
            zmq_error();
            return ZEFAIL;
        }
        msg.Dump();
        return ZEOK;
    }

    err_t Send(MsgStack &msgs, int flags = 0, bool dump = true){
        int i = 0;
        int size = msgs.msgs.size() - 1;
        if(size < 0){
            return ZEPARAM_INVALID;
        }
        if(dump){
            msgs.Dump(false);
        }
        for(i = 0; i < size; ++i){
            if(-1 == zmq_msg_send(&msgs.msgs[i], sock, flags | ZMQ_SNDMORE)){
                zmq_error();
                return ZEFAIL;
            }
        }

        if(-1 == zmq_msg_send(&msgs.msgs[i], sock, flags)){
            zmq_error();
            return ZEFAIL;
        }
        msgs.SetClose(false);
        return ZEOK;
    }

    err_t Recv(MsgStack &msgs, int flags = 0){
        int more = false;
        zmq_msg_t msg;
        do{
            zmq_msg_init(&msg);
            if(-1 == zmq_msg_recv(&msg, sock, flags)){
                zmq_error();
                return ZEFAIL;
            }
            msgs.Push(msg);
            more = zmq_msg_more(&msg);
        }while(more);
        msgs.Dump();
        return ZEOK;
    }
    /* 内部路由 */
    err_t Send(RoutePath &route_path, int flags = 0, bool dump = true){
        size_t i = 0;
        size_t dest_size = 0;
        int stat = 0; /* 0- src 1- msg 2- dest */
        if(dump){
            route_path.Dump(false);
        }

        if(route_path.msgs[1].size() < 2){
            return ZEPARAM_INVALID;
        }

        /* 发送目标地址 */
        if(route_path.msgs[0].size()){
            if(-1 == zmq_msg_send(&route_path.msgs[0][0],
                                  sock, flags | ZMQ_SNDMORE)){
                zmq_error();
                return ZEFAIL;
            }
        }

        /* 发送分割符号 */
        zmq_msg_t msg0;
        zmq_msg_init(&msg0);
        if(-1 == zmq_msg_send(&msg0, sock, flags | ZMQ_SNDMORE)){
            zmq_error();
            zmq_msg_close(&msg0);
            return ZEFAIL;
        }

        /* 发送消息正文 */
        for(i = 0; i < route_path.msgs[1].size(); ++i){
            if(-1 == zmq_msg_send(&route_path.msgs[1][i],
                                  sock, flags | ZMQ_SNDMORE)){
                zmq_error();
                return ZEFAIL;
            }
        }

        /* 发送路径 */
        dest_size = route_path.msgs[2].size();
        if(dest_size){
            /* 发送分割符号 */
            zmq_msg_t msg1;
            zmq_msg_init(&msg1);
            if(-1 == zmq_msg_send(&msg1, sock, flags | ZMQ_SNDMORE)){
                zmq_error();
                zmq_msg_close(&msg1);
                return ZEFAIL;
            }
            /* 发送路径内容 */
            for(i = 0; i < dest_size - 1; ++i){
                if(-1 == zmq_msg_send(&route_path.msgs[2][i],
                                      sock, flags | ZMQ_SNDMORE)){
                    zmq_error();
                    return ZEFAIL;
                }
            }
            /* 发送最后一条路径内容 */
            if(-1 == zmq_msg_send(&route_path.msgs[2][dest_size - 1],
                                  sock, flags)){
                zmq_error();
                return ZEFAIL;
            }
        }else{
            /* 发送分割符号 */
            zmq_msg_t msg1;
            zmq_msg_init(&msg1);
            if(-1 == zmq_msg_send(&msg1, sock, flags)){
                zmq_error();
                zmq_msg_close(&msg1);
                return ZEFAIL;
            }
        }
        route_path.need_close = false;
        return ZEOK;
    }

    err_t Recv(RoutePath &route_path, int flags = 0){
        int more = false;
        int stat = 0; /* 0- src 1- msg 2- dest */
        zmq_msg_t msg;
        do{
            zmq_msg_init(&msg);
            if(-1 == zmq_msg_recv(&msg, sock, flags)){
                zmq_error();
                return ZEFAIL;
            }

            more = zmq_msg_more(&msg);

            if(stat < 3 && zmq_msg_size(&msg)){
                route_path.msgs[stat].push_back(msg);
            }else{
                ++stat;
                zmq_msg_close(&msg);
                if(stat > 3){
                    zerrno(ZENOT_SUPPORT);
                }
            }
        }while(more);
        route_path.Dump(true);
        if(stat != 2){
            return ZENOT_SUPPORT;
        }
        return ZEOK;
    }

    err_t RecvByTimeout(RoutePath &route_path, int timeout_ms){
        err_t ret = ZEOK;
        zmq_pollitem_t items[]={sock, 0, ZMQ_POLLIN, 0};
        int ready_socks = zmq_poll(items, 1, timeout_ms);
        if(ready_socks > 0){
            if(items[0].revents & ZMQ_POLLIN){
                ret = Recv(route_path, ZMQ_DONTWAIT);
            }
        }else if(-1 == ready_socks){
            /* wait error */
            zmq_error();
            ret = ZEFAIL;
        }else{
            /* do idle worker */
            ret = ZETIMEOUT;
        }
        return ret;
    }

public:
    /** 高阶封装 */
    err_t LazyPiratReq(Message &req, Message &rep,
                       int timeout_ms, int trys = 1);
    err_t Reconnect();
    err_t Monitor(const char *endpoint, int events);
    err_t MonitorEvent(uint16_t *event, uint32_t *fd,
                       char *addr = NULL, int len = 0){
        /* 读取监信息 */
        err_t ret = ZEOK;
        zmq_msg_t msg;

        zmq_msg_init(&msg);
        if(-1 == zmq_msg_recv(&msg, sock, 0)){
            zmq_error();
            return ZEFAIL;
        }
        if(!zmq_msg_more(&msg)){
            return ZENOT_SUPPORT;
        }

        char *data = (char*)zmq_msg_data(&msg);
        *event = *(uint16_t*)data;
        *fd = *(uint32_t*)(data + 2);

        zmq_msg_close(&msg);

        zmq_msg_init(&msg);
        if(-1 == zmq_msg_recv(&msg, sock, 0)){
            zmq_errno();
            return ZEFAIL;
        }

        if(addr){
            if(len < zmq_msg_size(&msg) + 1){
                return ZEPARAM_INVALID;
            }
            memcpy(addr, zmq_msg_data(&msg), zmq_msg_size(&msg));
            *(addr + zmq_msg_size(&msg)) = 0;
        }

        zmq_msg_close(&msg);
        return ZEOK;
    }
    void DumpEvent(uint16_t event, uint32_t fd,
                   const char *addr = NULL);
    ctx_t GetSock(){
        return sock;
    }

    ConnInfos conns; /** 连接信息 */
    err_t SetConnStat(int stat, void *id, int len);
    err_t SetConnStat(int stat, const char *endp);
protected:
    static ctx_t s_ctx;
    static atmc_t total;
    ctx_t sock;
    /* for Reconnect */
    int sock_type;
    err_t Conn(const char *endpoint, const char *id, int len); /** 新连接 */
    err_t Disconn(const char *endpoint, const char *id, int len); /** 断开连接 */
};

ZNS_ZMQE
#endif
