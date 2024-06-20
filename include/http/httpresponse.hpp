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

public:
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

    char* _mmFile;
    struct stat _mmState;

    const static unordered_map<string, string> SUFFIX_TYPE;
    const static unordered_map<int, string> CODE_STATUS;
    const static unordered_map<int, string> CODE_HTML;
};

#endif