/*
 * @Date: 2024-10-23 08:52:16
 * @LastEditors: lyyzzz && lyyzzz@yu0.me
 * @LastEditTime: 2024-10-23 09:25:58
 * @FilePath: /myWeb/src/epoller/epoller.cpp
 */
#include "epoller.hpp"
#include <cassert>
#include <sys/epoll.h>
#include <unistd.h>

/**
 * @description: 构造函数
 * @param {int} maxEvent
 * @return {*}
 */
epoller::epoller(int maxEvent) : _events(maxEvent)
{
    // 构造函数，创建epollfd
    _epollfd = epoll_create(1024);
}

epoller::~epoller()
{
    close(_epollfd);
}

/**
 * @description: 将指定的fd添加到epoll监听中，并通过ev指定监听事件
 * @param {int} fd
 * @param {uint32_t} events
 * @return {*}
 */
bool epoller::addFd(int fd, uint32_t events)
{
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    int ret = epoll_ctl(_epollfd, EPOLL_CTL_ADD, fd, &ev);
    return ret == 0;
}

/**
 * @description: 修改epoll中已注册的文件描述符的监听事件
 * @param {int} fd 要修改的文件描述符
 * @param {uint32_t} events 新的监听事件类型（如EPOLLIN、EPOLLOUT等）
 * @return {bool} 修改是否成功
 * 
 * MOD(modify)操作用于更新已经在epoll实例中注册的文件描述符的事件设置
 * 常见使用场景：当需要改变某个连接的监听状态时，比如从"仅读"改为"读写"
 */
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
