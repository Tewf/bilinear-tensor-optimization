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

/// The number of multiplications a set of slices costs, which is the quantity
/// the whole search exists to reduce: the sum of their ranks.
std::size_t multiplication_count(const Field& field, const std::vector<Matrix>& slices);

/// Solve `coefficients * rows = target` for a set of rows that are linearly
/// independent, so the solution is unique when it exists at all.
///
/// Returns false when `target` is outside their span. Every operation goes
/// through the field, which is the whole point: the original solved the same
/// system in machine integers and reduced afterwards.
bool solve_in_row_space(const Field& field, const std::vector<std::vector<int64_t>>& rows,
                        const std::vector<int64_t>& target, std::vector<int64_t>& coefficients);

/// Whether every one of `targets` lies in the span of `spanning_set`.
///
/// This is the property that makes a rewrite a rewrite rather than a different
/// map that happens to be cheaper, and it is checked on every run: a search bug
/// that quietly dropped a slice would otherwise report an excellent number.
bool spans_all(const Field& field, const std::vector<Matrix>& spanning_set,
               const std::vector<Matrix>& targets);

/// Write `matrix` as a sum of exactly rank(matrix) rank-one matrices.
///
/// A rank-one bilinear form is one multiplication, so this is what turns a map
/// into the products that compute it, and the terms are the candidates the
/// search then recombines.
std::vector<Matrix> rank_one_decomposition(const Field& field, const Matrix& matrix);

}  // namespace exact
