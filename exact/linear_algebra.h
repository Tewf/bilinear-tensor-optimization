#pragma once

#include <givaro/modular.h>

#include <cstddef>
#include <vector>

#include "matrix.h"

namespace exact {

/// One field type for the whole finite-field side of the repository. The primes
/// in play are tiny, so a machine integer representation is ample and the
/// arithmetic stays exact by construction rather than by convention.
using Field = Givaro::Modular<int64_t>;

/// Exact rank over the field. The original computed this with a hand-written
/// elimination that reduced modulo p only at the very end, which overflowed
/// int64 and admitted negative residues; this delegates to FFPACK.
std::size_t rank(const Field& field, const Matrix& matrix);

/// Rank of a set of matrices regarded as vectors in the space they live in,
/// which is what "how many of these are linearly independent" means here.
std::size_t rank_of_span(const Field& field, const std::vector<Matrix>& matrices);

/// Whether `candidate` lies outside the span of `basis`.
///
/// The original wrote this as two branches that reduce to the same predicate,
/// with a zero-matrix case that can never be true. Adding one vector raises the
/// rank by at most one, so the strict increase says it all.
bool raises_rank(const Field& field, const std::vector<Matrix>& basis, const Matrix& candidate);

}  // namespace exact
