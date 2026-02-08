cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

project(IACrux LANGUAGES C CXX)
enable_testing()

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(IACRUX_IS_TOP_LEVEL ON)
else()
    set(IACRUX_IS_TOP_LEVEL OFF)
endif()

set(IACRUX_ROOT "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "")
set(IACRUX_CMAKE_DIR "${IACRUX_ROOT}/cmake" CACHE INTERNAL "")
list(APPEND CMAKE_MODULE_PATH "${IACRUX_CMAKE_DIR}")
include(crux_setup_project)

message(STATUS "[IACrux] Detected Compiler: ${CMAKE_CXX_COMPILER_ID}")

if (MSVC AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR 
        "\n-------------------------------------------------------------\n"
        "CRITICAL ERROR: Unsupported Compiler (MSVC/cl.exe)\n"
        "IACrux requires GCC or Clang. On Windows, use 'Clang-cl' or MinGW.\n"
        "-------------------------------------------------------------\n"
    )
endif()

crux_setup_project()

option(IACrux_BUILD_TESTS "Build unit tests" ${IACRUX_IS_TOP_LEVEL})

include(find_deps)

add_subdirectory(src)

if(IACrux_BUILD_TESTS)
    add_subdirectory(tests)
endif()
