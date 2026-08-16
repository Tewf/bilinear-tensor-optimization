add_library(optimisation STATIC integer_programme.cpp standard_form.cpp simplex.cpp
                                branch_and_bound.cpp mps_format.cpp external_solver.cpp
                                solver_chain.cpp)
target_include_directories(optimisation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(optimisation PUBLIC linear_algebra PRIVATE project_warnings)

add_subdirectory(commands)
add_subdirectory(tests)
