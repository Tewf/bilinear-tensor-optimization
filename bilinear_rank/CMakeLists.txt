add_library(bilinear_rank STATIC span_enumeration.cpp smallest_basis.cpp minimise_rank.cpp
                              algorithm_recovery.cpp exhaustive_search.cpp fewest_products.cpp
                              candidate_pool.cpp memory_budget.cpp parallel.cpp
                              automorphism.cpp group_construction.cpp orbit_search.cpp
                              orbit_heuristic.cpp rank_one_basis.cpp plateau_search.cpp
                              pool_orbits.cpp orbit_cubes.cpp flip_graph.cpp
                              map_construction.cpp)
target_include_directories(bilinear_rank PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(bilinear_rank PUBLIC linear_algebra formats PRIVATE project_warnings)

# The one part of the rank strand that speaks to the linear programme, kept out
# of the archive every other command links. Marking the dependency PRIVATE would
# not have been enough: a static library propagates even a private dependency as
# a link requirement, because its objects still reference those symbols, so
# `make-tensor` would go on naming liboptimisation.a on its link line. Taking the
# translation unit out of the archive is what actually severs it.
add_library(integer_programme_encoding STATIC integer_programme_encoding.cpp)
target_link_libraries(integer_programme_encoding PUBLIC bilinear_rank optimisation
                                                 PRIVATE project_warnings)

add_subdirectory(commands)
add_subdirectory(tests)
