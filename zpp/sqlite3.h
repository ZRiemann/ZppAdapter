/**
 * MIT License
 *
 * Copyright (c) 2018 Z.Riemann
 * https://github.com/ZRiemann/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the Software), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM
 * , OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef _ZPP_SQLITE3_H_
#define _ZPP_SQLITE3_H_

/**
 * @file sqlite3.h
 * @brief <A brief description of what this file is.>
 * @author Z.Riemann https://github.com/ZRiemann/
 * @date 2018-07-26 Z.Riemann found
 */
#ifndef NSB_Z
#define NSB_Z namespace z{
#define NSE_Z }
#endif

#include <string>

#include <sqlite3.h>
#include <zsi/base/error.h>
#include <zsi/base/trace.h>

NSB_Z

/**
 * @brief sqlite3 wrapper
 */
class Sqlite{
public:
    sqlite3 *db;
public:
    Sqlite()
        :db(NULL){}
    ~Sqlite(){Close();}
    zerr_t Open(const char *filename,
                int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
                const char *vfs = NULL){
        if(db){
            zerr("sqlite already open");
            return ZE_EXIST;
        }
        int err;
        if(SQLITE_OK != (err = sqlite3_open_v2(filename, &db, flags, vfs))){
            zerr("%s", sqlite3_errstr(err));
            return ZEFAIL;
        }
        return ZEOK;
    }
    zerr_t Close(){
        if(db){
            sqlite3_close_v2(db);
            db = NULL;
        }
    }

    /*
     * Configuring SQLite
     */
    /*
     * Extending SQLite
     */
    /*
     * Other Interfaces
     */
};

class SqliteStmt{
public:
    sqlite3 *db;
    sqlite3_stmt *stmt;
    const char *tail;
public:
    SqliteStmt()
        :stmt(NULL)
        ,tail(NULL){}
    ~SqliteStmt(){
        if(stmt){
            sqlite3_finalize(stmt);
        }
    }
    /**
     * @brief Complied sql statement into byte-code program.
     * @pre sqlte3_open()
     */
    zerr_t Prepare(Sqlite &sqldb, std::string &sql,
            unsigned int flags = 0){
        db = sqldb.db;
        if(!db){
            zerr("database not connected! call db.Open() first.");
            return ZEPARAM_INVALID;
        }
        if(sql.empty()){
            zerr("query is empty");
            return ZEPARAM_INVALID;
        }
        int err = sqlite3_prepare_v3(db, sql.data(), sql.length(), flags, &stmt, &tail);
        if(SQLITE_OK != err){
            zerr("%s", sqlite3_errstr(err));
            return ZEFAIL;
        }else{
            zinf("prepare: %s", sql.c_str());
        }
        return ZEOK;
    }
    /**
     * @brief Called one or more times to evaluate the statement.
     * @pre sqlite3_prepare()
     */
    int Step(){
        return sqlite3_step(stmt);
#if 0
        switch(ret){
        case SQLITE_BUSY:
            return ZEAGAIN;
        case SQLITE_DONE:
            return ZEOK;
        case SQLITE_ROW:
            return ZE_EXIST;
        case SQLITE_ERROR:
            zerr("%s", sqlite3_errmsg(db));
            return ZEFAIL;
        case SQLITE_MISUSE:
            return ZESTATUS_INVALID;
        default:
            return ZEFAIL;
        }
        return ZEFAIL;
#endif
    }
    const void *GetBlob(int column, int *len){
        const void *ret =  sqlite3_column_blob(stmt, column);
        *len = sqlite3_column_bytes(stmt, column);
        return ret;
    }
    double GetDouble(int column){return sqlite3_column_double(stmt, column);}
    sqlite3_int64 GetInt64(int column){return sqlite3_column_int64(stmt, column);}
    const unsigned char *GetText(int column, int *len){
        const unsigned char* text = sqlite3_column_text(stmt, column);
        *len = sqlite3_column_bytes(stmt, column);
        return text;
    }
    const void *GetText16(int column, int *len){
        const void *text = sqlite3_column_text16(stmt, column);
        *len = sqlite3_column_bytes16(stmt, column);
        return text;
    }
    sqlite3_value *GetValue(int column){return sqlite3_column_value(stmt, column);}

