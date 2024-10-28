#ifndef __HTTPCONN_H__
#define __HTTPCONN_H__

/*
 * @Date: 2024-10-23 09:49:22
 * @LastEditors: lyyzzz && lyyzzz@yu0.me
 * @LastEditTime: 2024-10-25 06:14:52
 * @FilePath: /myWeb/src/http/httpconn.hpp
 */
#include "httprequest.hpp"
#include "httpresponse.hpp"
#include <netinet/in.h>

// 该类主要用于处理HTTP请求，对HTTP请求进行读取和回复
class HttpConn {
public:
    HttpConn();
    ~HttpConn();

    void init(int fd, const sockaddr_in& addr);

    ssize_t read(int* saveErrno);
    ssize_t write(int* saveEerrno);

    void Close();

    int getFd() const;

    const char* getIP() const;

    sockaddr_in getAddr() const;

    int getPort() const;

    bool process();

    int ToWriteBytes();

    bool isKeepAlive() const
    {
        return request_.isKeepAlive();
    }

    static bool isET;
    static const char* srcDir;
    static std::atomic<int> userCount;

private:
    // 文件描述符
    int fd_;
    struct sockaddr_in addr_;

    bool isClose_;
    int iov_count_;
    struct iovec iov_[2];

    Buffer readBuff_;
    Buffer writeBuff_;

    httpRequest request_;
    httpResponse response_;
};
#endif // __HTTPCONN_H__