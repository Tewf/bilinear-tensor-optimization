# The heuristic Mohamed wrote at the LJK in 2024, corrected and continued.
# Three steps: a matroid greedy that is exact for the basis it picks, then two
# that relax the constraint it works under and guarantee nothing.
add_library(internship_heuristic STATIC span_enumeration.cpp smallest_basis.cpp
                                       minimise_rank.cpp algorithm_recovery.cpp
                                       candidate_pool.cpp)
target_include_directories(internship_heuristic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(internship_heuristic PUBLIC linear_algebra run_limits
                                           PRIVATE project_warnings)

add_subdirectory(commands)
add_subdirectory(tests)
