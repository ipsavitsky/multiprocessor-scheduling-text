cmake_minimum_required(VERSION 3.22)
project(time_schedule)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost 1.74 REQUIRED COMPONENTS graph)

include_directories(../logging)

add_library(time_schedule STATIC time_schedule.cpp)

target_link_libraries(time_schedule)