cmake_minimum_required(VERSION 3.30)

project(dslc VERSION 1.0)

option(BUILD_TESTS "Build the tests" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(scanner)

add_executable(dslc dslc/main.cpp)
target_link_libraries(dslc PRIVATE scanner)

if(BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)

    add_executable(dslc-test tests/main_test.cpp)
    target_link_libraries(dslc-test PRIVATE GTest::GTest GTest::Main)

    add_test(NAME DefaultTests COMMAND dslc-test)
endif (BUILD_TESTS)