#include "linear_algebra.h"

#include <fflas-ffpack/fflas-ffpack.h>

#include "span_basis.h"

namespace exact {

std::size_t rank(const Field& field, const Matrix& matrix) {
    if (matrix.rows() == 0 || matrix.columns() == 0) return 0;
    Matrix scratch = matrix;  // FFPACK::Rank overwrites its input
    return FFPACK::Rank(field, matrix.rows(), matrix.columns(), scratch.data(), matrix.columns());
}

std::size_t multiplication_count(const Field& field, const std::vector<Matrix>& slices) {
    std::size_t total = 0;
    for (const Matrix& slice : slices) {
        total += rank(field, slice);
    }
    return total;
}

namespace {

/// Lay each matrix out flat as one row, so that linear algebra on the resulting
/// matrix is linear algebra on the matrices as vectors.
Matrix flatten(const std::vector<Matrix>& matrices) {
    if (matrices.empty()) return Matrix();
    Matrix flattened(matrices.size(), matrices.front().entry_count());
    for (std::size_t index = 0; index < matrices.size(); ++index) {
        const Matrix& matrix = matrices[index];
        for (std::size_t entry = 0; entry < matrix.entry_count(); ++entry) {
            flattened(index, entry) = matrix.data()[entry];
        }
    }
    return flattened;
}

}  // namespace

std::size_t rank_of_span(const Field& field, const std::vector<Matrix>& matrices) {
    return rank(field, flatten(matrices));
}

bool raises_rank(const Field& field, const std::vector<Matrix>& basis, const Matrix& candidate) {
    if (candidate.is_zero()) return false;
    if (basis.empty()) return true;

    std::vector<Matrix> extended = basis;
    extended.push_back(candidate);
    return rank_of_span(field, extended) > rank_of_span(field, basis);
}

bool spans_all(const Field& field, const std::vector<Matrix>& spanning_set,
               const std::vector<Matrix>& targets) {
    if (targets.empty()) return true;
    if (spanning_set.empty()) return false;
    SpanBasis span(field, spanning_set.front().entry_count());
    for (const Matrix& element : spanning_set) span.try_add(element);
    for (const Matrix& target : targets) {
        if (!span.contains(target)) return false;
    }
    return true;
}

bool solve_in_row_space(const Field& field, const std::vector<std::vector<int64_t>>& rows,
                        const std::vector<int64_t>& target, std::vector<int64_t>& coefficients) {
    const std::size_t unknowns = rows.size();
    if (unknowns == 0) {
        coefficients.clear();
        for (int64_t entry : target) {
            if (!field.isZero(entry)) return false;
        }
        return true;
    }
    const std::size_t equations = target.size();

    // Augmented system: column j is rows[j] read down the equations, and the
    // last column is the target.
    Matrix system(equations, unknowns + 1);
    for (std::size_t equation = 0; equation < equations; ++equation) {
        for (std::size_t unknown = 0; unknown < unknowns; ++unknown) {
            system(equation, unknown) = rows[unknown][equation];
        }
        system(equation, unknowns) = target[equation];
    }

    std::vector<std::size_t> pivot_of_unknown(unknowns, equations);  // equations == "none"
    std::size_t pivot_row = 0;
    for (std::size_t unknown = 0; unknown < unknowns && pivot_row < equations; ++unknown) {
        std::size_t found = equations;
        for (std::size_t row = pivot_row; row < equations; ++row) {
            if (!field.isZero(system(row, unknown))) {
                found = row;
                break;
            }
        }
        if (found == equations) continue;

        for (std::size_t column = 0; column <= unknowns; ++column) {
            std::swap(system(pivot_row, column), system(found, column));
        }

        int64_t scale;
        field.inv(scale, system(pivot_row, unknown));
        for (std::size_t column = 0; column <= unknowns; ++column) {
            field.mulin(system(pivot_row, column), scale);
        }

        for (std::size_t row = 0; row < equations; ++row) {
            if (row == pivot_row || field.isZero(system(row, unknown))) continue;
            int64_t factor;
            field.neg(factor, system(row, unknown));
            for (std::size_t column = 0; column <= unknowns; ++column) {
                field.axpyin(system(row, column), factor, system(pivot_row, column));
            }
        }
        pivot_of_unknown[unknown] = pivot_row;
        ++pivot_row;
    }

    // Any equation left reading 0 == nonzero means the target is outside the span.
    for (std::size_t row = pivot_row; row < equations; ++row) {
        if (!field.isZero(system(row, unknowns))) return false;
    }

    coefficients.assign(unknowns, 0);
    for (std::size_t unknown = 0; unknown < unknowns; ++unknown) {
        if (pivot_of_unknown[unknown] != equations) {
            coefficients[unknown] = system(pivot_of_unknown[unknown], unknowns);
        }
    }
    return true;
}

std::vector<Matrix> rank_one_decomposition(const Field& field, const Matrix& matrix) {
    // A maximal independent set of rows, taken in order. Their span is the row
    // space, so every row is a combination of them.
    std::vector<std::vector<int64_t>> basis_rows;
    std::vector<std::size_t> basis_row_indices;
    for (std::size_t row = 0; row < matrix.rows(); ++row) {
        std::vector<int64_t> entries(matrix.columns());
        for (std::size_t column = 0; column < matrix.columns(); ++column) {
            entries[column] = matrix(row, column);
        }
        std::vector<int64_t> unused;
        if (!solve_in_row_space(field, basis_rows, entries, unused)) {
            basis_rows.push_back(std::move(entries));
            basis_row_indices.push_back(row);
        }
    }

    // matrix == coefficients * basis_rows, so term j is column j of the
    // coefficients against basis row j: an outer product, hence rank one.
    std::vector<Matrix> terms(basis_rows.size(), Matrix(matrix.rows(), matrix.columns()));
    for (std::size_t row = 0; row < matrix.rows(); ++row) {
        std::vector<int64_t> entries(matrix.columns());
        for (std::size_t column = 0; column < matrix.columns(); ++column) {
            entries[column] = matrix(row, column);
        }
        std::vector<int64_t> coefficients;
        solve_in_row_space(field, basis_rows, entries, coefficients);
        for (std::size_t term = 0; term < basis_rows.size(); ++term) {
            for (std::size_t column = 0; column < matrix.columns(); ++column) {
                field.axpyin(terms[term](row, column), coefficients[term],
                             basis_rows[term][column]);
            }
        }
    }
    return terms;
}

}  // namespace exact
