#include "log.hpp"
#include "pool/sqlconnpool.hpp"
#include <cassert>
#include <mysql/mysql.h>

sqlConnPool::sqlConnPool()
{
    _freeCount = 0;
    _useCount = 0;
}

sqlConnPool* sqlConnPool::getInstance()
{
    static sqlConnPool sqlPool;
    return &sqlPool;
}

void sqlConnPool::init(const char* host, int port, const char* user,
                       const char* pwd, const char* dbName, int connSize = 10)
{
    assert(connSize > 0);
    for (int i = 0; i < connSize; i++) {
        MYSQL* sql = nullptr;
        sql = mysql_init(sql);
        if (!sql) {
            LOG_ERROR("Mysql init error!The reason is:%s", mysql_error(sql));
            assert(sql);
        }
        sql = 
    }
}