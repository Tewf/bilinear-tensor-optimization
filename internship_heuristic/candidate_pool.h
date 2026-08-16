#pragma once

#include <cstddef>
#include <vector>

#include "bilinear_rank_aliases.h"

/// The rank-one maps a search is allowed to recombine.
///
/// Both searches need this and neither owns it, which is why it is not in
/// either of their files.
namespace bilinear_rank {

/// The rank-one maps making up `slices`: one multiplication each, and exactly
/// as many as the slices cost.
std::vector<Matrix> rank_one_candidates(const Field& field, const std::vector<Matrix>& slices);

/// Nonzero vectors whose leading nonzero entry is 1: exactly one per scalar
/// class, `(p^length − 1)/(p−1)` of them.
///
/// The pool below is the grid of outer products of two of these lists, and
/// [its orbits](../orbit_reduction/pool_orbits.h) are computed on the lists rather than on the
/// grid, so they are worth having by name.
std::vector<std::vector<int64_t>> normalised_vectors(const Field& field, std::size_t length);

/// Every rank-one map of the given shape, one per scalar class.
///
/// There are `(p^rows - 1)(p^columns - 1) / (p-1)^2` of them: 961 for 5×5 over
/// GF(2), 4732 for 3×6 over GF(3). Built as outer products of vectors
/// normalised to leading entry 1; the original grew tuples of vectors and
/// tested the rank of every partial result to reach the same set.
std::vector<Matrix> all_rank_one_maps(const Field& field, std::size_t rows, std::size_t columns);

/// Whether the row space of `inner` sits inside that of `outer`.
///
/// This is the original's `auto_verify`, which asks whether some `z` has
/// `z · x = y` by comparing `rank(x)` with `rank([xᵀ | yᵀ])`. Left
/// multiplication cannot leave the row space, so that comparison is exactly
/// this containment.
bool row_space_contains(const Field& field, const Matrix& outer, const Matrix& inner);

/// Whether any of `known` has a row space containing `candidate`'s. The
/// original's `auto_sort`.
bool has_equivalent(const Field& field, const std::vector<Matrix>& known, const Matrix& candidate);

/// One representative per row-space class of a pool of rank-one maps.
///
/// The write-up's conclusion names shrinking `G` by automorphisms as the way
/// forward, and this is how far the original got. **It is not wired into either
/// search**, and should not be until the rest of the argument exists: replacing
/// a candidate by a representative of its class only preserves the answer if
/// the same automorphism is applied to the map being decomposed. Measuring the
/// reduction is useful on its own, and
/// [`tests/test_candidate_pool.cpp`](tests/test_candidate_pool.cpp) reports it.
std::vector<Matrix> one_per_row_space(const Field& field, const std::vector<Matrix>& pool);

}  // namespace bilinear_rank
