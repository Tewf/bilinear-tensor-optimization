#pragma once

#include <cstddef>
#include <vector>

#include "linear_algebra.h"
#include "matrix.h"

/// The heuristic itself: rewrite a bilinear map in a different basis of the
/// same space, so that the ranks of its slices sum to less than they did.
///
/// Nothing here proves the result is optimal. It is a search, and every step is
/// greedy.
namespace rank_search {

using exact::Field;
using exact::Matrix;

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

}  // namespace rank_search
