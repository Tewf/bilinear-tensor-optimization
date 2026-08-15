add_library(sparsify STATIC sparsify.cpp)
target_include_directories(sparsify PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(sparsify PUBLIC exact PRIVATE project_warnings)

add_executable(sparsify-operator sparsify_main.cpp)
target_link_libraries(sparsify-operator PRIVATE sparsify project_warnings)

add_subdirectory(tests)
