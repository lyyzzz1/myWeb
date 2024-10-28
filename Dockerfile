# 第一阶段：编译
FROM gcc:12.2 as builder

WORKDIR /app

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
FROM alpine:3.19

WORKDIR /app

# 安装运行时依赖
RUN apk add --no-cache \
    mariadb-connector-c \
    openssl \
    libstdc++

# 从builder阶段复制编译好的二进制文件和必要的源文件
COPY --from=builder /app/webserver /app/
COPY ./source/ /app/source

# 运行
CMD ["./webserver"]
