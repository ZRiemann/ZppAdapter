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
        int RealQuery(const char *query, unsigned long len){return mysql_real_query(mysql, query, len);}
        uint32_t RealEscape(char *to, const char *from, uint32_t len){return mysql_real_escape_string(mysql, to, from, len);}
        const char *StrErr(){return mysql_error(mysql);}
        uint64_t GetIncId(){return (uint64_t)mysql_insert_id(mysql);}
        bool IsError(){return mysql_errno(mysql) ? true : false;}
        bool Commit(){return mysql_commit(mysql);}
        bool RollBack(){return mysql_rollback(mysql);}
        bool Ping(){return mysql_ping(mysql);}
        int SelectDb(const char *db){return mysql_select_db(mysql, db);}
        int SetCharSet(const char *set){return mysql_set_character_set(mysql, set);}// "utf8"
        bool StoreResult(){result = mysql_store_result(mysql); return result?1:0;}
        bool UseResult(){result = mysql_use_result(mysql); return result?1:0;}
        uint32_t GetFields(){return (uint32_t)mysql_num_fields(result);}
        uint64_t GetAftRows();
        uint64_t GetNumRows(){return (uint64_t)mysql_num_rows(result);}
        unsigned long *FetchLens(){return mysql_fetch_lengths(result);}
        MYSQL_ROW FetchRow(){return mysql_fetch_row(result);}
        void FreeResult(){if(result){mysql_free_result(result); result = NULL;}}
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
