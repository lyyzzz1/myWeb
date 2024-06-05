#include "timer.hpp"
#include <iostream>
#include <thread>

void testCallback() {
    std::cout << "Timer expired!" << std::endl;
}

int main() {
    heapTimer timer;
    
    // 添加定时器
    timer.add(1, 2000, testCallback);  // 2秒后触发
    timer.add(2, 3000, testCallback);  // 3秒后触发

    // 模拟时间流逝
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    
    // 处理到期的定时器
    timer.tick();  // 应该触发第一个定时器

    // 查看下一个即将到期的定时器
    int nextTick = timer.getNextTick();
    std::cout << "Next timer will expire in: " << nextTick << " ms" << std::endl;
    
    return 0;
}
