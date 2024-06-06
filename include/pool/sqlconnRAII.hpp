#ifndef SQLRAII_H
#define SQLRAII_H
#include "sqlconnpool.hpp"
#include <cassert>

class sqlConnRAII {
public:
    sqlConnRAII(MYSQL** sql, sqlConnPool* connpool)
    {
        assert(connpool);
        *sql = connpool->getConn();
        _sql = *sql;
        _connPool = connpool;
    }
    ~sqlConnRAII()
    {
        if (_sql) {
            _connPool->freeConn(_sql);
        }
    }

private:
    MYSQL* _sql;
    sqlConnPool* _connPool;
};

#endif