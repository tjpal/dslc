if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    message(FATAL_ERROR "The Homebrew LLVM toolchain supports macOS only")
endif()

find_program(BREW_EXECUTABLE brew REQUIRED)
execute_process(
    COMMAND "${BREW_EXECUTABLE}" --prefix llvm
    OUTPUT_VARIABLE LLVM_ROOT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
)

set(CMAKE_C_COMPILER "${LLVM_ROOT}/bin/clang" CACHE FILEPATH "C compiler")
set(CMAKE_CXX_COMPILER "${LLVM_ROOT}/bin/clang++" CACHE FILEPATH "C++ compiler")

set(CMAKE_CXX_FLAGS_INIT "-stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS_INIT
    "-stdlib=libc++ -L${LLVM_ROOT}/lib/c++ -Wl,-rpath,${LLVM_ROOT}/lib/c++"
)
set(CMAKE_SHARED_LINKER_FLAGS_INIT
    "-stdlib=libc++ -L${LLVM_ROOT}/lib/c++ -Wl,-rpath,${LLVM_ROOT}/lib/c++"
)
