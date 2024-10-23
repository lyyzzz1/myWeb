/*
设计一个队列，用于异步输出日志
*/

#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>

using namespace std;

template <class T> class blockDeque {
public:
    explicit blockDeque(size_t maxCapacity = 1000); //构造函数，默认任务数为1000
    ~blockDeque();
    // 清除队列
    void clear();
    // 判断是否为空
    bool empty();
    // 判断是否满了
    bool full();
    // 关闭队列
    void close();
    // 获取队列大小
    size_t size();
    // 获取队列容量
    size_t capacity();
    // 提取队头元素
    T front();
    // 提取队尾元素
    T back();
    // 在队尾加入元素
    void push_back(const T& item);
    // 在队头加入元素
    void push_front(const T& item);
    // 弹出元素
    bool pop(T& item);
    bool pop(T& item, int timeout);
    // 通知消费者线程即写线程目前队列中有任务可以取出写
    void flush();

private:
    deque<T> _deque;  //队列
    size_t _capacity; //容量大小
    mutex _mtx;       //操作队列的互斥锁
    bool _isClose;    //关闭标识
    condition_variable
        _producer; //用于通知生产者的信号量，当任务队列满时等待pop出任务后使用这个信号量通知push
    condition_variable
        _comsumer; //用于通知消费者的信号量，当任务队列空时等待push进任务后使用这个信号量通知pop
};

template <class T>
blockDeque<T>::blockDeque(size_t maxCapacity) : _capacity(maxCapacity)
{
    assert(maxCapacity > 0);
    _isClose = false;
}

template <class T> blockDeque<T>::~blockDeque()
{
    close();
}

template <class T> void blockDeque<T>::close()
{
    {
        lock_guard<mutex> lock(_mtx);
        _deque.clear();
        _isClose = true;
    }
    _comsumer.notify_all();
    _producer.notify_all();
}

template <class T> void blockDeque<T>::flush()
{
    _comsumer.notify_one();
}

template <class T> void blockDeque<T>::clear()
{
    lock_guard<mutex> lock(_mtx);
    _deque.clear();
}

template <class T> bool blockDeque<T>::empty()
{
    lock_guard<mutex> lock(_mtx);
    return _deque.empty();
}

template <class T> T blockDeque<T>::front()
{
    lock_guard<mutex> lock(_mtx);
    return _deque.front();
}

template <class T> T blockDeque<T>::back()
{
    lock_guard<mutex> lock(_mtx);
    return _deque.back();
}

template <class T> size_t blockDeque<T>::size()
{
    lock_guard<mutex> lock(_mtx);
    return _deque.size();
}

template <class T> size_t blockDeque<T>::capacity()
{
    lock_guard<mutex> lock(_mtx);
    return _capacity;
}

template <class T> void blockDeque<T>::push_back(const T& item)
{
    // 获取操作队列的互斥锁
    unique_lock<mutex> locker(_mtx);
    while (_deque.size() >= _capacity) {
        // 等待线程通知
        _producer.wait(locker);
    }
    _deque.push_back(item);
    // 成功插入后通知消费者
    _comsumer.notify_one();
}

template <class T> void blockDeque<T>::push_front(const T& item)
{
    // 获取操作队列的互斥锁
    unique_lock<mutex> locker(_mtx);
    while (_deque.size() >= _capacity) {
        // 等待线程通知
        _producer.wait(locker);
    }
    _deque.push_front(item);
    // 成功插入后通知消费者
    _comsumer.notify_one();
}

template <class T> bool blockDeque<T>::full()
{
    lock_guard<mutex> lock(_mtx);
    return _deque.size() >= _capacity;
}

template <class T> bool blockDeque<T>::pop(T& item)
{
    // 获取任务
    unique_lock<mutex> locker(_mtx);
    while (_deque.empty()) { //任务队列空时没有可消费的
        _comsumer.wait(locker);
        if (_isClose) {
            return false;
        }
    }
    item = _deque.front();
    _deque.pop_front();
    _producer.notify_one();
    return true;
}

template <class T> bool blockDeque<T>::pop(T& item, int timeout)
{
    // 获取任务
    unique_lock<mutex> locker(_mtx);
    while (_deque.empty()) { //任务队列空时没有可消费的
        if (_comsumer.wait_for(locker, chrono::seconds(timeout)) ==
            cv_status::timeout) {
            return false;
        }
        if (_isClose) {
            return false;
        }
    }
    item = _deque.front();
    _deque.pop_front();
    _producer.notify_one();
    return true;
}

#endif