add_executable(test_sparsify test_sparsify.cpp)
target_include_directories(test_sparsify PRIVATE ${PROJECT_SOURCE_DIR}/exact/tests)
target_link_libraries(test_sparsify PRIVATE sparsify project_warnings)
add_test(NAME sparsify COMMAND test_sparsify ${PROJECT_SOURCE_DIR}/fixtures)
