#include "pool/sqlconnpool.hpp"
#include "log.hpp"
#include <cassert>
#include <mutex>
#include <mysql/mysql.h>
#include <semaphore.h>

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
            mysql_real_connect(sql, host, user, pwd, dbName, port, nullptr, 0);
        if (!sql) {
            LOG_ERROR("Mysql connect error!The reason is:%s", mysql_error(sql));
            assert(sql);
        }
        _connQue.push(sql);
        LOG_DEBUG("sqlconn:%d,has successful create!", i);
    }
    _MAX_CONN = connSize;
    sem_init(&_semId, 0, _MAX_CONN);
}

MYSQL* sqlConnPool::getConn()
{
    // 取出一个连接
    MYSQL* sql = nullptr;
    sem_wait(&_semId);
    {
        lock_guard<mutex> locker(_mtx);
        sql = _connQue.front();
        _connQue.pop();
    }
    return sql;
}

void sqlConnPool::freeConn(MYSQL* conn)
{
    assert(conn);
    lock_guard<mutex> locker(_mtx);
    _connQue.push(conn);
    sem_post(&_semId);
}

void sqlConnPool::closePool()
{
    // 关闭线程池，将所有的连接释放
    lock_guard<mutex> locker(_mtx);
    while (!_connQue.empty()) {
        auto item = _connQue.front();
        mysql_close(item);
        _connQue.pop();
    }
    mysql_library_end();
}

int sqlConnPool::getFreeCount()
{
    lock_guard<mutex> locker(_mtx);
    return _connQue.size();
}

sqlConnPool::~sqlConnPool()
{
    closePool();
}