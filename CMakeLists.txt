cmake_minimum_required(VERSION 3.30)

project(dslc VERSION 1.0)

option(BUILD_TESTS "Build the tests" OFF)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(scanner)

add_executable(dslc dslc/dslc_main.cpp)
target_link_libraries(dslc PRIVATE libscanner)

add_executable(dslc-scanner dslc/scanner_main.cpp)
target_link_libraries(dslc-scanner PRIVATE libscanner)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif (BUILD_TESTS)

include(cmake/clang-format.cmake)
add_clang_format_target(format scanner dslc tests)
