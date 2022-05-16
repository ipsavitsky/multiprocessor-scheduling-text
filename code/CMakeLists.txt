cmake_minimum_required(VERSION 3.22)

project(Scheduling VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost 1.74 REQUIRED COMPONENTS graph log)

include_directories(schedule)

# include_directories(schedule_io)

add_executable(greedy main.cpp)

add_subdirectory(schedule)

# add_subdirectory(schedule_io)

target_link_libraries(greedy schedule Boost::graph Boost::log)