    /*
     * Binding Parameters and Reusing Prepared Statements
     * Parameter forms:
     *   ?
     *   ?<int>
     *   :<identifier>
     *   $<identifier>
     *   @<identifier>
     */
    /**
     * @brief Reset statement object back to its initial state.
     *
     * @par Does not change the values of any bindings on the prepared statment
     */
    zerr_t Reset(){
        return SQLITE_OK == sqlite3_reset(stmt) ? ZEOK : ZEFAIL;
    }
    /**
     * @brief Binding values to prepared statements.
     * @param idx [in] index of parameter, the leftmost has index of 1.
     * @param data [in] value to bind to the parameter
     * @param fn [in] dispose of data; SQLITE_STATIC/SQLITE_TRANSIENT
     * @par sqlite3_clear_bindings() to reset the binding
     *
     */
    zerr_t Bind(int idx, double data){
        zerr_t ret = SQLITE_OK == sqlite3_bind_double(stmt, idx, data) ? ZEOK : ZEFAIL;
        if(ZEFAIL == ret){
            zerr("%s", sqlite3_errmsg(db));
        }
        return ret;
    }
    zerr_t Bind(int idx, int data){
        zerr_t ret = SQLITE_OK == sqlite3_bind_int(stmt, idx, data) ? ZEOK : ZEFAIL;
        if(ZEFAIL == ret){
            zerr("%s", sqlite3_errmsg(db));
        }
        return ret;
    }
    zerr_t Bind(int idx, sqlite3_int64 data){
        zerr_t ret = SQLITE_OK == sqlite3_bind_int64(stmt, idx, data) ? ZEOK : ZEFAIL;
        if(ZEFAIL == ret){
            zerr("%s", sqlite3_errmsg(db));
        }
        return ret;
    }
    zerr_t Bind(int idx){
        zerr_t ret = SQLITE_OK == sqlite3_bind_null(stmt, idx) ? ZEOK : ZEFAIL;
        if(ZEFAIL == ret){
            zerr("%s", sqlite3_errmsg(db));
        }
        return ret;
    }
    zerr_t Bind(int idx, const sqlite3_value *data){
        zerr_t ret = SQLITE_OK == sqlite3_bind_value(stmt, idx, data) ? ZEOK : ZEFAIL;
        if(ZEFAIL == ret){
            zerr("%s", sqlite3_errmsg(db));
        }
        return ret;
    }
    zerr_t BindBlob(int idx, const void *data, int n, void(*fn)(void*)){
        zerr_t ret = SQLITE_OK == sqlite3_bind_blob(stmt, idx, data, n, fn) ? ZEOK : ZEFAIL;
        if(ZEFAIL == ret){
            zerr("%s", sqlite3_errmsg(db));
        }
        return ret;
    }
    zerr_t BindBlob64(int idx, const void *data, sqlite3_uint64 n, void(*fn)(void*)){
        zerr_t ret = SQLITE_OK == sqlite3_bind_blob64(stmt, idx, data, n, fn) ? ZEOK : ZEFAIL;
        if(ZEFAIL == ret){
            zerr("%s", sqlite3_errmsg(db));
        }
        return ret;
    }
    zerr_t BindText(int idx, const char *data, int len, void(*fn)(void*)){
        zerr_t ret = SQLITE_OK == sqlite3_bind_text(stmt, idx, data, len, fn) ? ZEOK : ZEFAIL;
        if(ZEFAIL == ret){
            zerr("%s", sqlite3_errmsg(db));
        }
        return ret;
    }
    zerr_t BindText16(int idx, const char *data, int len, void(*fn)(void*)){
        zerr_t ret = SQLITE_OK == sqlite3_bind_text16(stmt, idx, data, len, fn) ? ZEOK : ZEFAIL;
        if(ZEFAIL == ret){
            zerr("%s", sqlite3_errmsg(db));
        }
        return ret;
    }
    zerr_t BindText64(int idx, const char *data, sqlite3_uint64 len,
                      void(*fn)(void*), unsigned char encoding = SQLITE_UTF8){
        zerr_t ret = SQLITE_OK == sqlite3_bind_text64(stmt, idx, data, len, fn, encoding)\
            ? ZEOK : ZEFAIL;
        if(ZEFAIL == ret){
            zerr("%s", sqlite3_errmsg(db));
        }
        return ret;
    }
    zerr_t BindPointer(int idx, void *data, const char *pc, void(*fn)(void*)){
        zerr_t ret = SQLITE_OK == sqlite3_bind_pointer(stmt, idx, data, pc, fn) ? ZEOK : ZEFAIL;
        if(ZEFAIL == ret){
            zerr("%s", sqlite3_errmsg(db));
        }
        return ret;
    }
    zerr_t BindZeroBlob(int idx, int data){
        zerr_t ret = SQLITE_OK == sqlite3_bind_zeroblob(stmt, idx, data) ? ZEOK : ZEFAIL;
        if(ZEFAIL == ret){
            zerr("%s", sqlite3_errmsg(db));
        }
        return ret;
    }
    zerr_t BindZeroBlob64(int idx, sqlite3_uint64 data){
        zerr_t ret = SQLITE_OK == sqlite3_bind_zeroblob64(stmt, idx, data) ? ZEOK : ZEFAIL;
        if(ZEFAIL == ret){
            zerr("%s", sqlite3_errmsg(db));
        }
        return ret;
    }
};

NSE_Z

#endif /*_ZPP_SQLITE3_H_*/
