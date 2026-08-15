add_executable(test_fixtures test_fixtures.cpp)
target_link_libraries(test_fixtures PRIVATE exact project_warnings)
add_test(NAME fixtures COMMAND test_fixtures ${PROJECT_SOURCE_DIR}/fixtures)
