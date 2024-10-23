/*
 * @Date: 2024-10-23 08:51:26
 * @LastEditors: lyyzzz && lyyzzz@yu0.me
 * @LastEditTime: 2024-10-23 09:11:24
 * @FilePath: /myWeb/test/epollertest.cpp
 */
#include "epoller.hpp"
#include <arpa/inet.h>
#include <cstddef>
#include <ctype.h>
#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
using namespace std;
int main()
{
    // 1.创建socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    // 2.绑定
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(5999);
    addr.sin_family = AF_INET;
    int ret = bind(listenfd, (struct sockaddr*)&addr, sizeof(addr));
    // 3.监听
    ret = listen(listenfd, 1024);
    // 4.注册epoll
    epoller ep;
    ep.addFd(listenfd, EPOLLIN);
    while (true) {
        int num = ep.wait();
        for (int i = 0; i < num; i++) {
            if (ep.getEventFd(i) == listenfd) {
                // 如果是监听的，则将新的连接注册到epoll中
                int fd = accept(listenfd, NULL, NULL);
                ep.addFd(fd, EPOLLIN);
            } else {
                // 非监听的发送的消息按照原文返回
                char buffer[1024] = {0};
                int s = read(ep.getEventFd(i), buffer, 1024);
                cout << "receive message:" << buffer << endl;
                const char* buff = "i have succeess recv your message!";
                ret = write(ep.getEventFd(i), buff, strlen(buff));
            }
        }
    }
}