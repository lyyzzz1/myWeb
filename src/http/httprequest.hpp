#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "buffer.hpp"
#include <errno.h>
#include <mysql/mysql.h> //mysql
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>
using namespace std;

class httpRequest {
public:
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,
    };

    enum HTTP_CODE {
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };

    httpRequest()
    {
        init();
    }
    ~httpRequest() = default;

    void init();
    bool parse(Buffer& buff);

    string path() const;
    string& path();
    string mothod() const;
    string version() const;
    string getPost(const string& key) const;
    string getPost(const char* key) const;

    bool isKeepAlive() const;

private:
    bool _parseRequestLine(const string& line);
    void _parseHeader(const string& line);
    void _parseBody(const string& line);

    string _decoded(string& str);

    void _parsePath();
    void _parsePost();
    // 将POST的body如username=John+Doe&password=pass%40word&remember_me=true解析成正确格式
    void _parseFromUrlencoded();

    static bool userVerify(const string& name, const string& pwd, bool islogin);

    PARSE_STATE _state;
    string _method, _path, _version, _body;
    // 存储请求头信息，类似于 key:value
    unordered_map<string, string> _header;
    unordered_map<string, string> _post;

    static const unordered_set<string> DEFAULT_HTML;
    static const unordered_map<string, int> DEFAULT_HTML_TAG;
    static int converHex(char ch);
};

#endif