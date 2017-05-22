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
#include <map>
#include <zit/thread/mutex.h>

namespace z{
  namespace zmq{
    /**@class z::zmq::Msg
     * @brief zmq消息封装
     */
    class Msg{
    public:
      Msg();// zmq_msg_init
      Msg(Msg *copy); // copy msg
      Msg(int *initOk, size_t size); // zmq_msg_init_size
      Msg(int *initOk, void* data, size_t size, zmq_free_fn *ffn, void*hint); // zmq_msg_init_data
      ~Msg(); // zmq_msg_close
      
      //int IsMore(); // call socket
    public:
      int Move(Msg *srcMsg){return zmq_msg_move(&msg, &(srcMsg->msg));}
      int Copy(Msg *srcMsg){return zmq_msg_copy(&msg, &(srcMsg->msg));}
      
      void *Data(){return zmq_msg_data(&msg);}
      size_t Size(){return zmq_msg_size(&msg);}
      void Dump(int isSend);
      // get/set property...
      //protected:
    public:
      zmq_msg_t msg;
    };

    /**@class z::zmq::Socket
     * @brief zmq_socket 封装
     */
    typedef std::vector<std::string> Endpoints;
    class Socket{
    public:
      static int Init(bool setLinger0 = true); // 初始化zmq,程序开始时调用
      static int Fini(); // 反初始化zmq,程序结束是调用
      
      Socket(int sockType); // 设置默认收发高水位100
      Socket(int sockType, const char *id); // 创建命名套接字，id为字符串
      ~Socket();
      
      int Bind(const char *endpoint);
      int Connect(const char *endpoint);
      
      int MsgSend(Msg *msg, int flags); // zmq_msg_send
      int MsgRecv(Msg *msg, int flags); // zmq_msg_recv

      int GetOpt(int option_name, void *option_value, size_t *option_len);
      int SetOpt(int option_name, const void *option_value, size_t option_len);
      int IsMore(); // test has more frame
      int SetHwm(int is_send, int hwm); //
      int SetLinger(int linger);
      int SetId(const char *id, int len);
      //protected:
      
    public:
      // lazy pirate pattern for ZMQ_REQ socket, return ZOK or Z*
      int LazyPirateReq(Msg *msgReq, Msg *msgRep, int timeout_sec = 5, int trys = 2);
      int Reconnect(); // close and reconnect;
      
    public: // SUB interface
      int SubAll(); // sub all message
      int Subscribe(const char *filter, int len);
      int Unsubscribe(const char *filter, int len);
    public:
      static int traceFlag;
      static void *s_ctx; ///< zmq context
      
      void *sockHandle; ///< an opaque handle to the socket
      // for request
      int sockType;
      Endpoints endpoints;
      
    };
    
    //class Device{};
    
    /**@class z::zmq::Connecter
     * @brief 维护多线程ZMQ的长连接,保证zmq socket线程专用;
     * @note
     *   1. 优化短连接>6ms
     *   2. GetConn 动态建立线程专属连接
     *   3. CloseConn 关闭线程连接
     *   4. 各线可以保证自己的id
     */
    class Connecter{
    public:
      Connecter();
      ~Connecter();
      
      z::zmq::Socket * Connect(const std::string &endpoint, int sockType = ZMQ_REQ); // 按需获取
      // int AddConn(z::zmq::Socket *sock, std::vector<std::string> endps); // 添加自主连接
      void Close(); // 线程id可复用，线程退出必须调用。
    
    protected:
      zmutex_t mtx;
      typedef std::map<std::string, z::zmq::Socket*> ep2sock; 
      std::map< uint32_t, ep2sock* > tid2ep;
    };
  }
}
#endif
