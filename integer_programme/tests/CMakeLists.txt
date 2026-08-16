add_executable(test_optimisation test_optimisation.cpp)
target_link_libraries(test_optimisation PRIVATE integer_programme testing project_warnings)
add_test(NAME optimisation COMMAND test_optimisation)

add_executable(test_integer_programme_encoding test_integer_programme_encoding.cpp)
target_link_libraries(test_integer_programme_encoding PRIVATE integer_programme_encoding
                                                              internship_heuristic formats
                                                              testing project_warnings)
add_test(NAME integer_programme_encoding
         COMMAND test_integer_programme_encoding ${PROJECT_SOURCE_DIR}/fixtures)
