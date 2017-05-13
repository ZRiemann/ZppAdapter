#include <zit/base/type.h>
#include <zit/base/error.h>
#include <zit/base/trace.h>
#include <zit/base/time.h>

#include <zmq.h>
#include <zpp/zppzmq.h>
#include <stdlib.h>
#include <string.h>

#define zdbg_zmq(fmt, ...) zdbgx(0, ZmqSocket::traceFlag, "[ln:%04d fn:%s]\t" fmt, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define zmsg_zmq(fmt, ...) zmsgx(0, ZmqSocket::traceFlag, "[ln:%04d fn:%s]\t" fmt, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define zwar_zmq(fmt, ...) zwarx(0, ZmqSocket::traceFlag, "[ln:%04d fn:%s]\t" fmt, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define zerr_zmq(fmt, ...) zerrx(0, ZmqSocket::traceFlag, "[ln:%04d fn:%s]\t" fmt, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define zinf_zmq(fmt, ...) zinfx(0, ZmqSocket::traceFlag, "[ln:%04d fn:%s]\t" fmt, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#define ZMQERR() ZERR("%s", zmq_strerror(errno))
#define ZTRACE_ZMQ 1
using namespace z;

int ZmqSocket::traceFlag = 0xfe;
void *ZmqSocket::s_ctx = NULL;
int ZmqSocket::ZmqInit(bool setLinger0){
  int major, minor,  patch;
  zmq_version(&major, &minor, &patch);
  if(major < 4 || (major >=4 && minor < 2)){
    ZERR("ZeroMQ version mast v4.2.1 or later.");
    return ZNOT_SUPPORT;
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
  zdbg_zmq("ctx<%p> = ZmqInit(setLinger0:%d) version%d.%d.%d", s_ctx, setLinger0, major, minor, patch);
#endif
  return ZOK;
}

int ZmqSocket::ZmqFini(){
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
  return ZOK;
}

ZmqSocket::ZmqSocket(int _sockType){
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

ZmqSocket::ZmqSocket(int _sockType, const char *id){
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
ZmqSocket::~ZmqSocket(){
  zmq_close(sockHandle);
#if ZTRACE_ZMQ
  zdbg_zmq("close<handle:%p ptr:%p>", sockHandle, this);
#endif
}

int ZmqSocket::Bind(const char *endpoint){
  int ret = zmq_bind(sockHandle, endpoint);
  if(-1 == ret){
    ZMQERR();
  }
  ZMSG("zmq_bind(%p, %s)=%d", sockHandle, endpoint, ret);
  return ret;
}

int ZmqSocket::Connect(const char *endpoint){
  int ret = zmq_connect(sockHandle, endpoint);
  if(-1 == ret){
    ZMQERR();
  }else{
    endpoints.push_back(endpoint);
  }
  ZMSG("zmq_connect(%p, %s)=%d", sockHandle, endpoint,ret);
  return ret;
}

int ZmqSocket::MsgSend(ZmqMsg *msg, int flags){
  int ret;
#if ZTRACE_ZMQ
  msg->Dump(1); // 放前面，因为send成功后，内容被自动消除无法打印
#endif
  if(-1 == (ret = zmq_msg_send(&msg->msg, sockHandle, flags))){
    ZMQERR();
  }
  return ret;
}

int ZmqSocket::MsgRecv(ZmqMsg *msg, int flags){
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

int ZmqSocket::LazyPirateReq(ZmqMsg *_msgReq, ZmqMsg *_msgRep, int timeout_sec, int trys){
  int ret = ZTIMEOUT;
  int rc;

  do{
    ZmqMsg msgReq(_msgReq);
    ZmqMsg msgRep;

    if(-1 == MsgSend(&msgReq, 0)){
      ret = ZFAIL;
      break;
    }

    zmq_pollitem_t items[]={{sockHandle, 0, ZMQ_POLLIN, 0}};
    rc = zmq_poll(items, 1, timeout_sec*1000);
    if(rc > 0){
      // reply
      ret = MsgRecv(&msgRep, 0);
      if(-1 != ret){
	_msgRep->Move(&msgRep); // move to client;
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

int ZmqSocket::Reconnect(){
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

int ZmqSocket::IsMore(){
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

int ZmqSocket::SetHwm(int is_send, int hwm){
  int option_name = ZMQ_RCVHWM;
  const char* name = "RecvHwm";
  if(is_send){
    option_name = ZMQ_SNDHWM;
    name = "SendHwm";
  }
  zmsg_zmq("Set %s option: %d", name, hwm);
  return SetOpt(option_name, &hwm, sizeof(hwm));
}

int ZmqSocket::SetLinger(int linger){
  zmsg_zmq("Set Linger option: %d", linger);
  return SetOpt(ZMQ_LINGER, &linger, sizeof(linger));
}

int ZmqSocket::SetId(const char *id, int len){
  char buf[128];
  int ln = len;
  zdump_mix(buf, 128, (const unsigned char*)id, &ln);
  zmsg_zmq("Set socket ID: %s", buf);
  return SetOpt(ZMQ_IDENTITY, id, len);
}

int ZmqSocket::SubAll(){
  return Subscribe("", 0);
}

int ZmqSocket::Subscribe(const char *filter, int len){
  char buf[128];
  int ln = len;
  zdump_mix(buf, 128, (const unsigned char*)filter, &ln);
  zmsg_zmq("Subscribe :%s", buf);
  return SetOpt(ZMQ_SUBSCRIBE, filter, len);
}

int ZmqSocket::Unsubscribe(const char *filter, int len){
  char buf[128];
  int ln = len;
  zdump_mix(buf, 128, (const unsigned char*)filter, &ln);
  zmsg_zmq("Unsubscribe :%s", buf);
  return SetOpt(ZMQ_UNSUBSCRIBE, filter, len);
}

int ZmqSocket::GetOpt(int option_name, void *option_value, size_t *option_len){
  int ret = zmq_getsockopt(sockHandle, option_name, option_value, option_len);
  if(-1 == ret){
    ZMQERR();
  }
  return ret;
}
int ZmqSocket::SetOpt(int option_name, const void *option_value, size_t option_len){
  int ret = zmq_setsockopt(sockHandle, option_name, option_value, option_len);
  if(-1 == ret){
    ZMQERR();
  }
  return ret;
}

ZmqMsg::ZmqMsg(){
  zmq_msg_init(&msg); //always return 0
}

ZmqMsg::ZmqMsg(ZmqMsg *copy){
  zmq_msg_init(&msg);
  Copy(copy);
}

ZmqMsg::ZmqMsg(int *initOk, size_t size){
  *initOk = zmq_msg_init_size(&msg, size);
  if(-1 == *initOk){
    ZMQERR();
  }
}

ZmqMsg::ZmqMsg(int *initOk, void* data, size_t size, zmq_free_fn *ffn, void*hint){
  *initOk = zmq_msg_init_data(&msg, data, size, ffn, hint);
  if(-1 == *initOk){
    ZMQERR();
  }
}
ZmqMsg::~ZmqMsg(){
#if ZTRACE_ZMQ
  //zdbg_zmq("%d = zmq_msg_close(%p)", zmq_msg_close(&msg), &msg);
  zmq_msg_close(&msg);
#else
  zmq_msg_close(&msg);
#endif
}

//int ZmqMsg::IsMore(){}
int ZmqMsg::Move(ZmqMsg *srcMsg){
  int ret = zmq_msg_move(&msg, &(srcMsg->msg));
  if(-1 == ret){
    ZMQERR();
  }
  return ret;
}
int ZmqMsg::Copy(ZmqMsg *srcMsg){
  int ret = zmq_msg_copy(&msg, &(srcMsg->msg));
  if(-1 == ret){
    ZMQERR();
  }
  return ret;
}
void *ZmqMsg::Data(){
  return zmq_msg_data(&msg);
}
size_t ZmqMsg::Size(){
  return zmq_msg_size(&msg);
}

void ZmqMsg::Dump(int isSend){
  if(!(ZmqSocket::traceFlag & ZTRACE_FLAG_MSG)){
    return; // control debug
  }
  char buf[4096] = {0};
  size_t offset = 0;
  size_t size= Size();
  int len = (int)size;
  unsigned char *data = (unsigned char*)Data();
  
  while(len){
    zdump_mix(buf, 3900, data+offset, &len);
    zmsg_zmq("%s zmq_msg<size:%d>: %s", isSend ? "Send" : "Recv", size, buf);
    offset += len;
    len = size - offset;
  }
}
