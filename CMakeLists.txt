cmake_minimum_required(VERSION 3.14)
project(DPLL)

set(CMAKE_CXX_STANDARD 17)
add_executable(DPLL main.cpp)

enable_testing()

add_subdirectory(lib)
add_subdirectory(tests)

target_link_libraries(DPLL Solver)