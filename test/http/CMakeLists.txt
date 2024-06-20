set(TEST_EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/test/bin)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${TEST_EXECUTABLE_OUTPUT_PATH})

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(BUFFER_DIR ${SRC_DIR}/buffer.cpp)
set(LOG_DIR ${SRC_DIR}/log.cpp)
set(HTTP_DIR ${SRC_DIR}/http)

add_executable(httprequest_test httprequest_test.cpp ${HTTP_DIR}/httprequest.cpp ${SQLCONNRAII_DIR} ${SQLCONNPOOL_DIR} ${LOG_DIR} ${BUFFER_DIR})

target_link_libraries(httprequest_test gtest gtest_main Threads::Threads mysqlclient)