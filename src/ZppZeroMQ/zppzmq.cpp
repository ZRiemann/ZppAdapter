#include <zit/base/type.h>
#include <zit/base/error.h>
#include <zit/base/trace.h>
#include <zit/base/time.h>
#include <zit/thread/thread.h>
#include <zpp/framework/mapfind.hpp>
#include <zmq.h>
#include <zpp/zppzmq.h>
#include <stdlib.h>
#include <string.h>

namespace z{
  namespace zmq{
    static const char *traceTitle = "[zmq]";
#define zdbg_zmq(fmt, ...) zdbgx(Socket::traceFlag, "[ln:%04d fn:%s]%s\t" fmt, __LINE__, __FUNCTION__,traceTitle, ##__VA_ARGS__)
#define zmsg_zmq(fmt, ...) zmsgx(Socket::traceFlag, "[ln:%04d fn:%s]%s\t" fmt, __LINE__, __FUNCTION__,traceTitle, ##__VA_ARGS__)
#define zwar_zmq(fmt, ...) zwarx(Socket::traceFlag, "[ln:%04d fn:%s]%s\t" fmt, __LINE__, __FUNCTION__,traceTitle, ##__VA_ARGS__)
#define zerr_zmq(fmt, ...) zerrx(Socket::traceFlag, "[ln:%04d fn:%s]%s\t" fmt, __LINE__, __FUNCTION__,traceTitle, ##__VA_ARGS__)
#define zinf_zmq(fmt, ...) zinfx(Socket::traceFlag, "[ln:%04d fn:%s]%s\t" fmt, __LINE__, __FUNCTION__,traceTitle, ##__VA_ARGS__)
    
#define ZMQERR() ZERR("%s", zmq_strerror(errno))
#define ZTRACE_ZMQ 1
    
    int Socket::traceFlag = 0xfe;
    void *Socket::s_ctx = NULL;
    int Socket::Init(bool setLinger0){
      int major, minor,  patch;
      zmq_version(&major, &minor, &patch);
      if(major < 4 || (major ==4 && minor < 2)){
	ZERR("ZeroMQ version mast v4.2.1 or later.");
	return ZNOT_SUPPORT;
      }
      if(s_ctx){
	return ZOK; // init already
      }
      s_ctx = zmq_ctx_new();
      if(setLinger0){
	zmsg_zmq("Set zmq context linger timeout 0");
	zmq_ctx_set(s_ctx, ZMQ_BLOCKY, false);
      }
      if(NULL == s_ctx){
	ZMQERR();
	return ZFAIL;
      }
#if ZTRACE_ZMQ
      zdbg_zmq("ctx<%p> = Init(setLinger0:%d) version%d.%d.%d", s_ctx, setLinger0, major, minor, patch);
#endif
      return ZOK;
    }
    
    int Socket::Fini(){
      if(!s_ctx){
	return ZOK;
      }
      int ret = zmq_ctx_term(s_ctx);
      if(-1 == ret){
	if(EINTR == errno){
	  ret = zmq_ctx_term(s_ctx);
	}
	if(-1 == ret){
	  ZMQERR();
	}
      }
#if ZTRACE_ZMQ
      zdbg_zmq("ctxTerm<ctx:%p, ret:%d>", s_ctx, ret);
#endif
      s_ctx = NULL;
      return ZOK;
    }
    
    Socket::Socket(int _sockType){
      sockType = _sockType;
      sockHandle = zmq_socket(s_ctx, sockType);
      if(NULL == sockHandle){
	ZMQERR();
	return;
      }
      
      if(sockType != ZMQ_REQ && sockType != ZMQ_REP && sockHandle){
	SetHwm(0, 100);
	SetHwm(1, 100);
      }
      SetLinger(0);
      
#if ZTRACE_ZMQ
      zdbg_zmq("zmqsock<type:%d handle:%p ptr:%p>", sockType, sockHandle, this);
#else
      if(NULL == sockHandle){
	ZERR("zmqsock<type:%d handle:%p ptr:%p>", sockType, sockHandle, this);
      }
#endif
    }
    
