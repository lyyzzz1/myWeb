#include "buffer.hpp"
#include <algorithm>
#include <cassert>

Buffer::Buffer(int initBufferSize)
    : _buffer(initBufferSize), _readPos(0), _writePos(0)
{
}

size_t Buffer::writableBytes() const
{
    return _buffer.size() - _writePos;
}

size_t Buffer::readableBytes() const
{
    return _writePos - _readPos;
}

size_t Buffer::prependableBytes() const
{
    return _readPos;
}

const char* Buffer::peek() const
{
    return _beginPtr() + _readPos;
}

void Buffer::ensureWriteable(size_t len)
{
    if (writableBytes() < len) {
        _makeSpace(len);
    }
    assert(len <= writableBytes());
}

void Buffer::hasWritten(size_t len)
{
    _writePos += len;
}

void Buffer::retrieve(size_t len)
{
    assert(len <= readableBytes());
    _readPos += len;
}

void Buffer::retrieveUntil(const char* end)
{
    assert(peek() <= end);
    retrieve(end - peek());
}

void Buffer::retrieveAll()
{
    bzero(&_buffer, _buffer.size());
    _readPos = 0;
    _writePos = 0;
}

string Buffer::retrieveAllToStr()
{
    string str(peek(), readableBytes());
    retrieveAll();
    return str;
}

const char* Buffer::beginWriteConst() const
{
    return _beginPtr() + _writePos;
}

char* Buffer::beginWrite()
{
    return _beginPtr() + _writePos;
}

void Buffer::append(const string& str)
{
    append(str.data(), str.size());
}

void Buffer::append(const char* str, size_t len)
{
    // 所有append方法的基础方法
    assert(str);
    ensureWriteable(len);
    copy(str, str + len, beginWrite());
    hasWritten(len);
}

void Buffer::append(const void* str, size_t len)
{
    append(static_cast<const char*>(str), len);
}

void Buffer::append(const Buffer& buffer)
{
    append(buffer.peek(), buffer.readableBytes());
}

ssize_t Buffer::readFd(int fd, int* errno)
{
    return 0;
}

ssize_t Buffer::writeFd(int fd, int* errno)
{
    return 0;
}

char* Buffer::_beginPtr()
{
    // 对迭代器解引用得到的是第一个元素本身，再取地址就变为了char*
    return &*_buffer.begin();
}

const char* Buffer::_beginPtr() const
{
    return &*_buffer.begin();
}

void Buffer::_makeSpace(size_t len)
{
    if (writableBytes() + prependableBytes() < len) {
        _buffer.resize(_writePos + len + 1);
    } else {
        // 将目前的有效部分移动到最前方
        size_t readable = readableBytes();
        copy(_beginPtr() + _readPos, _beginPtr() + _writePos, _beginPtr());
        _readPos = 0;
        _writePos = _readPos + readable;
        assert(readable == readableBytes());
    }
}