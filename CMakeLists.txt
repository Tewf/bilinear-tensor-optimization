cmake_minimum_required(VERSION 3.22)
project(bilinear_tensor_optimization LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# Givaro, from the CASYS team at the LJK, for exact arithmetic over GF(p) and
# over the rationals. Both strands here search over ranks and over counts of
# nonzero entries, so an answer that is nearly right answers a different
# question; nothing in this repository is ever a float.
find_package(PkgConfig REQUIRED)
pkg_check_modules(GIVARO REQUIRED IMPORTED_TARGET givaro)

add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE -Wall -Wextra -Wpedantic)

enable_testing()
add_subdirectory(cli)
add_subdirectory(linear_algebra)
add_subdirectory(formats)
add_subdirectory(bilinear_rank)
add_subdirectory(matrix_sparsification)
