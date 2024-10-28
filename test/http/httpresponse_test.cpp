/*
 * @Date: 2024-10-23 08:51:26
 * @LastEditors: lyyzzz && lyyzzz@yu0.me
 * @LastEditTime: 2024-10-24 06:40:42
 * @FilePath: /myWeb/test/http/httpresponse_test.cpp
 */
// #include "http/httpresponse.hpp"
// #include <gtest/gtest.h>

// TEST(HttpResponseTest, InitTest) {
//     httpResponse response;
//     response.init("/home/lyy/project/myWeb/scource", "/index.html", true, 200);
//     EXPECT_EQ(response._sourceDir, "/home/lyy/project/myWeb/scource");
//     EXPECT_EQ(response._path, "/index.html");
//     EXPECT_TRUE(response._iskeepalive);
//     EXPECT_EQ(response._code, 200);
//     EXPECT_EQ(response._mmFile, nullptr);
// }

// TEST(HttpResponseTest, FileTest) {
//     httpResponse response;
//     Log::getInstance()->init(0);
//     response.init("/home/lyy/project/myWeb/scource", "/index.html", true, 200);
//     Buffer buffer;
//     response._writeResBody(buffer);
//     EXPECT_NE(response.file(), nullptr);
//     response.unmapFile();
//     EXPECT_EQ(response.file(), nullptr);
// }

// TEST(HttpResponseTest, FileLenTest) {
//     httpResponse response;
//     response.init("/home/lyy/project/myWeb/scource", "/index.html", true, 200);
//     Buffer buffer;
//     response._writeResBody(buffer);
//     EXPECT_GT(response.fileLen(), 0);
//     response.unmapFile();
//     EXPECT_EQ(response.fileLen(), 0);
// }

// TEST(HttpResponseTest, GetContentTypeTest) {
//     Log::getInstance()->init(0);
//     httpResponse response;
//     response._path = "/index.html";
//     EXPECT_EQ(response._getContentType(), "text/html");
//     response._path = "/styles.css";
//     EXPECT_EQ(response._getContentType(), "text/css ");
//     response._path = "/unknown.ext";
//     EXPECT_EQ(response._getContentType(), "application/octet-stream");
// }

// TEST(HttpResponseTest, ErrorContentTest) {
//     httpResponse response;
//     Buffer buffer;
//     response.errorContent(buffer, "Test error message");
//     std::string expected = "Content-length: 135\r\n\r\n<html><title>Error</title><body bgcolor=\"ffffff\">0 : Bad Request\n<p>Test error message</p><hr><em>TinyWebServer</em></body></html>";
//     EXPECT_EQ(buffer.retrieveAllToStr(), expected);
// }
