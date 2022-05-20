cmake_minimum_required(VERSION 3.22)

project(Scheduling VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)

include_directories(schedule logging time_schedule)

add_executable(greedy main.cpp)

add_subdirectory(schedule)

add_subdirectory(logging)

add_subdirectory(time_schedule)

target_link_libraries(greedy schedule logging time_schedule)