    Socket::Socket(int _sockType, const char *id){
      sockType = _sockType;
      sockHandle = zmq_socket(s_ctx, sockType);
      if(NULL == sockHandle){
	ZMQERR();
	return;
      }
      
      if(sockType != ZMQ_REQ && sockType != ZMQ_REP && sockHandle){
	SetHwm(0, 100);
	SetHwm(1, 100);
      }
      SetLinger(0);
      SetId(id, strlen(id));
#if ZTRACE_ZMQ
      zdbg_zmq("zmqsock<type:%d handle:%p ptr:%p>", sockType, sockHandle, this);
#else
      if(NULL == sockHandle){
	ZERR("zmqsock<type:%d handle:%p ptr:%p>", sockType, sockHandle, this);
      }
#endif
    }
    Socket::~Socket(){
      zmq_close(sockHandle);
#if ZTRACE_ZMQ
      zdbg_zmq("close<handle:%p ptr:%p>", sockHandle, this);
#endif
    }
    
    int Socket::Bind(const char *endpoint){
      int ret = zmq_bind(sockHandle, endpoint);
      if(-1 == ret){
	ZMQERR();
      }
      ZMSG("zmq_bind(%p, %s)=%d", sockHandle, endpoint, ret);
      return ret;
    }
    
    int Socket::Connect(const char *endpoint){
      int ret = zmq_connect(sockHandle, endpoint);
      if(-1 == ret){
	ZMQERR();
      }else{
	endpoints.push_back(endpoint);
      }
      ZMSG("zmq_connect(%p, %s)=%d", sockHandle, endpoint,ret);
      return ret;
    }
    
    int Socket::MsgSend(Msg *msg, int flags){
      int ret;
#if ZTRACE_ZMQ
      msg->Dump(1); // 放前面，因为send成功后，内容被自动消除无法打印
#endif
      if(-1 == (ret = zmq_msg_send(&msg->msg, sockHandle, flags))){
	ZMQERR();
      }
      return ret;
    }
    
    int Socket::MsgRecv(Msg *msg, int flags){
      int ret;
      if(-1 == (ret = zmq_msg_recv(&msg->msg, sockHandle, flags))){
	ZMQERR();
      }
#if ZTRACE_ZMQ
      else{
	msg->Dump(0);
      }
#endif
      return ret;
    }

    int Socket::LazyPirateReq(Msg *_msgReq, Msg *_msgRep, int timeout_ms, int trys){
      int ret = ZTIMEOUT;
      int rc;

      do{
	Msg msgReq(_msgReq);
	//Msg msgRep;

	if(-1 == MsgSend(&msgReq, 0)){
	  ret = ZFAIL;
	  break;
	}

	zmq_pollitem_t items[]={{sockHandle, 0, ZMQ_POLLIN, 0}};
	rc = zmq_poll(items, 1, timeout_ms);
	if(rc > 0){
	  // reply
	  //ret = MsgRecv(&msgRep, 0);
	  ret = MsgRecv(_msgRep, 0);
	  if(-1 != ret){
	    //_msgRep->Move(&msgRep); // move to client;
	    ret = ZOK;
	  }

	  break;
	}else if(0 == rc){
	  // timeout
	  ZMSG("Wiat reply timeout tyrs:%d", trys);
	}else{
	  // error
	  ZMQERR();
	  //break; | try again
	  ZERR("Wait fail trys:%d", trys);
	}
	Reconnect(); // for try again
      }while(--trys);
      ZERRCX(ret);
      return ret;
    }

