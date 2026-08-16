# Handing a minimisation to somebody else's solver: the model, the built-in
# simplex and branch and bound, the MPS writer, and the chain that tries the
# installed backends in turn.
add_library(integer_programme STATIC integer_programme.cpp standard_form.cpp simplex.cpp
                                     branch_and_bound.cpp mps_format.cpp external_solver.cpp
                                     solver_chain.cpp)
target_include_directories(integer_programme PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(integer_programme PUBLIC linear_algebra PRIVATE project_warnings)

# The one part of the rank strand that speaks to the linear programme, kept out
# of the archive every other command links. Marking the dependency PRIVATE would
# not have been enough: a static library propagates even a private dependency as
# a link requirement, because its objects still reference those symbols, so
# `make-tensor` would go on naming libinteger_programme.a on its link line.
# Taking the translation unit out of the archive is what actually severs it.
add_library(integer_programme_encoding STATIC integer_programme_encoding.cpp)
target_link_libraries(integer_programme_encoding PUBLIC integer_programme linear_algebra
                                                PRIVATE project_warnings)

add_subdirectory(commands)
add_subdirectory(tests)
