#include "http/httprequest.hpp"
#include "log.hpp"
#include "pool/sqlconnRAII.hpp"
#include "pool/sqlconnpool.hpp"
#include <algorithm>
#include <cstdio>
#include <mysql/mysql.h>
#include <regex>
#include <utility>

const unordered_set<string> httpRequest::DEFAULT_HTML{
    "/index", "/register", "/login", "/welcome", "/video", "/picture",
};

const unordered_map<string, int> httpRequest::DEFAULT_HTML_TAG{
    {"/register.html", 0},
    {"/login.html", 1},
};

void httpRequest::init()
{
    _method = _path = _body = _version = "";
    _state = REQUEST_LINE;
    _header.clear();
    _post.clear();
}

bool httpRequest::parse(Buffer& buff)
{
    const char CRLF[] = "\r\n";
    if (buff.readableBytes() < 0) {
        return false;
    }
    while (buff.readableBytes() && _state != FINISH) {
        // 每次取一行
        const char* lineEnd =
            search(buff.peek(), buff.beginWriteConst(), CRLF, CRLF + 2);
        string line(buff.peek(), lineEnd);
        switch (_state) {
            case REQUEST_LINE:
                if (!_parseRequestLine(line)) {
                    return false;
                }
                _parsePath();
                break;
            case HEADERS:
                _parseHeader(line);
                if (buff.readableBytes() <= 2) {
                    _state = FINISH;
                }
                break;
            case BODY:
                _parseBody(line);
                break;
            default:
                break;
        }
        if (lineEnd == buff.beginWriteConst()) {
            break;
        }
        buff.retrieveUntil(lineEnd + 2);
    }
    LOG_DEBUG("[%s], [%s], [%s]", _method.c_str(), _path.c_str(),
              _version.c_str());
    return true;
}

string httpRequest::path() const
{
    return _path;
}

string& httpRequest::path()
{
    return _path;
}

string httpRequest::mothod() const
{
    return _method;
}

string httpRequest::version() const
{
    return _version;
}

string httpRequest::getPost(const string& key) const
{
    assert(key != "");
    if (_post.count(key) == 1) {
        return _post.find(key)->second;
    }
    return "";
}

string httpRequest::getPost(const char* key) const
{
    assert(key != nullptr);
    if (_post.count(key) == 1) {
        return _post.find(key)->second;
    }
    return "";
}

bool httpRequest::isKeepAlive() const
{
    if (_header.count("Connection") == 1) {
        return _header.find("Connection")->second == "keep-alive" &&
               _version == "1.1";
    }
    return false;
}

