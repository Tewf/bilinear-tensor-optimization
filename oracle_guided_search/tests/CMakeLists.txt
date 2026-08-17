# The checks that need no solver, which is the floor refusal and the arithmetic.
add_executable(test_fixed_rank_finder test_fixed_rank_finder.cpp)
target_link_libraries(test_fixed_rank_finder PRIVATE oracle_guided_search formats testing
                                                     project_warnings)
add_test(NAME fixed_rank_finder COMMAND test_fixed_rank_finder ${PROJECT_SOURCE_DIR}/fixtures)

# The one that calibrates the whole module: Strassen's seven, found and multiplied
# out, and a descent that stops at seven rather than below it. Behind a flag
# because it runs a solver.
add_test(NAME fixed_rank_finder_finds_strassen
         COMMAND test_fixed_rank_finder ${PROJECT_SOURCE_DIR}/fixtures --with-solver)
set_tests_properties(fixed_rank_finder_finds_strassen PROPERTIES LABELS slow TIMEOUT 900)
