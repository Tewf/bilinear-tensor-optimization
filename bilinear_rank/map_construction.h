#pragma once

#include <cstddef>
#include <vector>

#include "types.h"

/// Building the bilinear maps the search then decomposes.
///
/// Two are in the original: multiplying two polynomials, and multiplying two
/// elements of GF(pⁿ), which is the first reduced modulo an irreducible
/// polynomial. The second was reachable only through an interactive prompt and
/// was lost when the fixtures were first written out by hand.
namespace bilinear_rank {

/// Coefficients highest degree first, the way the original's prompt asked for
/// them: `1 0 3` is `x² + 3`.
using Polynomial = std::vector<Element>;

/// The tensor of multiplying a polynomial of `left_terms` coefficients by one
/// of `right_terms`.
///
/// Slice `i` carries a 1 at `(j, k)` exactly when `j + k == i`, and there are
/// `left_terms + right_terms - 1` of them, in ascending degree. Computing every
/// product separately costs `left_terms * right_terms`, which is the rank to
/// beat.
std::vector<Matrix> polynomial_multiplication_tensor(std::size_t left_terms,
                                                     std::size_t right_terms);

/// Whether `modulus` is irreducible over GF(p), so `GF(p)[x]/(modulus)` is a
/// field.
///
/// The original tested this with sympy's `factor_list`, its only real use of
/// sympy; Givaro's polynomial domain does it without the dependency.
bool is_irreducible(const Field& field, const Polynomial& modulus);

/// Euclidean division of a polynomial whose coefficients are matrices by one
/// whose coefficients are scalars, keeping the remainder.
///
/// `slices` are in descending degree here, matching how `modulus` is written.
/// Zero coefficients are dropped from the result, so it is shorter than what
/// went in.
std::vector<Matrix> reduce_tensor_modulo(const Field& field, std::vector<Matrix> slices,
                                         const Polynomial& modulus);

/// The tensor of multiplication in GF(pⁿ), for `modulus` of degree `n`:
/// multiply two elements as polynomials, then reduce.
///
/// The original built the polynomial product from `len(modulus)` terms rather
/// than `deg(modulus)`, which multiplies operands that are not reduced. This
/// uses the degree, so both operands are field elements.
std::vector<Matrix> field_multiplication_tensor(const Field& field, const Polynomial& modulus);

}  // namespace bilinear_rank
