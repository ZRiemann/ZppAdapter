#include <zsi/base/trace.h>

NSB_NNG

static zerr_t do_request(zop_arg){
    Reply::Task *task = (Reply::Task*)in;
    task->reply->onRequest(task);
    return ZEOK;
}

static void ReplyOnEvent(void *arg){
    Reply::Task *task = (Reply::Task*)arg;
    Reply *reply = task->reply;
    switch(task->state){
    case Reply::Task::INIT:
        task->state = Reply::Task::RECV;
        nng_recv_aio(reply->sock, task->aio);
        break;
    case Reply::Task::RECV:
        if(0 != nng_aio_result(task->aio)){
            zerrno(ZEFAIL);
            reply->onError(task);
        }
        task->req = nng_aio_get_msg(task->aio);
        if(reply->thread_pool){
            //reply->onRequest(task);
            if(ZEOK != ztpool_post_task(reply->thread_pool, do_request, arg, NULL)){
                reply->onRequest(task);
            }
        }else{
            reply->onRequest(task);
        }
        break;
    case Reply::Task::SEND:
        if(0 != nng_aio_result(task->aio)){
            zerrno(ZEFAIL);
            reply->onError(task);
            nng_msg_free(task->rep);
        }
        nng_msg_free(task->req);
        task->state = Reply::Task::RECV;
        nng_recv_aio(reply->sock, task->aio);
        break;
    default:
        zerrno(ZENOT_SUPPORT);
    }
}

z::nng::Reply::Task::Task()
    :reply(NULL){
    if(0 != nng_aio_alloc(&aio, ReplyOnEvent, this)){
        aio = NULL;
    }
    state = INIT;
}

z::nng::Reply::Task::~Task(){
    nng_aio_free(aio);
}
zerr_t Reply::Task::Response(nng_msg *rep_msg){
    rep = rep_msg;
    nng_msg_header_append(rep, nng_msg_herder(req), nng_msg_header_len(req));
    nng_aio_set_msg(aio, rep);
    state = SEND;
    nng_send_aio(reply->sock, aio);
    return ZEOK;
}

Reply::Reply()
    :tasks(NULL)
    ,onRequest(NULL)
    ,onError(NULL)
    ,thread_pool(NULL){}

Reply::~Reply(){
    delete [] tasks;
}


zerr_t Reply::Listen(const std::string &url, int parallel, OnTask onReq, OnTask onErr, ztpl_t *tpool){
    zdbg("Reply::Listen(url:%s, parallel:%d, onReq:%p, tpool:%p)");
    if(!onReq){
        zerrno(ZEPARAM_INVALID);
        return ZEPARAM_INVALID;
    }
    if(parallel <= 0 || parallel > 64){
        parallel = 8;
    }
    task_num = parallel;
    tasks = new(std::nothrow) Task[task_num]();
    if(!tasks){
        zerrno(ZEMEM_INSUFFICIENT);
        return ZEMEM_INSUFFICIENT;
    }
    thread_pool = tpool;
    onRequest = onReq;
    onError = onErr;

    if(0 != nng_rep0_open_raw(&sock)){
        zerrno(ZEFAIL);
        return ZEFAIL;
    }
    if(0 != nng_listen(sock, url.c_str(), NULL, 0)){
        zerrno(ZEFAIL);
        return ZEFAIL;
    }

    for(int i = 0; i < task_num; ++i){
        tasks[i].reply = this;
        ReplyOnEvent(&tasks[i]);
    }
    return ZEOK;
}

NSE_NNG