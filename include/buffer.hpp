/*一个缓冲区(Buffer)类，主要用于网络编程中数据的缓存处理。*/
#ifndef BUFFER_H
#define BUFFER_H

#include <atomic>
#include <cstddef>
#include <string>
#include <sys/types.h>
#include <vector>
#include <strings.h>
using namespace std;
class Buffer {
public:
    Buffer(int initBufferSize = 1024);
    ~Buffer() = default;
    // 返回缓冲区中可以写入的字节数
    size_t writableBytes() const;
    // 返回缓冲区中可以读取的字节数
    size_t readableBytes() const;
    // 返回缓冲区前面部分的空闲字节数
    size_t prependableBytes() const;
    // 返回读取位置
    const char* peek() const;
    // 确认可写入，如果可写入空间不足则调用_makeSpace函数调整容量
    void ensureWriteable(size_t len);
    void hasWritten(size_t len);

    // 移动读取位置，跳过指定长度
    void retrieve(size_t len);
    // 通过传入的末尾指针跳过指定长度
    void retrieveUntil(const char* end);
    // 清空缓冲区
    void retrieveAll();     
    // 将缓冲区内的数据输出为string然后清空
    string retrieveAllToStr(); 

    // 返回可写位置的const指针
    const char* beginWriteConst() const;
    // 返回可写位置的指针
    char* beginWrite();

    // 向缓冲区中追加字符或者字符串
    void append(const string& str);
    void append(const char* str,size_t len);
    void append(const void* str,size_t len);
    void append(const Buffer& buffer);

    ssize_t readFd(int fd,int* errno);
    ssize_t writeFd(int fd,int* errno);
private:
    char* _beginPtr(); //辅助函数，返回buffer起始位置
    const char* _beginPtr() const;
    void _makeSpace(size_t len); // 确保缓冲区足够读写，否则就扩容

    vector<char> _buffer;     // 维护的缓冲区
    atomic<size_t> _readPos;  // 读取位置
    atomic<size_t> _writePos; // 写入位置
};

#endif