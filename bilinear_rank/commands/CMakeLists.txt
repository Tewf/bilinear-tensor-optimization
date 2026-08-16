# The binaries stay at build/bilinear_rank/, where the CI steps, the READMEs and
# anything already typed at a shell expect to find them. Only the sources moved.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..)

add_executable(minimise-rank minimise_rank_main.cpp)
target_link_libraries(minimise-rank PRIVATE bilinear_rank cli project_warnings)

add_executable(decide-rank decide_rank_main.cpp)
target_link_libraries(decide-rank PRIVATE bilinear_rank cli project_warnings)

add_executable(make-tensor make_tensor_main.cpp)
target_link_libraries(make-tensor PRIVATE bilinear_rank cli project_warnings)

add_executable(walk-scheme walk_scheme_main.cpp)
target_link_libraries(walk-scheme PRIVATE bilinear_rank cli project_warnings)
