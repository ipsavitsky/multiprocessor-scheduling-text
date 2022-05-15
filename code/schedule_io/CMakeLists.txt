cmake_minimum_required(VERSION 3.22)
project(schedule_io)

set(CMAKE_CXX_STANDARD 20)

add_library(schedule_io STATIC schedule_io.cpp)

target_link_libraries(schedule_io)