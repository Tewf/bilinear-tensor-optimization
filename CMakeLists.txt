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
#
# FFLAS-FFPACK was used at first for rank and then dropped. Its blocked
# routines are built for matrices far larger than the ones here, and reducing a
# candidate against a basis held in echelon form measured faster on every
# fixture. Givaro supplies what is actually load-bearing: modular inverse and
# exact rational arithmetic that cannot overflow or round.
find_package(PkgConfig REQUIRED)
pkg_check_modules(GIVARO REQUIRED IMPORTED_TARGET givaro)

add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE -Wall -Wextra -Wpedantic)

enable_testing()
add_subdirectory(exact)
add_subdirectory(rank/cpp)
add_subdirectory(sparsify/cpp)
