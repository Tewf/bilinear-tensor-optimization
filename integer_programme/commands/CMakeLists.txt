# As in the other strands: the source moved into commands/, the binary did not.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..)

add_executable(list-solvers solver_report.cpp)
target_link_libraries(list-solvers PRIVATE integer_programme project_warnings)

add_executable(decide-rank-by-ilp decide_rank_by_ilp_main.cpp)
target_link_libraries(decide-rank-by-ilp PRIVATE integer_programme_encoding internship_heuristic
                                                 orbit_reduction formats cli project_warnings)
