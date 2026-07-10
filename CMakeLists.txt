cmake_minimum_required(VERSION 3.30)

project(dslc VERSION 1.0)

option(BUILD_TESTS "Build the tests" OFF)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(DSLC_CMAKE_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/dslc")

add_subdirectory(scanner)

add_executable(dslc dslc/dslc_main.cpp)
target_link_libraries(dslc PRIVATE libscanner)

add_executable(dslc-scanner dslc/scanner_main.cpp)
target_link_libraries(dslc-scanner PRIVATE libscanner)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif (BUILD_TESTS)

configure_package_config_file(
    cmake/dslcConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/dslcConfig.cmake"
    INSTALL_DESTINATION "${DSLC_CMAKE_INSTALL_DIR}"
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/dslcConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
)

install(
    EXPORT dslcTargets
    FILE dslcTargets.cmake
    NAMESPACE dslc::
    DESTINATION "${DSLC_CMAKE_INSTALL_DIR}"
    CXX_MODULES_DIRECTORY modules
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/dslcConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/dslcConfigVersion.cmake"
    DESTINATION "${DSLC_CMAKE_INSTALL_DIR}"
)

include(cmake/clang-format.cmake)
add_clang_format_target(format scanner dslc tests)
