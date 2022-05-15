cmake_minimum_required(VERSION 3.22)
project(schedule)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER "/usr/bin/clang++-14")

add_library(schedule STATIC schedule.cpp)

target_link_libraries(schedule)