#pragma once

#include <vector>

#include "types.h"

/// Step 1, and the only step of the heuristic that guarantees anything.
///
/// Choosing a basis of `span(T)` whose ranks sum to the least is a **matroid**
/// problem: linear independence forms a matroid, and greedy by ascending weight
/// yields a minimum-weight basis (Rado-Edmonds). So this does not approximate,
/// and its answer does not depend on how ties are broken. What is heuristic is
/// the constraint that the answer be a basis of `span(T)` at all, which is
/// exactly what [steps 2 and 3](minimise_rank.h) relax.
namespace bilinear_rank {

/// Walk the span of the map from lowest rank upwards, greedily keeping anything
/// not already spanned, until a full basis is assembled.
///
/// The result spans exactly what it was given, so it computes the same bilinear
/// map, and its ranks sum to no more than the original's.
std::vector<Matrix> smallest_basis(const Field& field, const std::vector<Matrix>& slices);

}  // namespace bilinear_rank
