#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

/*
 * @Date: 2024-10-23 08:51:26
 * @LastEditors: lyyzzz && lyyzzz@yu0.me
 * @LastEditTime: 2024-10-25 02:04:40
 * @FilePath: /myWeb/src/server/webserver.hpp
 */

#include "epoller.hpp"
#include "httprequest.hpp"
#include "httpresponse.hpp"
#include "log.hpp"
#include "threadpool.hpp"
#include "sqlconnRAII.hpp"
#include "sqlconnpool.hpp"
#include "timer.hpp"
#include "httpconn.hpp"
class WebServer {
public:
    WebServer(
        int port, int trigMode, int timeoutMS, bool OptLinger, 
        int sqlPort, const char* sqlUser, const  char* sqlPwd, 
        const char* dbName, int connPoolNum, int threadNum,
        bool openLog, int logLevel, int logQueSize);

    ~WebServer();
    void Start();

private:
    bool InitSocket_(); 
    void InitEventMode_(int trigMode);
    void AddClient_(int fd, sockaddr_in addr);
  
    void DealListen_();
    void DealWrite_(HttpConn* client);
    void DealRead_(HttpConn* client);

    void SendError_(int fd, const char*info);
    void ExtentTime_(HttpConn* client);
    void CloseConn_(HttpConn* client);

    void OnRead_(HttpConn* client);
    void OnWrite_(HttpConn* client);
    void OnProcess(HttpConn* client);

    static const int MAX_FD = 65536;

    static int SetFdNonblock(int fd);

    int port_;
    bool openLinger_;
    int timeoutMS_;  /* 毫秒MS */
    bool isClose_;
    int listenFd_;
    char* srcDir_;
    
    uint32_t listenEvent_;
    uint32_t connEvent_;
   
    std::unique_ptr<heapTimer> timer_;
    std::unique_ptr<threadPool> threadpool_;
    std::unique_ptr<epoller> epoller_;
    std::unordered_map<int, HttpConn> users_;
};
#endif // __WEBSERVER_H__