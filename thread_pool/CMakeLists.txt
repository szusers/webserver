cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD_17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} thread_src)

add_library(thread_pool OBJECT ${thread_src})
target_link_libraries(thread_pool locker) # thread_pool中有使用到locker中的函数，因此链接的时候要把locker链接进来
