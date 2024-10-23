cmake_minimum_required(VERSION 3.8)

project(webserver)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# 设置输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 查找必要的包
find_package(Threads REQUIRED)

# 设置头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/log)
include_directories(${PROJECT_SOURCE_DIR}/src/pool)
include_directories(${PROJECT_SOURCE_DIR}/src/http)
include_directories(${PROJECT_SOURCE_DIR}/src/server)
include_directories(${PROJECT_SOURCE_DIR}/src/timer)
include_directories(${PROJECT_SOURCE_DIR}/src/buffer)
include_directories(${PROJECT_SOURCE_DIR}/src/epoller)

# 设置源文件目录
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)

# 添加子目录
add_subdirectory(src)
add_subdirectory(test)

# 设置生成clangd所需的compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)