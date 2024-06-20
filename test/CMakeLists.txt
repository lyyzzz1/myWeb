set(TEST_EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${TEST_EXECUTABLE_OUTPUT_PATH})

find_package(Threads REQUIRED)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(BUFFER_DIR ${SRC_DIR}/buffer.cpp)
set(LOG_DIR ${SRC_DIR}/log.cpp)
set(TIMER_DIR ${SRC_DIR}/timer.cpp)
set(POOL_DIR ${PROJECT_SOURCE_DIR}/include/pool)
set(THREADPOOL_DIR ${POOL_DIR}/threadpool.hpp)
set(SQLCONNPOOL_DIR ${SRC_DIR}/sqlconnpool.cpp)
set(SQLCONNRAII_DIR ${POOL_DIR}/sqlconnRAII.hpp)
set(EPOLLER_DIR ${SRC_DIR}/server/epoller.cpp)

add_executable(buffertest buffertest.cpp ${BUFFER_DIR})
add_executable(logtest logtest.cpp ${LOG_DIR} ${BUFFER_DIR})
add_executable(timertest timertest.cpp ${TIMER_DIR} ${LOG_DIR} ${BUFFER_DIR})
add_executable(threadpooltest threadpooltest.cpp ${THREADPOOL_DIR})
add_executable(sqlconntest sqlconntest.cpp ${SQLCONNRAII_DIR} ${SQLCONNPOOL_DIR} ${LOG_DIR} ${BUFFER_DIR})
add_executable(epollertest epollertest.cpp ${EPOLLER_DIR})

target_link_libraries(logtest Threads::Threads)
target_link_libraries(timertest Threads::Threads)
target_link_libraries(threadpooltest Threads::Threads)
target_link_libraries(sqlconntest Threads::Threads mysqlclient)

target_include_directories(buffertest PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(logtest PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(logtest PUBLIC ${PROJECT_SOURCE_DIR}/include/log)
target_include_directories(sqlconntest PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_subdirectory(http)