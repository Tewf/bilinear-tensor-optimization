add_executable(test_sms test_sms.cpp)
target_include_directories(test_sms PRIVATE ${PROJECT_SOURCE_DIR}/linear_algebra/tests)
target_link_libraries(test_sms PRIVATE formats project_warnings)
add_test(NAME sms COMMAND test_sms ${PROJECT_SOURCE_DIR}/fixtures)

add_executable(test_sms_interoperability test_sms_interoperability.cpp)
target_include_directories(test_sms_interoperability PRIVATE ${PROJECT_SOURCE_DIR}/linear_algebra/tests)
target_link_libraries(test_sms_interoperability PRIVATE formats project_warnings)
add_test(NAME sms_interoperability
         COMMAND test_sms_interoperability ${PROJECT_SOURCE_DIR}/fixtures)

add_executable(test_dimacs test_dimacs.cpp)
target_include_directories(test_dimacs PRIVATE ${PROJECT_SOURCE_DIR}/linear_algebra/tests)
target_link_libraries(test_dimacs PRIVATE formats project_warnings)
add_test(NAME dimacs COMMAND test_dimacs)
