add_library(exact STATIC linear_algebra.cpp tensor.cpp)
target_include_directories(exact PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(exact PUBLIC PkgConfig::FFLAS_FFPACK PRIVATE project_warnings)

add_subdirectory(tests)
