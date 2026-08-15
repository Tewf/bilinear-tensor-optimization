add_library(bilinear_rank STATIC span_enumeration.cpp smallest_basis.cpp minimise_rank.cpp
                              algorithm_recovery.cpp exhaustive_search.cpp
                              candidate_pool.cpp
                              map_construction.cpp)
target_include_directories(bilinear_rank PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(bilinear_rank PUBLIC linear_algebra formats PRIVATE project_warnings)

add_executable(minimise-rank minimise_rank_main.cpp)
target_link_libraries(minimise-rank PRIVATE bilinear_rank project_warnings)

add_subdirectory(tests)

add_executable(decide-rank decide_rank_main.cpp)
target_link_libraries(decide-rank PRIVATE bilinear_rank project_warnings)

add_executable(make-tensor make_tensor_main.cpp)
target_link_libraries(make-tensor PRIVATE bilinear_rank project_warnings)
