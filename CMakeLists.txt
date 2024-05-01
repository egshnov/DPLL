cmake_minimum_required(VERSION 3.28)
project(DPLL)

set(CMAKE_CXX_STANDARD 17)

add_executable(DPLL main.cpp
        CNF.hpp
        CNF.cpp
        DPLL.hpp
)
