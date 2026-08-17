#pragma once

#include <cstddef>
#include <vector>

#include "automorphism.h"
#include "bilinear_rank_aliases.h"

/// Naming a subspace so that two of them can be compared, and so that a whole
/// orbit of them has one name.
///
/// An enumerator that wants to visit one subspace per orbit needs to recognise the
/// same subspace twice, and a subspace has no canonical basis until one is chosen.
/// `SpanBasis` already keeps its rows in reduced row echelon form, so the only
/// thing missing is a fixed row order; sorting by pivot column supplies it, and the
/// result is unique to the subspace rather than to the order its generators arrived
/// in.
///
/// The **canonical code** goes one step further and is unique to the whole orbit:
/// the least code over every element of the group. That costs one pass over the
/// group per call, which is why this only works where the group can be held in a
/// list. `matrix_multiplication_symmetries` refuses above a million elements, and
/// `⟨3,3,3⟩` at 4 741 632 is past it, so this is a tool for the shapes where the
/// group is small, said here rather than discovered later.
namespace bilinear_rank {

/// A subspace's name. `Element` is `int64_t`, so codes compare lexicographically
/// with no conversion.
using SubspaceCode = std::vector<Element>;

/// The reduced basis of `span(generators)`, rows ordered by pivot column, laid end
/// to end. Equal codes mean equal subspaces.
SubspaceCode subspace_code(const Field& field, const std::vector<Matrix>& generators);

/// The least `subspace_code` over the orbit of `span(generators)` under `group`,
/// and every group element that attains it.
///
/// The attaining set is returned because the canonical augmentation test needs it:
/// choosing a distinguished pool element of a subspace means minimising over
/// exactly those elements that already carry the subspace to its canonical form.
/// An empty `group` makes this `subspace_code` with the identity, which is the
/// honest degenerate case rather than an error.
struct CanonicalSubspace {
    SubspaceCode code;
    std::vector<std::size_t> attaining;  // indices into `group`
};

CanonicalSubspace canonical_subspace(const Field& field,
                                     const std::vector<Automorphism>& group,
                                     const std::vector<Matrix>& generators);

/// The matrix `sigma` sends `form` to, flattened, so a distinguished pool element
/// can be compared under a group element rather than in place.
SubspaceCode image_code(const Field& field, const Automorphism& sigma, const Matrix& form);

}  // namespace bilinear_rank
