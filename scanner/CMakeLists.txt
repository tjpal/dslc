cmake_minimum_required(VERSION 3.30)

project(scanner)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(scanner STATIC)

target_sources(
    scanner PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
    regex/RegexNode.cppm
    regex/Concatenation.cppm
)