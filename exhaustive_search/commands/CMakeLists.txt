set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..)

# --anchor heuristic runs the heuristic first and searches from its result, so
# this command needs both methods; the library needs neither the other.
add_executable(decide-rank decide_rank_main.cpp)
target_link_libraries(decide-rank PRIVATE exhaustive_search internship_heuristic formats cli
                                          project_warnings)
