cmake_minimum_required(VERSION 3.14)

set(CMAKE_BUILD_TYPE Release)

project(DPLL)

find_package(benchmark REQUIRED)

add_executable(DPLL main.cpp)

add_subdirectory(lib)

target_link_libraries(DPLL Solver)

target_link_libraries(DPLL benchmark::benchmark)