/*
 * @Date: 2024-10-23 09:08:09
 * @LastEditors: lyyzzz && lyyzzz@yu0.me
 * @LastEditTime: 2024-10-28 08:04:41
 * @FilePath: /myWeb/src/server/webserver.cpp
 */
#include "webserver.hpp"
#include "httpconn.hpp"

WebServer::WebServer(int port, int trigMode, int timeoutMS, bool OptLinger,
                     int sqlPort, const char* sqlUser, const char* sqlPwd,
                     const char* dbName, int connPoolNum, int threadNum,
                     bool openLog, int logLevel, int logQueSize)
    : port_(port), openLinger_(OptLinger), timeoutMS_(timeoutMS),
      isClose_(false), threadpool_(new threadPool(threadNum)),
      timer_(new heapTimer()), epoller_(new epoller())
{
    srcDir_ = getcwd(nullptr, 256);
    char newPath[256] = {0};
    snprintf(newPath, 256, "%s/source/", srcDir_);
    free(srcDir_);
    srcDir_ = strdup(newPath);
    HttpConn::srcDir = srcDir_;
    if (openLog) {
        Log::getInstance()->init(logLevel, "./log", ".log", logQueSize);
        if (isClose_) {
            LOG_ERROR("========== Server init error!==========");
        } else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", port_,
                     OptLinger ? "true" : "false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                     (listenEvent_ & EPOLLET ? "ET" : "LT"),
                     (connEvent_ & EPOLLET ? "ET" : "LT"));
            LOG_INFO("LogSys level: %d", logLevel);
            LOG_INFO("srcDir: %s", HttpConn::srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum,
                     threadNum);
        }
    }
    HttpConn::userCount = 0;
    InitEventMode_(trigMode);
    if (!InitSocket_()) {
        isClose_ = true;
    }
    sqlConnPool::getInstance()->init("120.46.62.33", sqlPort, sqlUser, sqlPwd,
                                     dbName, connPoolNum);
}

WebServer::~WebServer()
{
    close(listenFd_);
    isClose_ = true;
    free(srcDir_);
    sqlConnPool::getInstance()->closePool();
}

void WebServer::Start()
{
    int timeMS = -1;
    if (!isClose_) {
        LOG_INFO("===========server start!=============");
    }
    while (!isClose_) {
        if (timeoutMS_ > 0) {
            timeMS = timer_->getNextTick();
        }
        int eventCount = epoller_->wait(timeMS);
        LOG_DEBUG("Epoll wait return with %d events", eventCount); // 添加此行
        for (int i = 0; i < eventCount; i++) {
            LOG_DEBUG("Processing event: fd=%d, events=%u",
                      epoller_->getEventFd(i),
                      epoller_->getEvents(i)); // 添加此行
            int fd = epoller_->getEventFd(i);
            uint32_t events = epoller_->getEvents(i);
            if (fd == listenFd_) {
                // 创建新的连接
                DealListen_();
            } else if (events & (EPOLLRDHUP | EPOLLERR | EPOLLHUP)) {
                // 处理错误或关闭的连接
                CloseConn_(&users_[fd]);
            } else if (events & EPOLLIN) {
                // 处理读事件，即有数据发送过来
                DealRead_(&users_[fd]);
            } else if (events & EPOLLOUT) {
                // fd可写，将数据写入
                DealWrite_(&users_[fd]);
            } else {
                LOG_ERROR("Unexpected Event");
            }
        }
    }
}

void WebServer::InitEventMode_(int trigMode)
{
    // EPOLLONESHOT
    // 这是一个重要的事件标志，用于防止多个线程同时处理同一个socket连接
    // 当一个事件发生并被处理后，该fd会被自动从epoll监听集合中移除
    // 如果要继续监听这个fd，需要再次调用epoll_ctl添加
    // 这样可以保证一个socket连接在任意时刻都只被一个线程处理，避免多线程竞争问题

    // EPOLLRDHUP
    // 这个事件用于检测对端(客户端)断开连接的情况
    // 当客户端正常断开连接(调用close())时会触发此事件
    // 相比于传统的EPOLLIN+read==0的方式，这是一种更优雅的检测连接断开的方法
    listenEvent_ = EPOLLRDHUP;
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP;
    // 根据trigmode来确定监听事件

    switch (trigMode) {
        case 0:
            break;
        case 1:
            connEvent_ |= EPOLLET;
            break;
        case 2:
            listenEvent_ |= EPOLLET;
            break;
        case 3:
            listenEvent_ |= EPOLLET;
            connEvent_ |= EPOLLET;
            break;
        default:
            listenEvent_ |= EPOLLET;
            connEvent_ |= EPOLLET;
            break;
    }

    HttpConn::isET = (connEvent_ & EPOLLET);
}

