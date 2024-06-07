#include "server/epoller.hpp"
#include <cassert>
#include <sys/epoll.h>
#include <unistd.h>

epoller::epoller(int maxEvent) : _events(maxEvent)
{
    // 构造函数，创建epollfd
    _epollfd = epoll_create(1024);
}

epoller::~epoller()
{
    close(_epollfd);
}

bool epoller::addFd(int fd, uint32_t events)
{
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    int ret = epoll_ctl(_epollfd, EPOLL_CTL_ADD, fd, &ev);
    return ret == 0;
}

bool epoller::modFd(int fd, uint32_t events)
{
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    int ret = epoll_ctl(_epollfd, EPOLL_CTL_MOD, fd, &ev);
    return ret == 0;
}

bool epoller::delFd(int fd)
{
    epoll_event ev = {0};
    int ret = epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, &ev);
    return ret == 0;
}

int epoller::wait(int timeoutMs)
{
    return epoll_wait(_epollfd, &_events[0], _events.size(), timeoutMs);
}

int epoller::getEventFd(size_t i) const
{
    assert(i >= 0 && i < _events.size());
    return _events[i].data.fd;
}

uint32_t epoller::getEvents(size_t i) const
{
    assert(i >= 0 && i < _events.size());
    return _events[i].events;
}