aux_source_directory(. SRC_LIST)

add_executable(webserver ${SRC_LIST})

target_link_libraries(webserver Threads::Threads)