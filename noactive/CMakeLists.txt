cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD_17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(noa noactive.cpp)
