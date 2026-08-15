#pragma once

#include <cstddef>
#include <utility>
#include <vector>

#include "field.h"
#include "matrix.h"
#include "solver.h"

/// Breaking a matrix into rank-one pieces.
///
/// A rank-one bilinear form is one multiplication, so this is what turns a map
/// into the products that compute it, and it is where both strands meet the
/// thing they are actually counting.
namespace linear_algebra {

/// Write `matrix` as a sum of exactly rank(matrix) rank-one matrices.
template <class Field>
std::vector<MatrixOver<Field>> rank_one_decomposition(const Field& field,
                                                      const MatrixOver<Field>& matrix) {
    using Element = typename Field::Element;

    // A maximal independent set of rows, in order: their span is the row space,
    // so every row is a combination of them.
    std::vector<std::vector<Element>> basis_rows;
    for (std::size_t row = 0; row < matrix.rows(); ++row) {
        std::vector<Element> entries = matrix.row(row);
        std::vector<Element> unused;
        if (!solve_in_row_space(field, basis_rows, entries, unused)) {
            basis_rows.push_back(std::move(entries));
        }
    }

    // matrix == coefficients * basis_rows, so term j is column j of the
    // coefficients against basis row j: an outer product, hence rank one.
    std::vector<MatrixOver<Field>> terms(basis_rows.size(),
                                         MatrixOver<Field>(matrix.rows(), matrix.columns()));
    for (std::size_t row = 0; row < matrix.rows(); ++row) {
        std::vector<Element> coefficients;
        solve_in_row_space(field, basis_rows, matrix.row(row), coefficients);
        for (std::size_t term = 0; term < basis_rows.size(); ++term) {
            for (std::size_t column = 0; column < matrix.columns(); ++column) {
                field.axpyin(terms[term](row, column), coefficients[term],
                             basis_rows[term][column]);
            }
        }
    }
    return terms;
}

}  // namespace linear_algebra
