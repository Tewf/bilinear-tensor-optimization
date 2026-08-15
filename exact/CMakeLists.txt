add_library(exact STATIC tensor.cpp rational_matrix_io.cpp)
target_include_directories(exact PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(exact PUBLIC PkgConfig::GIVARO PRIVATE project_warnings)

add_subdirectory(tests)
