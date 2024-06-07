#ifndef EPOLLER_H
#define EPOLLER_H

#include <cstdint>
#include <sys/epoll.h>
#include <fcntl.h>
#include <vector>
using namespace std;

class epoller{
public:
    explicit epoller(int maxEvent = 1024);
    ~epoller();
    bool addFd(int fd, uint32_t events);
    bool modFd(int fd, uint32_t events);
    bool delFd(int fd);
    int wait(int timeoutMs = -1);
    int getEventFd(size_t i) const;
    uint32_t getEvents(size_t i) const;
private:
    int _epollfd;
    vector<struct epoll_event> _events;
};

#endif