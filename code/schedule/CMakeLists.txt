cmake_minimum_required(VERSION 3.22)
project(schedule)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost 1.74 REQUIRED COMPONENTS graph)

add_library(schedule STATIC schedule.cpp)

target_link_libraries(schedule Boost::graph)