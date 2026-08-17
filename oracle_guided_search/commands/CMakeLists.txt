set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..)

add_executable(find-at-rank find_at_rank_main.cpp)
target_link_libraries(find-at-rank PRIVATE oracle_guided_search cli formats project_warnings)
