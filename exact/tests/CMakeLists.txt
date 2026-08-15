add_executable(test_fixtures test_fixtures.cpp)
target_link_libraries(test_fixtures PRIVATE exact project_warnings)
add_test(NAME fixtures COMMAND test_fixtures ${PROJECT_SOURCE_DIR}/fixtures)

add_executable(test_rank_one test_rank_one.cpp)
target_link_libraries(test_rank_one PRIVATE exact project_warnings)
add_test(NAME rank_one COMMAND test_rank_one ${PROJECT_SOURCE_DIR}/fixtures)
