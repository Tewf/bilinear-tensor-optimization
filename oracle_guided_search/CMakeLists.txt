# The oracle answers one question; this decides which questions are worth asking.
# Four files because there are four jobs: answering one `k` cheaply, choosing which
# `k` to ask about next, answering one `k` with a proof instead of a budget, and
# buying that proof from the tree rather than from a solver.
add_library(oracle_guided_search STATIC fixed_rank_finder.cpp descending_sweep.cpp
                                        strict_deflation.cpp tree_refutation.cpp)
target_include_directories(oracle_guided_search PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# This module is where the two sides meet, and the meeting is one-directional:
# nothing in `satisfiability` or `orbit_reduction` knows this exists. That is the
# same arrangement `satisfiability/commands` documents, moved into a library so a
# test can link it.
target_link_libraries(oracle_guided_search PUBLIC satisfiability orbit_reduction
                                                  internship_heuristic exhaustive_search
                                                  run_limits
                                           PRIVATE map_construction cli project_warnings)

add_subdirectory(commands)
add_subdirectory(tests)