    int Socket::Reconnect(){
      int ret = ZOK;
      int size = endpoints.size();
  
      if(size){
	Endpoints endp = endpoints;
	ZMSG("Reconnect<%p>", sockHandle);
	zmq_close(sockHandle);
	sockHandle = zmq_socket(s_ctx, sockType);
	if(NULL == sockHandle){
	  ret = errno;
	  ZERRC(ret);
	  return ZFUN_FAIL;
	}else{
	  SetLinger(0);
	}
    
	endpoints.clear(); // avoid mulit push
	do{
	  Connect(endp[size-1].c_str());
	}while(--size);
	zsleepsec(1); // wait connection establish
      }
      ZERRC(ret);
      return ret;
    }

    int Socket::IsMore(){
      int more;
      size_t len;
      int ret;
      len = sizeof(more);
      ret = GetOpt(ZMQ_RCVMORE, &more, &len);
      if(ret){
	return ret; 
      }
      return more;
    }

    int Socket::SetHwm(int is_send, int hwm){
      int option_name = ZMQ_RCVHWM;
#if 0
      const char* name = "RecvHwm";
      if(is_send){
	option_name = ZMQ_SNDHWM;
	name = "SendHwm";
      }
      zmsg_zmq("Set %s option: %d", name, hwm);
#endif
      return SetOpt(option_name, &hwm, sizeof(hwm));
    }

    int Socket::SetLinger(int linger){
      //zmsg_zmq("Set Linger option: %d", linger);
      return SetOpt(ZMQ_LINGER, &linger, sizeof(linger));
    }

    int Socket::SetId(const char *id, int len){
      char buf[128];
      int ln = len;
      zdump_mix(buf, 128, (const unsigned char*)id, &ln);
      zmsg_zmq("Set socket ID: %s", buf);
      return SetOpt(ZMQ_IDENTITY, id, len);
    }

    int Socket::SubAll(){
      return Subscribe("", 0);
    }

    int Socket::Subscribe(const char *filter, int len){
      char buf[128];
      int ln = len;
      zdump_mix(buf, 128, (const unsigned char*)filter, &ln);
      zmsg_zmq("Subscribe :%s", buf);
      return SetOpt(ZMQ_SUBSCRIBE, filter, len);
    }

    int Socket::Unsubscribe(const char *filter, int len){
      char buf[128];
      int ln = len;
      zdump_mix(buf, 128, (const unsigned char*)filter, &ln);
      zmsg_zmq("Unsubscribe :%s", buf);
      return SetOpt(ZMQ_UNSUBSCRIBE, filter, len);
    }

    int Socket::GetOpt(int option_name, void *option_value, size_t *option_len){
      int ret = zmq_getsockopt(sockHandle, option_name, option_value, option_len);
      if(-1 == ret){
	ZMQERR();
      }
      return ret;
    }
    int Socket::SetOpt(int option_name, const void *option_value, size_t option_len){
      int ret = zmq_setsockopt(sockHandle, option_name, option_value, option_len);
      if(-1 == ret){
	ZMQERR();
      }
      return ret;
    }

    Msg::Msg(){
      zmq_msg_init(&msg); //always return 0
    }

    Msg::Msg(Msg *copy){
      zmq_msg_init(&msg);
      Copy(copy);
    }

    Msg::Msg(int *initOk, size_t size){
      *initOk = zmq_msg_init_size(&msg, size);
      if(-1 == *initOk){
	ZMQERR();
      }
    }

