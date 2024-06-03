set(TEST_EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${TEST_EXECUTABLE_OUTPUT_PATH})

add_subdirectory(logtest)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(BUFFER_DIR ${SRC_DIR}/buffer.cpp)

add_executable(buffertest buffertest.cpp ${BUFFER_DIR})

target_include_directories(buffertest PUBLIC ${PROJECT_SOURCE_DIR}/include)