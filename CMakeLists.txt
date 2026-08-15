cmake_minimum_required(VERSION 3.22)
project(bilinear_tensor_optimization LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# Exact linear algebra over finite fields, from the CASYS team at the LJK. The
# heuristics here are searches over ranks, so an approximate rank is not a
# slightly worse answer, it is a different problem.
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFLAS_FFPACK REQUIRED IMPORTED_TARGET fflas-ffpack)

add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE -Wall -Wextra -Wpedantic)

enable_testing()
add_subdirectory(exact)
add_subdirectory(rank/cpp)
