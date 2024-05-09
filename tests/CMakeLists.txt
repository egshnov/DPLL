project(tests)

# 'lib' is the folder with Google Test sources
add_subdirectory(google-test)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(tests_run test.cpp)
target_link_libraries(tests_run Solver)
target_link_libraries(tests_run gtest gtest_main)

add_test(NAME TESTS COMMAND tests_run)