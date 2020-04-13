#include <zsi/base/trace.h>
#include <zsi/app/str_split.h>

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
    nng_msg_header_append(rep, nng_msg_header(req), nng_msg_header_len(req));
    nng_aio_set_msg(aio, rep);
    state = SEND;
    nng_send_aio(reply->sock, aio);
    return ZEOK;
}

Reply::Reply()
    :tasks(NULL)
    ,onRequest(NULL)
    ,onError(NULL)
    ,thread_pool(NULL)
    ,hint(NULL){}

Reply::~Reply(){
    delete [] tasks;
}


zerr_t Reply::Listen(const std::string &url, int parallel, OnTask onReq, OnTask onErr, void *_hint, ztpl_t *tpool){
    zdbg("Reply::Listen(url:%s, parallel:%d, onReq:%p, onErr:%p, hint:%p, tpool:%p)",
         url.c_str(), parallel, onReq, onErr, _hint, tpool);
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
    hint = _hint;

    if(0 != nng_rep0_open_raw(&sock)){
        zerrno(ZEFAIL);
        return ZEFAIL;
    }

    /* listen on mult urls, delemiter with ';' */
    int i;
    int argc = 10;
    char *argv[10];
    char *urls = (char*)malloc(url.length() + 1);
    if(!urls){
        return ZEMEM_INSUFFICIENT;
    }
    strcpy(urls, url.c_str());

    zstr_split(&argc, argv, urls, url.length(), ';', 0);
    for(i = 0; i < argc; ++i){
        zmsg("listen %s", argv[i]);
        if(0 != nng_listen(sock, argv[i], NULL, 0)){
            zerrno(ZEFAIL);
            free(urls);
            return ZEFAIL;
        }
    }
    free(urls);

    for(i = 0; i < task_num; ++i){
        tasks[i].reply = this;
        ReplyOnEvent(&tasks[i]);
    }
    return ZEOK;
}

NSE_NNG