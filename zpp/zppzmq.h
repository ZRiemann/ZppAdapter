#ifndef _ZPPZMQ_H_
#define _ZPPZMQ_H_
/**@file zpp/zppzmq.h
 * @brief ZeroMQ C++ 封装类, 提供编程效率
 * @note
 * 1. #include <zpp/zppzmq.h> 方便头文件管理
 *    #include <zmq.h> libzmq 必须预先安装并配置好环境变量
 * 2. 
 */
#include <zmq.h>
#include <vector>
#include <string>

namespace z{
  class ZmqMsg{
  public:
    ZmqMsg();// zmq_msg_init
    ZmqMsg(ZmqMsg *copy); // copy msg
    ZmqMsg(int *initOk, size_t size); // zmq_msg_init_size
    ZmqMsg(int *initOk, void* data, size_t size, zmq_free_fn *ffn, void*hint); // zmq_msg_init_data
    ~ZmqMsg(); // zmq_msg_close

    //int IsMore(); // call socket 
    int Move(ZmqMsg *msg);
    int Copy(ZmqMsg *msg);
    
    void *Data();
    size_t Size();
    void Dump(int isSend);
    // get/set property...
    //protected:
  public:
    zmq_msg_t msg;
  };

  typedef std::vector<std::string> Endpoints;
  class ZmqSocket{
  public:
    static int ZmqInit(bool setLinger0 = true); // 初始化zmq,程序开始时调用
    static int ZmqFini(); // 反初始化zmq,程序结束是调用

    ZmqSocket(int sockType); // 设置默认收发高水位100
    ZmqSocket(int sock, const char *id); // 创建命名套接字，id为字符串
    ~ZmqSocket();

    int Bind(const char *endpoint);
    int Connect(const char *endpoint);
    
    int MsgSend(ZmqMsg *msg, int flags); // zmq_msg_send
    int MsgRecv(ZmqMsg *msg, int flags); // zmq_msg_recv

    int GetOpt(int option_name, void *option_value, size_t *option_len);
    int SetOpt(int option_name, const void *option_value, size_t option_len);
    int IsMore(); // test has more frame
    int SetHwm(int is_send, int hwm); //
    int SetLinger(int linger);
    int SetId(const char *id, int len);
    //protected:

  public:
    // lazy pirate pattern for ZMQ_REQ socket, return ZOK or Z*
    int LazyPirateReq(ZmqMsg *msgReq, ZmqMsg *msgRep, int timeout_sec = 5, int trys = 2);
    int Reconnect(); // close and reconnect;

  public: // SUB interface
    int SubAll(); // sub all message
    int Subscribe(const char *filter, int len);
    int Unsubscribe(const char *filter, int len);
  public:
    static void *s_ctx; ///< zmq context

    void *sockHandle; ///< an opaque handle to the socket
    // for request
    int sockType;
    Endpoints endpoints;
    
  };

  class ZmqDevice{
    
  };
}
#endif
