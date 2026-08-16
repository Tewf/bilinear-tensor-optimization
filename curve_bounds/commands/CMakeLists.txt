set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..)

add_executable(curve-bounds curve_bounds_main.cpp)
target_link_libraries(curve-bounds PRIVATE curve_bounds project_warnings)