void WebServer::AddClient_(int fd, sockaddr_in addr)
{
    // 添加客户端
    users_[fd].init(fd, addr);
    SetFdNonblock(fd);
    if (epoller_->addFd(fd, connEvent_ | EPOLLIN) == 0) { // 添加错误检查
        LOG_ERROR("Failed to add client fd to epoll");
    }
    // 加入定时器序列中
    if (timeoutMS_ > 0) {
        LOG_DEBUG("Adding client fd to timer");
        timer_->add(fd, timeoutMS_,
                    std::bind(&WebServer::CloseConn_, this, &users_[fd]));
    }
    LOG_INFO("Client[%d] in!", users_[fd].getFd());
}

void WebServer::DealListen_()
{
    LOG_DEBUG("DealListen_");
    // 有客户端尝试连接，接受连接，创建定时器，添加客户端
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listenFd_, (struct sockaddr*)&addr, &len);
        if (fd <= 0) {
            return;
        } else if (HttpConn::userCount >= MAX_FD) {
            SendError_(fd, "达到最大连接数，请稍后再试！");
            LOG_WARN("Connect full!");
            return;
        }
        AddClient_(fd, addr);
    } while (listenEvent_ & EPOLLET);
}

void WebServer::DealWrite_(HttpConn* client)
{
    LOG_DEBUG("DealWrite_");
    ExtentTime_(client);
    threadpool_->addTask(std::bind(&WebServer::OnWrite_, this, client));
}

void WebServer::DealRead_(HttpConn* client)
{
    LOG_DEBUG("DealRead_");
    ExtentTime_(client);
    threadpool_->addTask(std::bind(&WebServer::OnRead_, this, client));
}

void WebServer::SendError_(int fd, const char* info)
{
    // 发送error连接信息，并关闭连接
    int ret = send(fd, info, strlen(info), 0);
    if (ret < 0) {
        LOG_WARN("send error to client[%d] error!", fd);
    }
    close(fd);
}

void WebServer::ExtentTime_(HttpConn* client)
{
    if (timeoutMS_ > 0) {
        // 添加检查，确保定时器存在
        if (timer_->exist(client->getFd())) {
            timer_->adjust(client->getFd(), timeoutMS_);
        } else {
            // 如果定时器不存在，重新添加
            timer_->add(client->getFd(), timeoutMS_,
                std::bind(&WebServer::CloseConn_, this, client));
        }
    }
}

void WebServer::CloseConn_(HttpConn* client)
{
    // 关闭连接
    LOG_INFO("Client[%d] quit!", client->getFd());
    epoller_->delFd(client->getFd());
    // 删除定时器
    // timer_->del(client->getFd());
    client->Close();
}

void WebServer::OnRead_(HttpConn* client)
{
    LOG_DEBUG("OnRead_");
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno);
    if (ret <= 0 && readErrno != EAGAIN) {
        CloseConn_(client);
        return;
    }
    OnProcess(client);
}

void WebServer::OnWrite_(HttpConn* client)
{
    LOG_DEBUG("OnWrite_");
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);
    if(client->ToWriteBytes() == 0) {
        /* 传输完成 */
        if(client->isKeepAlive()) {
            OnProcess(client);
            return;
        }
    }
    else if(ret < 0) {
        if(writeErrno == EAGAIN) {
            /* 继续传输 */
            epoller_->modFd(client->getFd(), connEvent_ | EPOLLOUT);
            return;
        }
    }
    CloseConn_(client);
}

void WebServer::OnProcess(HttpConn* client)
{
    if (client->process()) {
        epoller_->modFd(client->getFd(), connEvent_ | EPOLLOUT);
    } else {
        epoller_->modFd(client->getFd(), connEvent_ | EPOLLIN);
    }
}

int WebServer::SetFdNonblock(int fd)
{
    // 将fd设置为非阻塞状态
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

// 初始化listenfd
bool WebServer::InitSocket_()
{
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd_ == -1) {
        LOG_ERROR("socket failed!");
        return false;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_port = htons(port_);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ntohl(INADDR_ANY);

    // 设置优雅的关闭连接
    struct linger optLinger = {0};
    if (openLinger_) {
        optLinger.l_linger = 1;
        optLinger.l_onoff = 1;
    }
    int ret;
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLinger,
                     sizeof(optLinger));
    if (ret < 0) {
        close(listenFd_);
        LOG_ERROR("Init linger error!", port_);
        return false;
    }

    // 设置端口复用
    int optval = 1;
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &optval,
                     sizeof(optval));

    if (ret == -1) {
        LOG_ERROR("set socket setsockopt error !");
        close(listenFd_);
        return false;
    }

    // 绑定地址
    ret = bind(listenFd_, (sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        LOG_ERROR("Bind Port:%d error!", port_);
        close(listenFd_);
        return false;
    }

    // 监听
    ret = listen(listenFd_, 6);
    if (ret < 0) {
        LOG_ERROR("Listen port:%d error!", port_);
        close(listenFd_);
        return false;
    }

    // 加入到epoll监听当中
    ret = epoller_->addFd(listenFd_, listenEvent_ | EPOLLIN);
    if (ret == 0) {
        LOG_ERROR("Add listen error!");
        close(listenFd_);
        return false;
    }
    SetFdNonblock(listenFd_);
    LOG_INFO("Server port:%d", port_);
    return true;
}
