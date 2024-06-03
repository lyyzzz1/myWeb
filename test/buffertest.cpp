#include "buffer.hpp"
#include <iostream>
#include <cassert>
#include <cstring>
#include <string>

void testBuffer() {
    Buffer buffer(1024);

    // Test initial state
    assert(buffer.writableBytes() == 1024);
    assert(buffer.readableBytes() == 0);
    assert(buffer.prependableBytes() == 0);

    // Test append and retrieve
    std::string data = "Hello, Buffer!";
    buffer.append(data);
    assert(buffer.readableBytes() == data.size());
    assert(buffer.writableBytes() == 1024 - data.size());

    std::string retrieved = buffer.retrieveAllToStr();
    assert(retrieved == data);
    assert(buffer.readableBytes() == 0);
    cout << buffer.writableBytes() << endl;
    assert(buffer.writableBytes() == 1024);

    // Test ensureWriteable and hasWritten
    buffer.ensureWriteable(512);
    std::string newData = "New data test";
    buffer.append(newData);
    assert(buffer.readableBytes() == newData.size());
    assert(buffer.writableBytes() == 1024 - newData.size());

    buffer.retrieve(newData.size());
    assert(buffer.readableBytes() == 0);
    assert(buffer.writableBytes() == 1024 - newData.size());

    // Test readFd and writeFd with a pipe
    int pipefd[2];
    int result = pipe(pipefd);
    assert(result == 0);

    std::string pipeData = "Data through pipe";
    write(pipefd[1], pipeData.c_str(), pipeData.size());

    int saveErrno;
    buffer.readFd(pipefd[0], &saveErrno);
    assert(buffer.readableBytes() == pipeData.size());
    assert(buffer.writableBytes() == 1024 - pipeData.size() - newData.size());

    char output[1024];
    ssize_t writeLen = buffer.writeFd(pipefd[1], &saveErrno);
    read(pipefd[0], output, writeLen);
    output[writeLen] = '\0';

    assert(std::string(output) == pipeData);
    assert(buffer.readableBytes() == 0);

    close(pipefd[0]);
    close(pipefd[1]);

    std::cout << "All tests passed!" << std::endl;
}

int main() {
    testBuffer();
    return 0;
}
