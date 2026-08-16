# Header-only, and deliberately tiny: the commands share a clock and nothing
# else. Anything larger belongs to the strand that owns the question it answers.
add_library(cli INTERFACE)
target_include_directories(cli INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(tests)
