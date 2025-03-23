cmake_minimum_required(VERSION 3.16)
project(web)

set(CMAKE_CXX_STANDARD_17)
add_executable(web main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

include_directories(locker)
add_subdirectory(locker)

include_directories(thread_pool)
add_subdirectory(thread_pool)

include_directories(http)
add_subdirectory(http)

target_link_libraries(web locker thread_pool http)

