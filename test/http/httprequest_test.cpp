#include "http/httprequest.hpp"
#include "log.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include "pool/sqlconnRAII.hpp"
#include "pool/sqlconnpool.hpp"

#define TEST_DEBUG 1

#IF TEST_DEBUG
TEST(HttpRequestTest, ParseRequestLine)
{
    Log::getInstance()->init(0);
    httpRequest req;
    string line = "GET /index HTTP/1.1";
    ASSERT_TRUE(req._parseRequestLine(line));
    ASSERT_EQ(req._method, "GET");
    ASSERT_EQ(req._path, "/index");
    ASSERT_EQ(req._version, "1.1");
}

TEST(HttpRequestTest, ParseHeader)
{
    Log::getInstance()->init(0);
    httpRequest req;
    string line = "Host: localhost:8080";
    req._parseHeader(line);
    ASSERT_EQ(req._header["Host"], "localhost:8080");
}

TEST(HttpRequestTest, ParseBody)
{
    Log::getInstance()->init(0);
    httpRequest req;
    string line = "username=test&password=pass";
    req._parseBody(line);
    ASSERT_EQ(req._body, line);
    std::cout << req._post["username"] << std::endl;
    std::cout << req._post["password"] << std::endl;
    ASSERT_EQ(req.getPost("username"), "test");
    ASSERT_EQ(req.getPost("password"), "pass");
}

TEST(HttpRequestTest, ParsePath)
{
    Log::getInstance()->init(0);
    httpRequest req;
    req._path = "/";
    req._parsePath();
    ASSERT_EQ(req._path, "index.html");

    req._path = "/register";
    req._parsePath();
    ASSERT_EQ(req._path, "/register.html");
}

TEST(HttpRequestTest, ParsePost)
{
    Log::getInstance()->init(0);
    httpRequest req;
    req._method = "POST";
    req._header["Content-Type"] = "application/x-www-form-urlencoded";
    req._body = "username=test&password=pass";
    req._parsePost();
    ASSERT_EQ(req.getPost("username"), "test");
    ASSERT_EQ(req.getPost("password"), "pass");
}

TEST(HttpRequestTest, IsKeepAlive)
{
    Log::getInstance()->init(0);
    httpRequest req;
    req._header["Connection"] = "keep-alive";
    req._version = "1.1";
    ASSERT_TRUE(req.isKeepAlive());

    req._header["Connection"] = "close";
    ASSERT_FALSE(req.isKeepAlive());
}

TEST(HttpRequestTest, UserVerify)
{
    Log::getInstance()->init(0);
    sqlConnPool::getInstance()->init("127.0.0.1", 3306, "web", "123456", "web",
                                     20);
    // Test login
    ASSERT_TRUE(httpRequest::userVerify("lyy", "lyy", true));
    ASSERT_FALSE(httpRequest::userVerify("invaliduser", "invalidpass", true));

    // Test registration
    ASSERT_TRUE(httpRequest::userVerify("newuser", "newpass", false));
    ASSERT_FALSE(httpRequest::userVerify("lyy", "lyyyy", false));
}

#ELSE
int main()
{
    Log::getInstance()->init(0);
    httpRequest req;
    req._method = "POST";
    req._header["Content-Type"] = "application/x-www-form-urlencoded";
    req._body = "username=test&password=pass";
    req._parsePost();
    std::cout << req.getPost("username") << std::endl;
    std::cout << req.getPost("password") << std::endl;
}