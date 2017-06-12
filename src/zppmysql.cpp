/**@file src/zppmysql.cpp
 * @brief mysqlclient 封装类实现
 * @note
 * @history 
 *   2017-5-7 ZRiemann found
 * @memo
 * 1. 调用流程:
 *   mysql_library_init()->mysql_init()->mysql_real_connect()
 *   ->...->mysql_close()->mysql_library_end();
 * 2. 错误检出:
 *   mysql_errno(), mysql_error(MYSQL*)
 * 3. ubuntu 安装
 *   sudo apt-get install mysql-server/mysql-client/libmysqlclient-dev
 * 4. 编译连接
 *   $(CC) -o zpp_test -lmysqlclient -lzit -lrt
 * 5. 客户端命令
 *   $mysql -u root -p
 *   password: root
 *   >show databases; #显示数据库列表
 *   >use mysql; #进入mysql数据库
 *   >show tables; #列出mysql数据库中的表
 * 6. C API
 *   数据类型
 *   MYSQL:数据库连接局部；
 *   MYSQL_RES:结果集
 *   MYSQL_ROW:数据行
 *   MYSQL_FIELD:字段信息，名称、类型、大小;mysq_fetch_field()
 *   MYSQL_FIELD_OFFSET:mysql_field_seek()
 *   my_ulonglong: mysql_affected_rows/num_rows/insert_id();
 *                 printf("Number of rows: %lu\n",(unsigned long)mysql_num_rows(result));
 *   API 函数
 *   
 */
#include <zpp/zppmysql.h>
#include <zit/base/trace.h>

#define zdbg_mysql(fmt, ...) zdbgx(MySql::traceFlag, "[ln:%04d fn:%s]\t" fmt, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define zmsg_mysql(fmt, ...) zmsgx(MySql::traceFlag, "[ln:%04d fn:%s]\t" fmt, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define zwar_mysql(fmt, ...) zwarx(MySql::traceFlag, "[ln:%04d fn:%s]\t" fmt, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define zerr_mysql(fmt, ...) zerrx(MySql::traceFlag, "[ln:%04d fn:%s]\t" fmt, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define zinf_mysql(fmt, ...) zinfx(MySql::traceFlag, "[ln:%04d fn:%s]\t" fmt, __LINE__, __FUNCTION__, ##__VA_ARGS__)

using namespace z;

int MySql::traceFlag = 0xff;

MySql::MySql(){
  mysql = mysql_init(NULL);
  if(!mysql){
    ZERRC(ZMEM_INSUFFICIENT);
  }else{
    zdbg_mysql("mysql_init(%p)", mysql);
    unsigned int value = 3; // 3sec connect timeout
    SetOption(MYSQL_OPT_CONNECT_TIMEOUT, &value);
    my_bool reconn = true;
    SetOption(MYSQL_OPT_RECONNECT, &reconn);
    traceFlag = traceFlag;
  }
}

MySql::~MySql(){
  if(mysql){
    mysql_close(mysql);
  }
  zdbg_mysql("mysql_close(%p)", mysql);
}

int MySql::SetOption(enum mysql_option option, const void *arg){
  int ret;

  ret = mysql_options(mysql, option, arg);
  if(ret){
    ret = ZFUN_FAIL;
    const char *err = mysql_error(mysql);
    ZERR("%s", err);
  }
  return ret;
}

int MySql::Connect(const char *user, const char *passwd, const char *db, const char *host,  unsigned int port){
  zmsg_mysql("connect(user:%s pwd:%s db:%s host:%s port:%d)", user, passwd, db, host, port);
  if(NULL == mysql_real_connect(mysql, host, user, passwd, db, port, NULL, 0)){
    const char *err = mysql_error(mysql);
    zerr_mysql("%s", err);
  }
  return ZOK;
}

int MySql::Query(const char *query){
  int ret;
  ZDBG("%s", query);
  if(-1 == (ret = mysql_query(mysql, query))){
    const char *err = mysql_error(mysql);
    ZERR("%s", err);    
  }
  return ret;
}

uint64_t MySql::GetAftRows(){
  uint64_t rows = (uint64_t)mysql_affected_rows(mysql);
  zdbg_mysql("affected row<%lu>", rows);
  return rows;

}
/*
int MySql::CreateDb(const char *db){
  if(!mysql || !db){
    return ZFUN_FAIL;
  }
  int ret = mysql_create_db(mysql, db);
  if(ret){
    ret = ZFUN_FAIL;
    const char *err = mysql_error(mysql);
    ZERR("%s", err);
  }else{
    zmsg_mysql("create db: %s", db);
  }
  return ret;
}

int MySql::DropDb(const char *db){
  if(!mysql || !db){
    return ZFUN_FAIL;
  }
  int ret = mysql_drop_db(mysql, db);
  if(ret){
    ret = ZFUN_FAIL;
    const char *err = mysql_error(mysql);
    ZERR("%s", err);
  }else{
    zmsg_mysql("drop db: %s", db);
  }
  return ret;
}
*/
#if 0
// blob insert demo
char query[1000],*end;
 
end = strmov(query,"INSERT INTO test_table values(");
*end++ = '\'';
end += mysql_real_escape_string(&mysql, end,"What's this",11);
*end++ = '\'';
*end++ = ',';
*end++ = '\'';
end += mysql_real_escape_string(&mysql, end,"binary data: \0\r\n",16);
*end++ = '\'';
*end++ = ')';
 
if (mysql_real_query(&mysql,query,(unsigned int) (end - query)))
{
   fprintf(stderr, "Failed to insert row, Error: %s\n",
           mysql_error(&mysql));
}
#endif
