function(add_clang_format_target target_name)
    find_program(CLANG_FORMAT_EXEC clang-format)

    if (NOT CLANG_FORMAT_EXEC)
        message(FATAL_ERROR "clang-format not found")
    endif()

    set(ALL_FILES_TO_FORMAT "")
    foreach(folder IN LISTS ARGN)
        file(GLOB_RECURSE FILES_IN_FOLDER
            ${CMAKE_SOURCE_DIR}/${folder}/*.cpp
            ${CMAKE_SOURCE_DIR}/${folder}/*.hpp
            ${CMAKE_SOURCE_DIR}/${folder}/*.h
            ${CMAKE_SOURCE_DIR}/${folder}/*.cppm
            ${CMAKE_SOURCE_DIR}/${folder}/*.ixx
        )
        list(APPEND ALL_FILES_TO_FORMAT ${FILES_IN_FOLDER})
    endforeach()

    message(STATUS "Found clang-format: ${CLANG_FORMAT_EXEC}")
    message(STATUS "All files to format: ${ALL_FILES_TO_FORMAT}")

    add_custom_target(${target_name}
        COMMAND ${CLANG_FORMAT_EXEC} -i ${ALL_FILES_TO_FORMAT}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Format all source files in the project"
    )
endfunction()
