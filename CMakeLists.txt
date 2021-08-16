# TODO: Determine the lowest version of CMake allowable
cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(samos LANGUAGES C CXX VERSION 0.2.0)
configure_file (
    "${PROJECT_SOURCE_DIR}/samos_config.hpp.in"
    "${PROJECT_SOURCE_DIR}/samos_config.hpp"
)
enable_testing()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_CPPCHECK "cppcheck")
set(CXX_CPPCHECK "project=target/debug/compile_commands.json;enable=information;force")

include(FindPostgreSQL)
include(GoogleTest)
include(samos_cmake_functions.cmake)

find_package(Eigen3 3.3 REQUIRED)
find_package(GTest REQUIRED)
# Require dot, treat the other components as optional
find_package(Doxygen
             REQUIRED dot
             OPTIONAL_COMPONENTS mscgen dia)

option(COVERAGE "Add coverage compilation and link options." OFF)
if (${COVERAGE})
  add_compile_options("--coverage")
  add_link_options("--coverage")
endif()

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(data)
