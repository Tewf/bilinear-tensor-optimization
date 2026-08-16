add_executable(test_pipeline test_pipeline.cpp)
target_include_directories(test_pipeline PRIVATE ${PROJECT_SOURCE_DIR}/linear_algebra/tests)
target_link_libraries(test_pipeline PRIVATE bilinear_rank project_warnings)
add_test(NAME pipeline COMMAND test_pipeline ${PROJECT_SOURCE_DIR}/fixtures)

# Step 3 searches every rank-one map of the shape, so it is the expensive one.
# In Julia these took an hour and a half, or never finished; they are labelled
# so `ctest -LE slow` skips them, but they are cheap enough to keep in CI.
add_executable(test_step3 test_step3.cpp)
target_include_directories(test_step3 PRIVATE ${PROJECT_SOURCE_DIR}/linear_algebra/tests)
target_link_libraries(test_step3 PRIVATE bilinear_rank project_warnings)

# f3_3x6 expects 10, which is our result and not the internship's: its step 3
# never terminated, so the published 11 came from an abandoned run.
foreach(case "f2_5x5;14" "f2_3x8;15" "f2_4x7;16" "f3_3x6;10")
  list(GET case 0 case_name)
  list(GET case 1 case_expected)
  add_test(NAME step3_${case_name}
           COMMAND test_step3 ${PROJECT_SOURCE_DIR}/fixtures ${case_name} ${case_expected})
  set_tests_properties(step3_${case_name} PROPERTIES LABELS slow TIMEOUT 900)
endforeach()

add_executable(test_algorithm_recovery test_algorithm_recovery.cpp)
target_include_directories(test_algorithm_recovery PRIVATE ${PROJECT_SOURCE_DIR}/linear_algebra/tests)
target_link_libraries(test_algorithm_recovery PRIVATE bilinear_rank project_warnings)
add_test(NAME algorithm_recovery COMMAND test_algorithm_recovery ${PROJECT_SOURCE_DIR}/fixtures)

add_executable(test_exhaustive_search test_exhaustive_search.cpp)
target_include_directories(test_exhaustive_search PRIVATE ${PROJECT_SOURCE_DIR}/linear_algebra/tests)
target_link_libraries(test_exhaustive_search PRIVATE bilinear_rank project_warnings)
add_test(NAME exhaustive_search COMMAND test_exhaustive_search ${PROJECT_SOURCE_DIR}/fixtures)
set_tests_properties(exhaustive_search PROPERTIES TIMEOUT 900)

# The other half of `12 <= rank(F2 5x5)`: 459 239 nodes, about 77 seconds. The
# 9 and 10 exclusions cost nothing and stay in the test above.
add_test(NAME f2_5x5_has_no_11_products
         COMMAND test_exhaustive_search ${PROJECT_SOURCE_DIR}/fixtures 11)
set_tests_properties(f2_5x5_has_no_11_products PROPERTIES LABELS slow TIMEOUT 900)

add_executable(test_map_construction test_map_construction.cpp)
target_include_directories(test_map_construction PRIVATE ${PROJECT_SOURCE_DIR}/linear_algebra/tests)
target_link_libraries(test_map_construction PRIVATE bilinear_rank project_warnings)
add_test(NAME map_construction COMMAND test_map_construction)

add_executable(test_candidate_pool test_candidate_pool.cpp)
target_include_directories(test_candidate_pool PRIVATE ${PROJECT_SOURCE_DIR}/linear_algebra/tests)
target_link_libraries(test_candidate_pool PRIVATE bilinear_rank project_warnings)
add_test(NAME candidate_pool COMMAND test_candidate_pool)

# The only test that links both sides of the orbit-cube boundary, because the
# question it asks needs both: do the cubes give the answer the whole formula
# gives? The contract they meet is
# `bilinear_rank/orbit_cube_boundary.md`.
add_executable(test_orbit_cubes test_orbit_cubes.cpp)
target_include_directories(test_orbit_cubes PRIVATE ${PROJECT_SOURCE_DIR}/linear_algebra/tests)
target_link_libraries(test_orbit_cubes PRIVATE bilinear_rank satisfiability project_warnings)
add_test(NAME orbit_cubes COMMAND test_orbit_cubes)

# The validation the boundary actually rests on: the same question whole and
# split into cubes, at 7 products where <2,2,2> is satisfiable and at 6 where it
# is not. Four minutes, and the refutation at 6 is nearly all of it.
add_test(NAME orbit_cubes_preserve_the_answer COMMAND test_orbit_cubes --with-solver)
set_tests_properties(orbit_cubes_preserve_the_answer PROPERTIES LABELS slow TIMEOUT 900)

add_executable(test_flip_graph test_flip_graph.cpp)
target_include_directories(test_flip_graph PRIVATE ${PROJECT_SOURCE_DIR}/linear_algebra/tests)
target_link_libraries(test_flip_graph PRIVATE bilinear_rank project_warnings)
add_test(NAME flip_graph COMMAND test_flip_graph ${PROJECT_SOURCE_DIR}/fixtures)

add_executable(test_integer_programme_encoding test_integer_programme_encoding.cpp)
target_include_directories(test_integer_programme_encoding PRIVATE ${PROJECT_SOURCE_DIR}/linear_algebra/tests)
target_link_libraries(test_integer_programme_encoding PRIVATE bilinear_rank optimisation project_warnings)
add_test(NAME integer_programme_encoding COMMAND test_integer_programme_encoding ${PROJECT_SOURCE_DIR}/fixtures)

add_executable(test_symmetry_agreement test_symmetry_agreement.cpp)
target_include_directories(test_symmetry_agreement PRIVATE ${PROJECT_SOURCE_DIR}/linear_algebra/tests)
target_link_libraries(test_symmetry_agreement PRIVATE bilinear_rank project_warnings)
add_test(NAME symmetry_agreement COMMAND test_symmetry_agreement ${PROJECT_SOURCE_DIR}/fixtures)
