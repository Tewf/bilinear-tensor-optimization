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

add_executable(test_prime_field_encoding test_prime_field_encoding.cpp)
target_include_directories(test_prime_field_encoding
                           PRIVATE ${PROJECT_SOURCE_DIR}/linear_algebra/tests)
target_link_libraries(test_prime_field_encoding PRIVATE satisfiability project_warnings)
add_test(NAME prime_field_encoding COMMAND test_prime_field_encoding)

add_executable(test_field_theory_encoding test_field_theory_encoding.cpp)
target_include_directories(test_field_theory_encoding
                           PRIVATE ${PROJECT_SOURCE_DIR}/linear_algebra/tests)
target_link_libraries(test_field_theory_encoding PRIVATE satisfiability project_warnings)
add_test(NAME field_theory_encoding COMMAND test_field_theory_encoding)

add_executable(test_end_to_end test_end_to_end.cpp)
target_include_directories(test_end_to_end PRIVATE ${PROJECT_SOURCE_DIR}/linear_algebra/tests)
target_link_libraries(test_end_to_end PRIVATE satisfiability project_warnings)
add_test(NAME end_to_end COMMAND test_end_to_end ${PROJECT_SOURCE_DIR}/fixtures)

# The same loop on a tensor the solver has to work for, with the refusal checked
# by drat-trim. Labelled slow the way bilinear_rank labels its own searches.
add_test(NAME end_to_end_slow COMMAND test_end_to_end ${PROJECT_SOURCE_DIR}/fixtures --slow)
set_tests_properties(end_to_end_slow PROPERTIES LABELS slow TIMEOUT 900)
