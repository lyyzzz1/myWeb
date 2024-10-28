/*
 * @Date: 2024-10-23 08:52:10
 * @LastEditors: lyyzzz && lyyzzz@yu0.me
 * @LastEditTime: 2024-10-28 08:02:47
 * @FilePath: /myWeb/src/main.cpp
 */
#include "webserver.hpp"
#include <iostream>

using namespace std;

int main()
{
    WebServer server(5999, 3, 60000, false, 3306, "web", "123456", "web", 12,
                     200, true, 1, 1024);
    server.Start();
}