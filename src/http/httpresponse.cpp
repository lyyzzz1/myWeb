#include "http/httpresponse.hpp"
#include "log.hpp"
#include <fcntl.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

const unordered_map<string, string> httpResponse::SUFFIX_TYPE = {
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/nsword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css "},
    {".js", "text/javascript "},
};

const unordered_map<int, string> httpResponse::CODE_STATUS = {
    {200, "OK"},
    {400, "Bad Request"},
    {403, "Forbidden"},
    {404, "Not Found"},
};

const unordered_map<int, string> httpResponse::CODE_HTML = {
    {400, "/400.html"},
    {403, "/403.html"},
    {404, "/404.html"},
};

httpResponse::httpResponse()
{
    _code = -1;
    _sourceDir = _path = "";
    _iskeepalive = false;
    _mmFile = nullptr;
    _mmState = {0};
}

void httpResponse::init(string scourceDir, string filename, bool iskeepAlive,
                        int code)
{
    if (_mmFile) {
        unmapFile();
    }
    _sourceDir = scourceDir;
    _path = filename;
    _iskeepalive = iskeepAlive;
    _code = code;
}

char* httpResponse::file()
{
    return _mmFile;
}

size_t httpResponse::fileLen() const
{
    return _mmState.st_size;
}

void httpResponse::writeRespons(Buffer& buffer)
{
    // 根据请求情况来写入状态码
    if (stat((_sourceDir + _path).data(), &_mmState) < 0 ||
        S_ISDIR(_mmState.st_mode)) {
        _code = 404;
    } else if (!(_mmState.st_mode & S_IROTH)) {
        _code = 403;
    } else if (_code == -1) {
        _code = 200;
    }
    _errorHTML();
    _writeStatusLine(buffer);
    _writeResHeader(buffer);
    _writeResBody(buffer);
}

void httpResponse::_errorHTML()
{
    if (CODE_HTML.find(_code) != CODE_HTML.end()) {
        // 找到了，则更改文件路径
        _path = CODE_HTML.find(_code)->second;
        stat((_sourceDir + _path).data(), &_mmState);
    }
}

void httpResponse::_writeStatusLine(Buffer& buffer)
{
    // 填写状态行 格式 类似于 HTTP/1.1 200 OK
    string status = "";
    status += ("HTTP/1.1 " + to_string(_code) + " " +
               CODE_STATUS.find(_code)->second) +
              "\r\n";
    buffer.append(status);
}

void httpResponse::_writeResHeader(Buffer& buffer)
{
    buffer.append("Connection: ");
    if (_iskeepalive) {
        buffer.append("keep-alive\r\n");
        buffer.append("keep-alive: max=6, timeout=120\r\n");
    } else {
        buffer.append("close\r\n");
    }
    buffer.append("Content-type: " + _getContentType() + "\r\n");
}

void httpResponse::_writeResBody(Buffer& buffer)
{
    int srcFd = open((_sourceDir + _path).data(), O_RDONLY);
    if (srcFd < 0) {
        errorContent(buffer, "File not found!");
        return;
    }

    LOG_DEBUG("file path:%s", (_sourceDir + _path).c_str());
    int* mmRet =
        (int*)mmap(0, _mmState.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
    if (*mmRet == -1) {
        errorContent(buffer, "File not found!");
        return;
    }
    _mmFile = (char*)mmRet;
    close(srcFd);
    buffer.append("Content-length: " + to_string(_mmState.st_size) +
                  "\r\n\r\n");
}

string httpResponse::_getContentType()
{
    // 通过请求的路径来判断类型
    int location = _path.find_last_of(".");
    string suffix = _path.substr(location);
    if (SUFFIX_TYPE.find(suffix) != SUFFIX_TYPE.end()) {
        // 找到了，正确返回
        return SUFFIX_TYPE.find(suffix)->second;
    } else {
        return "application/octet-stream";
    }
}

void httpResponse::unmapFile()
{
    if (_mmFile) {
        munmap(_mmFile, _mmState.st_size);
        _mmFile = nullptr;
        _mmState = {0};
    }
}

void httpResponse::errorContent(Buffer& buff, string message)
{
    string body;
    string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if (CODE_STATUS.count(_code) == 1) {
        status = CODE_STATUS.find(_code)->second;
    } else {
        status = "Bad Request";
    }
    body += to_string(_code) + " : " + status + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";

    buff.append("Content-length: " + to_string(body.size()) + "\r\n\r\n");
    buff.append(body);
}
