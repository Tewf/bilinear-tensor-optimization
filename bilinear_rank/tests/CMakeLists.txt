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
