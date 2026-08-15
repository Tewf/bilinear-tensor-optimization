#pragma once

#include <cstddef>
#include <vector>

#include "field.h"
#include "matrix.h"
#include "span_basis.h"

/// What an object costs, in the two currencies this repository counts:
/// multiplications, and the nonzero entries that become additions.
namespace linear_algebra {

/// Exact rank: the number of independent rows.
///
/// The original computed this with a hand-written elimination that reduced
/// modulo p only after finishing, which overflows a machine integer and leaves
/// negative residues, and then decided invertibility from the result. On the
/// rational side it used a floating-point SVD tolerance.
template <class Field>
std::size_t rank(const Field& field, const MatrixOver<Field>& matrix) {
    if (matrix.rows() == 0 || matrix.columns() == 0) return 0;
    SpanBasis<Field> span(field, matrix.columns());
    for (std::size_t row = 0; row < matrix.rows(); ++row) span.try_add(matrix.row(row));
    return span.dimension();
}

/// The number of multiplications a set of slices costs, which is what the rank
/// search exists to reduce: the sum of their ranks.
template <class Field>
std::size_t multiplication_count(const Field& field,
                                 const std::vector<MatrixOver<Field>>& slices) {
    std::size_t total = 0;
    for (const MatrixOver<Field>& slice : slices) total += rank(field, slice);
    return total;
}

/// How many entries are not zero, which is what the sparsification search
/// exists to reduce.
///
/// Worth being exact about where the original went wrong here, because it is
/// not where it looks. Its reported count ran every entry through
/// `Fraction(x).limit_denominator()`, which snaps 1.85e-18 to zero, so the
/// number it printed was right. The search objective inside its algorithm 3 did
/// not: it counted zeros with `== 0` on the raw doubles, and on the operator it
/// was run against that sees 86 zeros where 144 exist. The count was sound and
/// the thing being maximised was not.
template <class Field>
std::size_t nonzero_count(const Field& field, const MatrixOver<Field>& matrix) {
    std::size_t total = 0;
    for (std::size_t entry = 0; entry < matrix.entry_count(); ++entry) {
        if (!field.isZero(matrix.data()[entry])) ++total;
    }
    return total;
}

}  // namespace linear_algebra
