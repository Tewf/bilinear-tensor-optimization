add_executable(test_formula_to_tensor test_formula_to_tensor.cpp)
target_include_directories(test_formula_to_tensor
                           PRIVATE ${PROJECT_SOURCE_DIR}/linear_algebra/tests)
target_link_libraries(test_formula_to_tensor PRIVATE satisfiability project_warnings)
add_test(NAME formula_to_tensor COMMAND test_formula_to_tensor)

add_executable(test_binary_encoding test_binary_encoding.cpp)
target_include_directories(test_binary_encoding
                           PRIVATE ${PROJECT_SOURCE_DIR}/linear_algebra/tests)
target_link_libraries(test_binary_encoding PRIVATE satisfiability project_warnings)
add_test(NAME binary_encoding COMMAND test_binary_encoding)
