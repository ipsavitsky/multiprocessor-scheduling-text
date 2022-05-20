cmake_minimum_required(VERSION 3.22)
project(time_schedule)

set(CMAKE_CXX_STANDARD 20)

# set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=*")

include_directories(../logging)

add_library(time_schedule STATIC time_schedule.cpp)

target_link_libraries(time_schedule logging)