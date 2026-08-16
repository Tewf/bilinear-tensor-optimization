add_executable(test_fixtures test_fixtures.cpp)
target_link_libraries(test_fixtures PRIVATE formats project_warnings testing)
add_test(NAME fixtures COMMAND test_fixtures ${PROJECT_SOURCE_DIR}/fixtures)

add_executable(test_rank_one test_rank_one.cpp)
target_link_libraries(test_rank_one PRIVATE formats project_warnings testing)
add_test(NAME rank_one COMMAND test_rank_one ${PROJECT_SOURCE_DIR}/fixtures)

add_executable(test_tensor_flattening test_tensor_flattening.cpp)
target_link_libraries(test_tensor_flattening PRIVATE formats project_warnings testing)
add_test(NAME tensor_flattening COMMAND test_tensor_flattening ${PROJECT_SOURCE_DIR}/fixtures)
