cmake_minimum_required(VERSION 3.30)

project(dslc-tests)

enable_testing()
find_package(GTest REQUIRED)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(
    dslc-test
    scanner/generator/nfa-dfa-conversion/StateSetTest.cpp
)

target_link_libraries(
    dslc-test PRIVATE
    scanner
    GTest::Main
    GTest::GTest
    stdc++
)

add_test(NAME DefaultTests COMMAND dslc-test)