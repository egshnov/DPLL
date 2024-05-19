cmake_minimum_required(VERSION 3.14)
set(CMAKE_BUILD_TYPE Release)

project(DPLL)

set(CMAKE_CXX_STANDARD 20)
add_executable(DPLL main.cpp)

enable_testing()

add_subdirectory(lib)

target_link_libraries(DPLL Solver)