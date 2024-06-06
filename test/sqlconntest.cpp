#include "log.hpp"
#include "pool/sqlconnRAII.hpp"
#include "pool/sqlconnpool.hpp"
#include <iostream>
#include <mysql/mysql.h>
#include <thread>

int main()
{
    Log::getInstance()->init(0);
    sqlConnPool::getInstance()->init("127.0.0.1", 3306, "web", "123456", "web",
                                     20);
    std::vector<std::thread> threads;
    for (int i = 0; i < 30; i++) {
        threads.emplace_back([i]() {
            MYSQL* mysql;
            sqlConnRAII con(&mysql, sqlConnPool::getInstance());
            LOG_DEBUG("id:%d,has success get sqlconn!", i);
            if (!mysql) {
                std::cerr << "Failed to get mysql connection in thread " << i
                          << std::endl;
                return;
            }
            char sql[1024] = "select * from users";
            if (mysql_query(mysql, sql)) {
                std::cerr << "MySQL query error in thread " << i << ": "
                          << mysql_error(mysql) << std::endl;
                return;
            }

            MYSQL_RES* res = mysql_store_result(mysql);
            if (!res) {
                std::cerr << "Failed to store result in thread " << i << ": "
                          << mysql_error(mysql) << std::endl;
                return;
            }

            MYSQL_ROW row;
            int j = 0;
            while ((row = mysql_fetch_row(res))) {
                std::cout << "I'm thread " << i << "   ";
                std::cout << "Line " << j++ << "  username: " << row[0]
                          << " password: " << row[1] << std::endl;
            }

            mysql_free_result(res);
        });
    }

    for (auto& t : threads) {
        t.join();
    }
}
