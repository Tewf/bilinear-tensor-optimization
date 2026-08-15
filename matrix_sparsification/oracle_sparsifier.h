#pragma once

#include <cstddef>
#include <vector>

#include "linear_algebra.h"

/// Matrix sparsification: given the operator U of a fast multiplication
/// algorithm, find an invertible V minimising nnz(U V).
///
/// Fewer nonzeros in the operator means fewer additions in the algorithm it
/// encodes, which is what the multiplication count does not capture. Everything
/// here works in exact rationals, because the entries are ninths and the
/// quantity being minimised is how many of them are zero.
namespace matrix_sparsification {

using Field = linear_algebra::RationalField;
using Matrix = linear_algebra::RationalMatrix;

/// All subsets of `{0, ..., total-1}` of the given size, in lexicographic order.
std::vector<std::vector<std::size_t>> subsets(std::size_t total, std::size_t size);

/// The heuristic of the first article: take a square block of rows of `u` that
/// is invertible and use its inverse.
///
/// It guarantees that as many rows of the result as `u` has columns are
/// singletons, and costs one pass per subset of rows. Returns the sparsifier V,
/// or the identity when nothing improves on doing nothing.
Matrix row_basis_sparsifier(const Field& field, const Matrix& u);

/// A vector in the row space of `rows`, zero on every column of `columns`, whose
/// support meets `settled` nowhere.
///
/// This is the article's Omega validator. It exists exactly when some row
/// outside `settled` is, on those columns, in the span of the others; the
/// coefficients that say so are the validator.
struct Validator {
    bool found = false;
    std::size_t replaces = 0;                        // the row this vector may take over
    std::vector<typename Field::Element> combination;  // coefficients over the rows
};

Validator find_validator(const Field& field, const Matrix& rows,
                         const std::vector<std::size_t>& columns,
                         const std::vector<std::size_t>& settled);

/// The exact oracle, bottom-up: consider every column subset of size one less
/// than the number of rows, and keep the validator whose vector has the most
/// zeros. Algorithms 2.3 and 3 of the original.
Matrix sparsify_exhaustive(const Field& field, Matrix rows);

/// The exact oracle, top-down: walk column subsets from the largest down, and
/// take the first validator found, because a vector forced to zero on more
/// columns cannot be beaten by one forced on fewer. Algorithms 2.4 and 4.
///
/// The original could not run this at all: both branches of the choice called
/// the bottom-up version, and the search returned None when it found nothing,
/// which its caller unpacked into two variables.
Matrix sparsify_top_down(const Field& field, Matrix rows);

}  // namespace matrix_sparsification
