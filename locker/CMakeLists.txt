cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD_17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} locker_src)

add_library(locker OBJECT ${locker_src})