    Msg::Msg(int *initOk, void* data, size_t size, zmq_free_fn *ffn, void*hint){
      *initOk = zmq_msg_init_data(&msg, data, size, ffn, hint);
      if(-1 == *initOk){
	ZMQERR();
      }
    }
    Msg::~Msg(){
#if ZTRACE_ZMQ
      //zdbg_zmq("%d = zmq_msg_close(%p)", zmq_msg_close(&msg), &msg);
      zmq_msg_close(&msg);
#else
      zmq_msg_close(&msg);
#endif
    }

#if 1 // dump size
    void Msg::Dump(int isSend){
      zmsg_zmq("%s msg<size:%d, ptr:%p>", isSend ? "Send" : "Recv", Size(), Data());
    }
#else // dump detail    
    void Msg::Dump(int isSend){
      if(!(Socket::traceFlag & ZTRACE_FLAG_MSG)){
	return; // control debug
      }
      char buf[4096] = {0};
      size_t offset = 0;
      size_t size= Size();
      int len = (int)size;
      unsigned char *data = (unsigned char*)Data();
      if(0 == size){
	zmsg_zmq("%s zmq_msg<size:%d>: empty message", isSend ? "Send" : "Recv", size);
	return;
      }
      while(len){
	//zdump_mix(buf, 3900, data+offset, &len);
	zdump_bin(buf, 3900, data+offset, &len);
	zmsg_zmq("%s zmq_msg<size:%d>:\n%s", isSend ? "Send" : "Recv", size, buf);
	offset += len;
	len = size - offset;
      }
    }
#endif

    Connecter::Connecter(){
      zmutex_init(&mtx);
    }

    Connecter::~Connecter(){
      zmutex_uninit(&mtx);
    }
    
    z::zmq::Socket * Connecter::Connect(const std::string &endpoint, int sockType){
      ep2sock *ep2sk = NULL;
      z::zmq::Socket *sock = NULL;
      uint32_t tid = zthread_self();

      ZLOCK(&mtx);
      if(z::MapFind(tid2ep, tid, ep2sk)){
	ZUNLOCK(&mtx);
	// find sock
	if(!z::MapFind(*ep2sk, endpoint, sock)){
	  // add new sock
	  sock = new(std::nothrow)Socket(sockType);
	  if(!sock){
	    ZERRC(ZMEM_INSUFFICIENT);
	  }else{
	    if(0 == sock->Connect(endpoint.c_str())){
	      (*ep2sk)[endpoint] = sock; // map new sock
	      if(sockType == ZMQ_PUSH){
		zsleepms(500);//wait connect establish
	      }
	      zmsg_zmq("thr[%u] connect<type:%d endp:%s>", tid, sockType, endpoint.c_str());
	    }else{
	      //ZDEL(sock);
	      delete sock;
	      sock = NULL;
	    }
	  }	  
	}// else find sock
      }else{
	// first time connect
	ep2sk = new(std::nothrow) ep2sock;
	if(!ep2sk){
	  ZUNLOCK(&mtx);
	  ZERRC(ZMEM_INSUFFICIENT);
	}else{
	  tid2ep[tid] = ep2sk;
	  ZUNLOCK(&mtx);
	  // add new sock
	  sock = new(std::nothrow)Socket(sockType);
	  if(!sock){
	    ZERRC(ZMEM_INSUFFICIENT);
	  }else{
	    if(0 == sock->Connect(endpoint.c_str())){
	      (*ep2sk)[endpoint] = sock; // map new sock
	      if(sockType == ZMQ_PUSH){
		zsleepms(500);//wait connect establish
	      }
	      zmsg_zmq("thr[%u] connect<type:%d endp:%s>", tid, sockType, endpoint.c_str());
	    }else{
	      //ZDEL(sock);
	      delete sock;
	      sock = NULL;
	    }
	  }
	}
      }
      return sock;
    }
    
    void Connecter::Close(){
      ep2sock *ep2sk = NULL;
      uint32_t tid = zthread_self();
      ZLOCK(&mtx);
      if(z::MapFind(tid2ep, tid, ep2sk)){
	tid2ep.erase(tid); // erase tid
	ZUNLOCK(&mtx);
	z::MapClear(*ep2sk); // clear socks
	delete ep2sk;
      }else{
	ZUNLOCK(&mtx);
      }
    }
    
    //zmutex_t mtx;
    //typedef std::map<std::string, z::zmq::Socket*> ep2sock; 
    //std::map< uint32_t, ep2sock* > tid2ep;
    
  }// namespace zmq
}// namespace z
