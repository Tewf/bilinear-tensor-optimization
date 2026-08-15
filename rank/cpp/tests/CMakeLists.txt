add_executable(test_step1 test_step1.cpp)
target_include_directories(test_step1 PRIVATE ${PROJECT_SOURCE_DIR}/exact/tests)
target_link_libraries(test_step1 PRIVATE rank_search project_warnings)
add_test(NAME step1 COMMAND test_step1 ${PROJECT_SOURCE_DIR}/fixtures)
