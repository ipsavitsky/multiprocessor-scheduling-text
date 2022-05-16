cmake_minimum_required(VERSION 3.22)

project(Scheduling VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)

include_directories(schedule)

include_directories(logging)

add_executable(greedy main.cpp)

add_subdirectory(schedule)

add_subdirectory(logging)

target_link_libraries(greedy schedule logging)