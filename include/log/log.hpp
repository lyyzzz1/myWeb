/*
实现一个自定义的日志功能
*/

#ifndef LOG_H
#define LOG_H

#include "buffer.hpp"
#include "blockqueue.hpp"
#include <cstdio>
#include <memory>
#include <thread>
class Log {
public:
    void init(int level, const char* path = "./log",
              const char* suffix = ".log", int maxQueueCapacity = 1024);
    static Log* getInstance();
    static void flushLogThread();

    void write(int level, const char* format, ...);
    void flush();

    int getLevel();
    void setLevel(int level);
    bool isOpen()
    {
        return _isOpen;
    }

private:
    Log();
    void _appendLogLevelTitle(int level);
    virtual ~Log();
    void _asyncWrite();

private:
    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;

    const char* _path;
    const char* _suffix;

    int _MAX_LINES;

    int _lineCount;
    int _today;

    bool _isOpen;

    Buffer _buffer;
    int _level;
    bool _isAsync;

    FILE* _fp;
    unique_ptr<blockDeque<string> > _deque;
    unique_ptr<thread> _writeThread;
    mutex _mtx;
};

#define LOG_BASE(level, format, ...) \
    do {\
        Log* log = Log::getInstance();\
        if (log->isOpen() && log->getLevel() <= level) {\
            log->write(level, format, ##__VA_ARGS__); \
            log->flush();\
        }\
    } while(0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif