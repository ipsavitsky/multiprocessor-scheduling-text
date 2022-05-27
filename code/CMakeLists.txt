cmake_minimum_required(VERSION 3.22)

project(Scheduling VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE Debug)

find_package(Boost 1.74 REQUIRED COMPONENTS program_options)

include_directories(schedule logging time_schedule parser)

add_executable(greedy main.cpp)

add_subdirectory(schedule)

add_subdirectory(logging)

add_subdirectory(time_schedule)

add_subdirectory(parser)

target_link_libraries(greedy schedule logging time_schedule parser Boost::program_options)