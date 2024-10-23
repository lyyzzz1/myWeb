#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <algorithm>
#include <cassert>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
using namespace std;

class threadPool {
public:
    explicit threadPool(size_t threadCount = 8) : _pool(make_shared<pool>())
    {
        assert(threadCount > 0);
        for (size_t i = 0; i < threadCount; i++) {
            thread([pool = _pool]() {
                // 从pool中取得任务后执行，首先要获得锁，多线程下保证安全
                unique_lock<mutex> locker(pool->mtx);
                while (true) {
                    if (!pool->tasks.empty()) {
                        auto task = std::move(pool->tasks.front());
                        pool->tasks.pop();
                        locker.unlock();
                        task();
                        locker.lock();
                    } else if (pool->isClosed) {
                        break;
                    } else {
                        // 如果池里没有任务且没有被关闭，则等待有新任务加入
                        pool->cond.wait(locker);
                    }
                }
            }).detach();
        }
    }

    threadPool() = default;

    threadPool(threadPool&& threadpool) = default;

    ~threadPool()
    {
        if (static_cast<bool>(_pool)) { // pool申请过且存在
            {
                lock_guard<mutex> locker(_pool->mtx);
                _pool->isClosed = true;
            }
            _pool->cond.notify_all();
        }
    }

    template <class F> void addTask(F&& task)
    {
        {
            lock_guard<mutex> locker(_pool->mtx);
            _pool->tasks.emplace(std::forward<F>(task));
        }
        _pool->cond.notify_one();
    }

private:
    struct pool {
        mutex mtx;
        condition_variable cond;
        bool isClosed;
        queue<function<void()> > tasks;
    };
    shared_ptr<pool> _pool;
};

#endif