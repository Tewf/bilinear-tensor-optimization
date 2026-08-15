add_executable(test_sms test_sms.cpp)
target_include_directories(test_sms PRIVATE ${PROJECT_SOURCE_DIR}/linear_algebra/tests)
target_link_libraries(test_sms PRIVATE formats project_warnings)
add_test(NAME sms COMMAND test_sms ${PROJECT_SOURCE_DIR}/fixtures)
