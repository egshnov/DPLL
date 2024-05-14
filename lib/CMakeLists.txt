SET(HEADERS CNF.h DPLL.h errormsg.h)
SET(SOURCES CNF.cpp DPLL.cpp)
add_library(Solver STATIC ${HEADERS} ${SOURCES})

target_include_directories(Solver PUBLIC .)