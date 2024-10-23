#include "log.hpp"
#include "blockqueue.hpp"
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <memory>
#include <mutex>
#include <sys/stat.h>
#include <sys/time.h>
#include <thread>
#include <utility>

Log::Log()
    : _lineCount(0), _isAsync(false), _writeThread(nullptr), _deque(nullptr),
      _today(0), _fp(nullptr)
{
}

Log::~Log()
{
    if (_writeThread && _writeThread->joinable()) {
        while (!_deque->empty()) {
            _deque->flush();
        }
        _deque->close();
        _writeThread->join();
    }
    if (_fp) {
        lock_guard<mutex> locker(_mtx);
        flush();
        fclose(_fp);
    }
}

int Log::getLevel()
{
    lock_guard<mutex> lock(_mtx);
    return _level;
}

void Log::setLevel(int level)
{
    lock_guard<mutex> lock(_mtx);
    _level = level;
}

void Log::init(int level = 1, const char* path, const char* suffix,
               int maxQueueSize)
{
    _isOpen = true;
    _level = level;
    if (maxQueueSize > 0) {
        _isAsync = true; // 开启异步写入模式，使用blockDeque队列
        if (!_deque) {   // 目前队列为空
            unique_ptr<blockDeque<string> > newDeque(
                new blockDeque<string>(maxQueueSize));
            _deque = move(newDeque);

            unique_ptr<thread> newThread(new thread(flushLogThread));
            _writeThread = move(newThread);
        }
    } else {
        _isAsync = false;
    }

    _lineCount = 0;

    time_t timer = time(nullptr);
    struct tm* sysTime = localtime(&timer);
    struct tm t = *sysTime;
    _path = path;
    _suffix = suffix;
    char fileName[LOG_NAME_LEN] = {0};
    snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s", _path,
             t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, _suffix);
    _today = t.tm_mday;

    {
        lock_guard<mutex> locker(_mtx);
        _buffer.retrieveAll(); // 清空缓冲区
        if (_fp) {
            flush();
            fclose(_fp);
        }

        _fp = fopen(fileName, "a");
        if (_fp == nullptr) {
            mkdir(_path, 0777);
            _fp = fopen(fileName, "a");
        }
        assert(_fp != nullptr);
    }
}

void Log::write(int level, const char* format, ...)
{
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm* systime = localtime(&tSec);
    struct tm t = *systime;
    va_list vaList;


    //如果当前日期与 _today 不同（跨天了），或者(_lineConut 不为零并且 _lineConut 是 MAX_LINES 的整数倍（写入了足够多的日志行）
    if (_today != t.tm_mday || (_lineCount && (_lineCount % MAX_LINES == 0))) {
        unique_lock<mutex> locker(_mtx);
        locker.unlock();

        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1,
                 t.tm_mday);

        if (_today != t.tm_mday) {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", _path, tail,
                     _suffix);
            _today = t.tm_mday;
            _lineCount = 0;
        } else {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", _path, tail,
                     (_lineCount / MAX_LINES), _suffix);
        }

        locker.lock();
        flush();
        fclose(_fp);
        _fp = fopen(newFile, "a");
        assert(_fp != nullptr);
    }

    {
        unique_lock<mutex> locker(_mtx);
        _lineCount++;
        int n = snprintf(_buffer.beginWrite(), 128,
                         "%d-%02d-%02d %02d:%02d:%02d.%06ld ", t.tm_year + 1900,
                         t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec,
                         now.tv_usec);
        _buffer.hasWritten(n);
        _appendLogLevelTitle(level);

        va_start(vaList, format);
        int m = vsnprintf(_buffer.beginWrite(), _buffer.writableBytes(), format,
                          vaList);
        va_end(vaList);

        _buffer.hasWritten(m);
        _buffer.append("\n\0", 2);

        if (_isAsync && _deque && !_deque->full()) {
            _deque->push_back(_buffer.retrieveAllToStr());
        } else {
            fputs(_buffer.peek(), _fp);
        }
        _buffer.retrieveAll();
    }
}

void Log::_appendLogLevelTitle(int level)
{
    switch (level) {
        case 0:
            _buffer.append("[debug]: ", 9);
            break;
        case 1:
            _buffer.append("[info] : ", 9);
            break;
        case 2:
            _buffer.append("[warn] : ", 9);
            break;
        case 3:
            _buffer.append("[error]: ", 9);
            break;
        default:
            _buffer.append("[info] : ", 9);
            break;
    }
}

void Log::flush()
{
    if (_isAsync) {
        // 通知消费者可以取出string进行写入了
        _deque->flush();
    }
    fflush(_fp);
}

void Log::_asyncWrite()
{
    string str = "";
    while (_deque->pop(str)) {
        lock_guard<mutex> locker(_mtx);
        fputs(str.c_str(), _fp);
    }
}

Log* Log::getInstance()
{
    static Log instance;
    return &instance;
}

void Log::flushLogThread()
{
    Log::getInstance()->_asyncWrite();
}