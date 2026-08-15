#pragma once

#include <cstddef>
#include <vector>

#include "candidate_pool.h"
#include "types.h"
#include "matrix.h"
#include "span_basis.h"

/// The heuristic itself: rewrite a bilinear map in a different basis of the
/// same space, so that the ranks of its slices sum to less than they did.
///
/// Nothing here proves the result is optimal. It is a search, and every step is
/// greedy.
namespace bilinear_rank {

/// The linear combination `sum_i coefficients[i] * slices[i]`, reduced.
Matrix combine(const Field& field, const std::vector<Matrix>& slices,
               const std::vector<int64_t>& coefficients);

/// The coefficient vector at `index`, counting with the first coordinate
/// varying fastest. This is the order Julia's `product` produces, kept so that
/// ties between equal-rank candidates break the way the internship's run broke
/// them.
std::vector<int64_t> coefficient_vector(std::size_t index, std::size_t count, int64_t characteristic);

/// How many elements the span has, which is how many combinations must be
/// enumerated to see all of it.
std::size_t span_size(const Field& field, std::size_t slice_count);

/// Step 1. Walk the span of the map from lowest rank upwards, greedily keeping
/// anything that is not already spanned, until a full basis is assembled.
///
/// The result spans exactly what it was given, so it computes the same bilinear
/// map, and its ranks sum to no more than the original's.
std::vector<Matrix> smallest_basis(const Field& field, const std::vector<Matrix>& slices);

/// Keep only the candidates that, taken one at a time, would improve the map.
///
/// Named for what it does. The original called this `filter`, which shadowed
/// Julia's own `filter`, still called elsewhere in the same file.
std::vector<Matrix> improving_candidates(const Field& field, const std::vector<Matrix>& slices,
                                         const std::vector<Matrix>& candidates);

/// Steps 2 and 3, which differ only in the pool they are given.
///
/// Adopt a candidate whenever the basis of the enlarged span costs fewer
/// multiplications than the map costs now. Enlarging is allowed: the result
/// only has to *generate* the original map, so a spanning set of more slices
/// but lower total rank is a better answer, which is exactly what Karatsuba's
/// five products for a four-coefficient product is.
std::vector<Matrix> minimise_rank(const Field& field, std::vector<Matrix> slices,
                                  std::vector<Matrix> candidates);

}  // namespace bilinear_rank
