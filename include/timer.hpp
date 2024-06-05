/*
实现一个堆计时器，可以做到定时清理非活跃的连接
*/

#ifndef TIMER_H
#define TIMER_H

#include "log.hpp"
#include <chrono>
#include <cstddef>
#include <cassert>
#include <utility>
#include <functional>
#include <unordered_map>
#include <vector>

using timeoutCallback = function<void()>;
using Clock = chrono::high_resolution_clock;
using MS = chrono::milliseconds;
using timeStamp = Clock::time_point;

// 节点记录了id，定时时间以及重载了<来比较两个节点之间的超时时间
struct timerNode {
    int id;
    timeStamp expires;
    timeoutCallback cb;
    bool operator<(const timerNode& t)
    {
        return expires < t.expires;
    }
};

class heapTimer {
public:
    heapTimer()
    {
        _heap.reserve(64);
    };
    ~heapTimer()
    {
        clear();
    };
    void adjust(int id, int newExpires);
    void add(int id, int timeOut, const timeoutCallback& cb);
    void doWork(int id);
    void clear();
    void tick();
    void pop();
    int getNextTick();

private:
    // 删除指定索引的定时器
    void _del(size_t i);
    // 向上调整堆
    void _siftup(size_t i);
    // 向下调整堆
    bool _siftdown(size_t index,size_t n);
    // 交换两个节点
    void _swapNode(size_t i, size_t j);

private:
    // 维护一个堆
    vector<timerNode> _heap;
    // 映射定时器ID到堆索引
    unordered_map<int, size_t> _ref;
};

#endif