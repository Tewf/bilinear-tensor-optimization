set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..)

add_executable(decide-rank-by-sat decide_rank_by_sat_main.cpp)
target_link_libraries(decide-rank-by-sat PRIVATE satisfiability cli project_warnings)
