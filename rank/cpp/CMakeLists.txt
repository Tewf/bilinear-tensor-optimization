add_library(rank_search STATIC search.cpp)
target_include_directories(rank_search PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(rank_search PUBLIC exact PRIVATE project_warnings)

add_executable(minimise-rank minimise_rank_main.cpp)
target_link_libraries(minimise-rank PRIVATE rank_search project_warnings)

add_subdirectory(tests)
