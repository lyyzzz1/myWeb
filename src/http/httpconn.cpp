/*
 * @Date: 2024-10-23 09:49:29
 * @LastEditors: lyyzzz && lyyzzz@yu0.me
 * @LastEditTime: 2024-10-28 07:57:48
 * @FilePath: /myWeb/src/http/httpconn.cpp
 */
#include "httpconn.hpp"
#include <arpa/inet.h>
#include <sys/mman.h>

const char* HttpConn::srcDir;
std::atomic<int> HttpConn::userCount;
bool HttpConn::isET;

HttpConn::HttpConn()
{
    fd_ = -1;
    addr_ = {0};
    isClose_ = true;
}

HttpConn::~HttpConn()
{
    Close();
}

void HttpConn::init(int fd, const sockaddr_in& addr)
{
    assert(fd > 0);
    userCount++;
    // 初始化连接
    fd_ = fd;
    addr_ = addr;
    writeBuff_.retrieveAll();
    readBuff_.retrieveAll();
    isClose_ = false;
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd_, getIP(), getPort(),
             (int)userCount);
}

ssize_t HttpConn::read(int* saveErrno)
{
    // 如果是ET模式需要一次性读完
    int len = -1;
    do {
        len = readBuff_.readFd(fd_, saveErrno);
        if (len <= 0) {
            break;
        }
    } while (isET);
    return len;
}

ssize_t HttpConn::write(int* saveErrno)
{
    ssize_t len = -1;
    do {
        // LOG_INFO("now write filename:%s", request_.path().c_str());
        // LOG_INFO("iov[0] len:%d,iov[1] len:%d", iov_[0].iov_len,
        //          iov_[1].iov_len);
        len = writev(fd_, iov_, iov_count_);
        if (len <= 0) {
            *saveErrno = errno;
            break;
        }
        if (iov_[0].iov_len + iov_[1].iov_len == 0) {
            break;
        } /* 传输结束 */
        else if (static_cast<size_t>(len) > iov_[0].iov_len) {
            iov_[1].iov_base =
                (uint8_t*)iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if (iov_[0].iov_len) {
                writeBuff_.retrieveAll();
                iov_[0].iov_len = 0;
            }
        } else {
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len;
            iov_[0].iov_len -= len;
            writeBuff_.retrieve(len);
        }
    } while (isET || ToWriteBytes() > 10240);
    return len;
}

void HttpConn::Close()
{
    // 取消共享内存
    response_.unmapFile();
    if (isClose_ == false) {
        isClose_ = true;
        userCount--;
        close(fd_);
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", fd_, getIP(),
                 getPort(), (int)userCount);
    }
}

int HttpConn::getFd() const
{
    return fd_;
}

const char* HttpConn::getIP() const
{
    return inet_ntoa(addr_.sin_addr);
}

sockaddr_in HttpConn::getAddr() const
{
    return addr_;
}

int HttpConn::getPort() const
{
    return addr_.sin_port;
}

bool HttpConn::process()
{
    request_.init();
    if (readBuff_.readableBytes() <= 0) {
        return false;
    } else if (request_.parse(readBuff_)) {
        // 成功解析
        response_.init(srcDir, request_.path(), request_.isKeepAlive(), 200);
    } else {
        // 解析失败
        response_.init(srcDir, request_.path(), false, 400);
    }

    response_.writeRespons(writeBuff_);

    /*响应头*/
    iov_[0].iov_base = const_cast<char*>(writeBuff_.peek());
    iov_[0].iov_len = writeBuff_.readableBytes();
    iov_count_ = 1;

    /*响应体*/
    if (response_.file() && response_.fileLen() > 0) {
        iov_[1].iov_base = response_.file();
        iov_[1].iov_len = response_.fileLen();
        iov_count_ = 2;
    }
    LOG_DEBUG("filesize:%d, %d  to %d", response_.fileLen(), iov_count_,
              ToWriteBytes());
    return true;
}

int HttpConn::ToWriteBytes()
{
    return iov_[0].iov_len + iov_[1].iov_len;
}
