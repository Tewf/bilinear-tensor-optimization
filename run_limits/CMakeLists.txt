# How much of the machine one run may take: memory, and cores. Shared by every
# method, so it belongs to none of them.
add_library(run_limits STATIC memory_budget.cpp parallel.cpp)
target_include_directories(run_limits PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(run_limits PRIVATE project_warnings)
