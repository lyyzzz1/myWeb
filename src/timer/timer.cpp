#include "timer.hpp"
#include <cassert>
#include <chrono>
#include <cstddef>

void heapTimer::_swapNode(size_t i, size_t j)
{
    assert(i >= 0 && i < _heap.size());
    assert(j >= 0 && j < _heap.size());
    swap(_heap[i], _heap[j]);
    // 修改映射
    _ref[_heap[i].id] = j;
    _ref[_heap[j].id] = i;
}

void heapTimer::_siftup(size_t i)
{
    // 自下向上调整堆，先找到i的父节点，不断调整直到根节点
    assert(i >= 0 && i < _heap.size());
    // 一个节点的左子节点为 2 * i + 1,右子节点为 2 * i + 2
    size_t j = (i - 1) / 2; // j 为父节点的索引
    while (j >= 0) {
        if (_heap[j] < _heap[i]) {
            break;
        }
        // 如果父节点比子节点大，交换两个节点
        _swapNode(i, j);
        // 更新 i的值，再次判断直到
        i = j;
        j = (i - 1) / 2;
    }
}

bool heapTimer::_siftdown(size_t index, size_t n)
{
    // 向下调整，范围为[index,n)
    assert(index >= 0 && index < _heap.size());
    assert(n >= 0 && n <= _heap.size());
    size_t i = index;
    size_t j = index * 2 + 1;
    while (j < n) {
        // 在调整小顶堆时需要维护小顶堆的特性
        // 即在从上向下调整时，需要保证交换后的父节点小于其两个子节点
        // 所以要选择两个子节点中较小的那一个来进行交换
        if (j + 1 < n && _heap[j + 1] < _heap[j]) {
            j++;
        }
        if (_heap[i] < _heap[j]) {
            break;
        }
        _swapNode(i, j);
        i = j;
        j = index * 2 + 1;
    }
    return i > index;
}

void heapTimer::add(int id, int timeout, const timeoutCallback& cb)
{
    assert(id >= 0);
    size_t i;
    if (_ref.count(id) == 0) {
        // 没有记录，即新节点，队尾插入然后调整堆
        i = _heap.size();
        _ref[id] = i; // id映射到索引
        _heap.push_back({id, Clock::now() + MS(timeout), cb});
        _siftup(i);
    } else {
        // 已有节点，需要调整堆
        i = _ref[id];
        _heap[i].expires = Clock::now() + MS(timeout);
        _heap[i].cb = cb;
        if (!_siftdown(i, _heap.size())) {
            // 这样写的目的是如果当前需要调整的节点的过期时间小于其子节点则不会下沉
            // 但是无法保证其大于其父节点，所以如果不下沉就要执行上浮操作维护性质
            _siftup(i);
        }
    }
}

void heapTimer::doWork(int id)
{
    // 删除指定的id的节点，并执行其cb函数
    if (_heap.empty() || _ref.count(id) == 0) {
        return;
    }
    size_t i = _ref[id];
    auto cb = _heap[i].cb;
    cb();
    _del(i);
}

void heapTimer::_del(size_t index)
{
    // 删除指定位置的节点
    assert(!_heap.empty() && index >= 0 && index < _heap.size());
    // 将想要删除的节点调整到队尾，然后调整堆
    size_t i = index;
    size_t n = _heap.size() - 1;
    assert(i <= n);
    if (i < n) {
        _swapNode(i, n);
        if (!_siftdown(i, n)) {
            _siftup(i);
        }
    }
    _ref.erase(_heap.back().id);
    _heap.pop_back();
}

void heapTimer::adjust(int id, int timeout)
{
    // 调整指定id的节点
    assert(!_heap.empty() && _ref.count(id) > 0);
    _heap[_ref[id]].expires = Clock::now() + MS(timeout);
    _siftdown(_ref[id], _heap.size());
}

void heapTimer::tick()
{
    // 清除超时节点
    if (_heap.empty()) {
        return;
    }
    while (!_heap.empty()) {
        timerNode node = _heap.front();
        if (chrono::duration_cast<MS>(node.expires - Clock::now()).count() >
            0) {
            break;
        }
        node.cb();
        pop();
    }
}

void heapTimer::pop()
{
    assert(!_heap.empty());
    _del(0);
}

void heapTimer::clear()
{
    _ref.clear();
    _heap.clear();
}

int heapTimer::getNextTick()
{
    tick();
    size_t res = -1;
    if (!_heap.empty()) {
        res = chrono::duration_cast<MS>(_heap.front().expires - Clock::now())
                  .count();
        if (res < 0) {
            res = 0;
        }
    }
    return res;
}