add_executable(test_sparsify test_sparsify.cpp)
target_link_libraries(test_sparsify PRIVATE matrix_sparsification project_warnings testing)
add_test(NAME sparsify COMMAND test_sparsify ${PROJECT_SOURCE_DIR}/fixtures)
