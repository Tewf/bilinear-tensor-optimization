add_library(bilinear_rank STATIC heuristic_search.cpp)
target_include_directories(bilinear_rank PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(bilinear_rank PUBLIC linear_algebra formats PRIVATE project_warnings)

add_executable(minimise-rank minimise_rank_main.cpp)
target_link_libraries(minimise-rank PRIVATE bilinear_rank project_warnings)

add_subdirectory(tests)
