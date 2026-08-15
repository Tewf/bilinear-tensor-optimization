add_library(bilinear_rank STATIC span_enumeration.cpp smallest_basis.cpp minimise_rank.cpp
                              algorithm_recovery.cpp exhaustive_search.cpp fewest_products.cpp
                              candidate_pool.cpp memory_budget.cpp
                              map_construction.cpp)
target_include_directories(bilinear_rank PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(bilinear_rank PUBLIC linear_algebra formats PRIVATE project_warnings)

add_subdirectory(commands)
add_subdirectory(tests)
