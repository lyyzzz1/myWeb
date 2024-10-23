#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mutex>
#include <mysql/mysql.h>
#include <queue>
#include <semaphore.h>

using namespace std;

class sqlConnPool {
public:
    static sqlConnPool* getInstance();

    MYSQL* getConn();
    void freeConn(MYSQL * conn);
    int getFreeCount();

    void init(const char* host, int port, const char* user, const char* pwd,
              const char* dbName, int connSize);
    void closePool();

private:
    sqlConnPool();
    ~sqlConnPool();

    int _MAX_CONN;
    int _useCount;
    int _freeCount;

    queue<MYSQL*> _connQue;
    mutex _mtx;
    sem_t _semId;
};

#endif