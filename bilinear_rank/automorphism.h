#pragma once

#include <vector>

#include "types.h"

/// The rank-preserving automorphism group a search may quotient by.
///
/// Covanov 2018 (arXiv:1705.07728) section 3, itself an unpublished improvement
/// by Barbulescu and Zimmermann to the framework [the exact
/// search](exhaustive_search.h) implements. In one line: an invertible change of
/// coordinates on each of the two operands preserves rank, so if it also maps
/// the target subspace to itself then it maps solutions to solutions, and the
/// search need visit only one per orbit.
///
/// **The group is the setwise stabiliser of the target subspace, not of the
/// map** (Covanov's Definition 13). That is the larger group, because remixing
/// the slices among themselves leaves their span untouched, and the size of the
/// group is exactly what the reduction is worth.
///
/// Where the groups come from is [`group_construction.h`](group_construction.h).
namespace bilinear_rank {

/// Covanov's Definition 7: the pair `(μ, ν)` acting on a bilinear form by
/// `(a, b) ↦ Φ(μa, νb)`.
///
/// On the matrix of that form it is `M ↦ μᵀ M ν`, since
/// `(μa)ᵀ M (νb) = aᵀ (μᵀ M ν) b`.
struct Automorphism {
    Matrix left;   // μ, on the left operand: rows x rows
    Matrix right;  // ν, on the right operand: columns x columns
};

/// `μᵀ M ν`.
Matrix act_on(const Field& field, const Automorphism& sigma, const Matrix& form);

/// The pair that does nothing.
Automorphism identity_automorphism(const Field& field, std::size_t rows, std::size_t columns);

/// Doing `first`, then `second`. Componentwise, because
/// `ν'ᵀ(μᵀ M ν)ν' = (μμ')ᵀ M (νν')`.
Automorphism compose(const Field& field, const Automorphism& first, const Automorphism& second);

/// Everything reachable from `generators` by composition.
///
/// Breadth first, deduplicated by the entries themselves. Refused past the
/// memory budget rather than attempted: a group is perfectly capable of being
/// larger than the machine, and `(GL_3)³` at 14 million is already too large to
/// hold this way.
std::vector<Automorphism> group_closure(const Field& field,
                                        const std::vector<Automorphism>& generators);

/// The elements of `group` that map `span(slices)` into itself.
std::vector<Automorphism> stabiliser_of(const Field& field, const std::vector<Matrix>& slices,
                                        const std::vector<Automorphism>& group);

}  // namespace bilinear_rank
