/*
 * @Date: 2024-10-23 08:51:26
 * @LastEditors: lyyzzz && lyyzzz@yu0.me
 * @LastEditTime: 2024-10-23 10:28:09
 * @FilePath: /myWeb/src/http/httpresponse.hpp
 */
/*
与request类配合工作，通过其来获取方法并写入回复报文
*/
#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <fcntl.h>
#include <unordered_map>

#include "buffer.hpp"
#include "httprequest.hpp"
#include "log.hpp"

class httpResponse {
public:
    httpResponse();
    ~httpResponse() = default;

    void writeRespons(Buffer& buffer);

    void init(string scourceDir, string filename, bool iskeepAlive = false,
              int code = -1);

    void unmapFile();
    void errorContent(Buffer& buffer,string message);
    char* file();
    size_t fileLen() const;

private:
    void _writeStatusLine(Buffer& buffer);
    void _writeResHeader(Buffer& buffer);
    void _writeResBody(Buffer& buffer);
    void _errorHTML();
    string _getContentType();
    // 记录状态码
    int _code;
    // 路径前缀
    string _sourceDir;
    // 请求的文件
    string _path;

    bool _iskeepalive;

    // 指向内存映射文件的指针，用于高效读取文件内容
    char* _mmFile;        
    // 存储被映射文件的状态信息（大小、权限等）
    struct stat _mmState;  
    const static unordered_map<string, string> SUFFIX_TYPE;
    const static unordered_map<int, string> CODE_STATUS;
    const static unordered_map<int, string> CODE_HTML;
};

#endif
