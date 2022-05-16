cmake_minimum_required(VERSION 3.22)
project(logging)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost 1.74 REQUIRED COMPONENTS log)

add_library(logging STATIC boost_logger.cpp)

target_link_libraries(logging Boost::log)