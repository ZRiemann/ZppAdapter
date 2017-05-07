#ifndef _ZPP_MYSQL_H_
#define _ZPP_MYSQL_H_
/**@file zpp/zppmysql.h
 * @brief MySQL API封装类
 *
 */
#include <mysql/mysql.h>
#include <zit/base/type.h>

namespace z{
  class MySql{
  public:
    MYSQL *mysql;
    MYSQL_RES *result;
    //MYSQL_ROW row;
    static int traceFlag;

  public:
    MySql();
    ~MySql();
    int SetOption(enum mysql_option option, const void *arg);//MYSQL_OPT_CONNECT/READ/WRITE_TIMEOUT
    int Connect(const char *user, const char *passwd, const char *db, const char *host = "localhost",  unsigned int port = 0);
    // int Reconnect();
    int Query(const char *query);
    
    inline uint64_t GetIncId(){return (uint64_t)mysql_insert_id(mysql);}
    inline bool IsError(){return mysql_errno(mysql) ? true : false;} 
    inline bool Commit(){return mysql_commit(mysql);}
    inline bool RollBack(){return mysql_rollback(mysql);}
    inline bool Ping(){return mysql_ping(mysql);}
    inline int SelectDb(const char *db){return mysql_select_db(mysql, db);}
    inline int SetCharSet(const char *set){return mysql_set_character_set(mysql, set);}// "utf8"
    
    inline int StoreResult(){result = mysql_store_result(mysql); return result?1:0;}
    inline int UseResult(){result = mysql_use_result(mysql); return result?1:0;}
    
    uint64_t GetRows();
    
    inline void FreeResult(){if(result){mysql_free_result(result);}}
    //int CreateDb(const char *db);
    //int DropDb(const char *db);
      //public: //预处理语句
    // mysql_stmt_* task delay...
      //public: // 线程安全
      //inline int IsThreadSafe(){return mysql_thread_safe();}
      //inline void ThreadEnd(){mysql_thread_end();}
    
  };
}

#endif
