# 设置源文件
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(BUFFER_SRC ${SRC_DIR}/buffer/buffer.cpp)
set(LOG_SRC ${SRC_DIR}/log/log.cpp)
set(TIMER_SRC ${SRC_DIR}/timer/timer.cpp)
set(SQLCONNPOOL_SRC ${SRC_DIR}/pool/sqlconnpool.cpp)
set(EPOLLER_SRC ${SRC_DIR}/epoller/epoller.cpp)
set(HTTPREQUEST_SRC ${SRC_DIR}/http/httprequest.cpp)
set(HTTPRESPONSE_SRC ${SRC_DIR}/http/httpresponse.cpp)
set(HTTPCONN_SRC ${SRC_DIR}/http/httpconn.cpp)
set(WEBSERVER_SRC ${SRC_DIR}/server/webserver.cpp)

# 设置输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

add_executable(webserver main.cpp ${BUFFER_SRC} ${LOG_SRC} ${TIMER_SRC} ${SQLCONNPOOL_SRC} ${EPOLLER_SRC} ${HTTPREQUEST_SRC} ${HTTPRESPONSE_SRC} ${HTTPCONN_SRC} ${WEBSERVER_SRC})

target_link_libraries(webserver Threads::Threads mysqlclient)