bool httpRequest::_parseRequestLine(const string& line)
{
    regex reg("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    smatch result;
    if (regex_match(line, result, reg)) {
        _method = result[1];
        _path = result[2];
        _version = result[3];
        _state = HEADERS;
        return true;
    }
    LOG_ERROR("ParseRequestLine Error! Line is: %s", line.c_str());
    return false;
}

void httpRequest::_parseHeader(const string& line)
{
    regex reg("^([^:]*): ?(.*)$");
    smatch result;
    if (regex_match(line, result, reg)) {
        _header.insert(make_pair(result[1], result[2]));
    } else {
        _state = BODY;
    }
}

void httpRequest::_parseBody(const string& line)
{
    _body = line;
    _parsePost();
    _state = FINISH;
    LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
}

void httpRequest::_parsePath()
{
    if (_path == "/") {
        _path = "index.html";
    } else {
        for (auto& it : DEFAULT_HTML) {
            if (it == _path) {
                _path += ".html";
                break;
            }
        }
    }
}

void httpRequest::_parsePost()
{
    if (_method == "POST" &&
        _header["Content-Type"] == "application/x-www-form-urlencoded") {
        _parseFromUrlencoded();
        int tag;
        // 查看是注册还是登录
        if (DEFAULT_HTML_TAG.find(_path) != DEFAULT_HTML_TAG.end()) {
            tag = DEFAULT_HTML_TAG.find(_path)->second;
            bool isLogin = (tag == 1);
            if (userVerify(_post["username"], _post["password"], isLogin)) {
                _path = "/welcome.html";
            } else {
                _path = "/error.html";
            }
        }
    }
}

void httpRequest::_parseFromUrlencoded()
{
    if (_body.empty()) {
        return;
    }
    // 先分割，后解码
    int start = 0;
    string key, value;
    for (int i = 0; i < _body.size(); i++) {
        char ch = _body[i];
        if (ch == '=') {
            // 提取到Key
            key = _body.substr(start, i - start);
            start = i + 1;
        } else if (ch == '&') {
            // 提取到Value，对Value进行解码
            value = _body.substr(start, i - start);
            start = i + 1;
            _post[_decoded(key)] = _decoded(value);
            LOG_DEBUG("In _parseFromUrlencoded key:%s,value:%s",
                      _decoded(key).c_str(), _decoded(value).c_str());
        } else if (i == _body.size() - 1) {
            value = _body.substr(start, i + 1 - start);
            _post[_decoded(key)] = _decoded(value);
            LOG_DEBUG("In _parseFromUrlencoded key:%s,value:%s",
                      _decoded(key).c_str(), _decoded(value).c_str());
        }
    }
    string mkey, mvalue;
    for (auto p : _post) {
        mkey = p.first;
        mvalue = p.second;
        LOG_INFO("PrintPost key:%s,value:%s", mkey.c_str(), mvalue.c_str());
    }
}

string httpRequest::_decoded(string& str)
{
    std::string decoded = "";
    for (int i = 0; i < str.size(); i++) {
        char ch = str[i];
        if (ch == '+') {
            decoded.push_back(' ');
        } else if (ch == '%') {
            int num = converHex(str[i + 1]) * 16 + converHex(str[i + 2]);
            decoded.push_back(num);
            i += 2;
        } else {
            decoded.push_back(ch);
        }
    }
    return decoded;
}

bool httpRequest::userVerify(const string& name, const string& pwd,
                             bool isLogin)
{
    MYSQL* sql;
    sqlConnRAII(&sql, sqlConnPool::getInstance());
    char query[1024] = "";
    // TODO: 实现用户验证逻辑
    if (isLogin) {
        // 验证登录逻辑
        sprintf(query,
                "select username, password FROM users WHERE username = '%s'",
                name.c_str());
        if (mysql_query(sql, query)) {
            LOG_ERROR("Query Error: %s", mysql_error(sql));
            return false;
        }
        // 存储结果
        MYSQL_RES* res = mysql_store_result(sql);
        int num_rows = mysql_num_rows(res);
        if (num_rows == 0) {
            mysql_free_result(res);
            return false;
        } else {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row == nullptr) {
                mysql_free_result(res);
                return false;
            }
            string password = row[1];
            if (password == pwd) {
                return true;
            }
        }
    } else {
        // 注册，首先查询用户名是否存在
        sprintf(query, "select username FROM users WHERE username='%s'",
                name.c_str());
        if (mysql_query(sql, query)) {
            LOG_ERROR("Query Error: %s", mysql_error(sql));
            return false;
        }
        MYSQL_RES* res = mysql_store_result(sql);
        int rownum = mysql_num_rows(res);
        if (rownum != 0) {
            LOG_ERROR("The username:%s, is already used", name.c_str());
            mysql_free_result(res);
            return false;
        }
        sprintf(query, "INSERT INTO users(username,password) VALUES('%s','%s')",
                name.c_str(), pwd.c_str());
        if (mysql_query(sql, query)) {
            mysql_free_result(res);
            return false;
        }
        mysql_free_result(res);
        return true;
    }
    return false;
}

int httpRequest::converHex(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return 0; // 如果是非法字符，返回 0 或者抛出异常
}