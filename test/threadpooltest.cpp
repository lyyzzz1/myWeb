#include "pool/threadpool.hpp"
#include <iostream>
#include <chrono>

void simpleTask(int id) {
    std::cout << "Task " << id << " is starting." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Task " << id << " is completed." << std::endl;
}

int main() {
    // 创建一个包含4个线程的线程池
    threadPool pool(4);

    // 添加10个任务到线程池
    for (int i = 0; i < 10; ++i) {
        pool.addTask([i]() { simpleTask(i); });
    }

    // 等待所有任务完成
    std::this_thread::sleep_for(std::chrono::seconds(12));

    std::cout << "All tasks have been completed." << std::endl;

    return 0;
}
