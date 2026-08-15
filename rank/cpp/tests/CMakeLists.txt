add_executable(test_pipeline test_pipeline.cpp)
target_include_directories(test_pipeline PRIVATE ${PROJECT_SOURCE_DIR}/exact/tests)
target_link_libraries(test_pipeline PRIVATE rank_search project_warnings)
add_test(NAME pipeline COMMAND test_pipeline ${PROJECT_SOURCE_DIR}/fixtures)
