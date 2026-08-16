add_library(bilinear_rank STATIC span_enumeration.cpp smallest_basis.cpp minimise_rank.cpp
                              algorithm_recovery.cpp exhaustive_search.cpp fewest_products.cpp
                              candidate_pool.cpp memory_budget.cpp parallel.cpp
                              automorphism.cpp group_construction.cpp orbit_search.cpp
                              orbit_heuristic.cpp rank_one_basis.cpp plateau_search.cpp
                              pool_orbits.cpp orbit_cubes.cpp flip_graph.cpp integer_programme_encoding.cpp
                              map_construction.cpp)
target_include_directories(bilinear_rank PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(bilinear_rank PUBLIC linear_algebra formats optimisation PRIVATE project_warnings)

add_subdirectory(commands)
add_subdirectory(tests)
