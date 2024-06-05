#include "log/log.hpp"

int main()
{
    Log::getInstance()->init(0);
    thread t1([]() {
        for (int i = 0; i < 30000; i++) {
            LOG_DEBUG("我是debugTest:%d", i);
        }
    });
    thread t2([]() {
        for (int i = 0; i < 30000; i++) {
            LOG_INFO("我是infoTest:%d", i);
        }
    });
    t1.join();
    t2.join();
}