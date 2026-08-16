#pragma once

#include <cstddef>
#include <vector>

#include "types.h"

/// Fixing the first term of a decomposition to one representative per orbit.
///
/// A solver looking for `r` rank-one terms explores every choice of the first
/// one, and the group makes most of those choices the same question asked again:
/// if `σ` maps the target subspace to itself then it carries any decomposition
/// to another, so a solution can always be moved until its first term is an
/// orbit representative. For `⟨3,3,3⟩` that is thirteen choices instead of
/// 261 121, and for `⟨4,4,4⟩` twenty-six instead of 4 294 836 225.
///
/// Emitted as **cubes** rather than as one disjunction: one instance per
/// representative, each with the first term pinned. They are independent, so
/// they run in parallel, and each is a smaller formula than the disjunction
/// would be. The cubes are chosen by the group rather than by a lookahead
/// heuristic, which is the only unusual thing about this cube-and-conquer.
///
/// **This is the dangerous kind of constraint.** An over-strong symmetry break
/// turns a satisfiable formula unsatisfiable, and a wrong "no" is a wrong lower
/// bound, which nothing downstream can catch. Validate on maps of known rank
/// before believing any refutation built on it.
namespace bilinear_rank {

/// One cube per orbit, as literals over the caller's variables.
///
/// `left_variables` and `right_variables` are the solver's variable numbers for
/// the first term's two operand vectors, in coordinate order. A positive literal
/// asserts the coordinate is one, a negative literal that it is zero, which is
/// the DIMACS convention and needs no header from the encoder to honour.
std::vector<std::vector<int>> orbit_cubes(const Field& field, std::size_t rows, std::size_t inner,
                                          std::size_t columns,
                                          const std::vector<int>& left_variables,
                                          const std::vector<int>& right_variables);

}  // namespace bilinear_rank
