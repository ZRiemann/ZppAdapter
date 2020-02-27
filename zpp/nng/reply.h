#ifndef _ZPP_NNG_REPLY_H_
#define _ZPP_NNG_REPLY_H_
/**
 * @license MIT
 * @file zpp/nng/reply.hpp
 * @note nng asynchronous reply server
 */
#include <string>

#include <zsi/base/type.h>
#include <zsi/base/error.h>
#include <zsi/thread/thread_pool.h>

#include <nng/nng.h>
#include <nng/protocol/reqrep0/rep.h>
#include <nng/supplemental/util/platform.h>

#ifndef NSB_NNG
#define NSB_NNG namespace z{ namespace nng{
#define NSE_NNG }}
#endif

NSB_NNG

class Reply{
public:
    class Task{
    public:
        enum { INIT, RECV, SEND } state;
        Reply * reply;
        nng_aio * aio;
        nng_msg * req; // Request message
        nng_msg * rep; // reply message
    public:
        Task();
        ~Task();
        zerr_t Response(nng_msg *rep);
    };
    typedef void (*OnTask)(z::nng::Reply::Task *);

public:
    nng_socket sock;
    Task *tasks;
    int task_num;
    OnTask onRequest;
    OnTask onError;
    ztpl_t *thread_pool;

    friend class Task;
public:
    Reply();
    ~Reply();
    zerr_t Listen(const std::string &url, int parallel, OnTask onReq, OnTask onErr, ztpl_t *thread_pool = NULL);
};
NSE_NNG

#endif
