# 第一阶段：编译
FROM gcc:12.2 as builder

WORKDIR /app

# 设置使用清华源
RUN sed -i 's@/deb.debian.org/@/mirrors.tuna.tsinghua.edu.cn/@g' /etc/apt/sources.list && \
    sed -i 's@/security.debian.org/@/mirrors.tuna.tsinghua.edu.cn/@g' /etc/apt/sources.list

# 安装编译依赖
RUN apt-get update && apt-get install -y \
    cmake \
    libmariadb-dev \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

# 复制源代码
COPY ./src/ /app/src
COPY ./CMakeLists.txt /app
COPY ./source/ /app/source

# 编译
RUN mkdir build && \
    cd build && \
    cmake .. && \
    make

# 第二阶段：运行
FROM ubuntu:22.04

WORKDIR /app

# 设置清华源
RUN sed -i 's@/archive.ubuntu.com/@/mirrors.tuna.tsinghua.edu.cn/@g' /etc/apt/sources.list && \
    sed -i 's@/security.ubuntu.com/@/mirrors.tuna.tsinghua.edu.cn/@g' /etc/apt/sources.list

# 安装libstdc++6和必要的库
RUN apt-get update && \
    apt-get install -y libstdc++6 libmysqlclient-dev libmariadb3

# 从builder阶段复制编译好的二进制文件和必要的源文件
COPY --from=builder /app/webserver /app
COPY ./source/ /app/source

# 运行
CMD ["./webserver"]
