cmake_minimum_required(VERSION 3.0)

project(webserver)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)
set(CMAKE_CXX_STANDARD 17)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 配置头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)

# 设置生成clangd所需的comlile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 加载子目录
add_subdirectory(src)
add_subdirectory(test)