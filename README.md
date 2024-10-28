# WebServer

## 项目结构

- src: 源码
- source: 静态文件

## 性能数据

### QPS
QPS = 39363 / 10 = 3936.3 请求/秒
```
➜  myWeb git:(main) ✗ ./WebBench/webbench -c 1000 -t 10 http://localhost:5999/index
Webbench - Simple Web Benchmark 1.5
Copyright (c) Radim Kolar 1997-2004, GPL Open Source Software.

Request:
GET /index HTTP/1.0
User-Agent: WebBench 1.5
Host: localhost


Runing info: 1000 clients, running 10 sec.

Speed=234762 pages/min, 3724795 bytes/sec.
Requests: 39127 susceed, 0 failed.
```

### todo:加入协程库