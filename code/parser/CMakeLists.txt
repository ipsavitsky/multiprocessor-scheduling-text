cmake_minimum_required(VERSION 3.22)
project(parser)

set(CMAKE_CXX_STANDARD 17)

# set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=*")

include_directories(../logging ../schedule)

add_library(parser STATIC huawei_parser.cpp)

target_link_libraries(parser logging schedule)