#include "group_construction.h"

#include <stdexcept>

#include "measures.h"
#include "memory_budget.h"
#include "solver.h"

namespace bilinear_rank {

namespace {

/// `(A ⊗ B)[(i,j), (a,b)] = A[i][a] · B[j][b]`, the map that turns
/// `M ↦ A M Bᵀ` into a matrix acting on `M` read row by row.
Matrix kronecker(const Field& field, const Matrix& left, const Matrix& right) {
    Matrix result(left.rows() * right.rows(), left.columns() * right.columns());
    for (std::size_t outer_row = 0; outer_row < left.rows(); ++outer_row) {
        for (std::size_t outer_column = 0; outer_column < left.columns(); ++outer_column) {
            if (field.isZero(left(outer_row, outer_column))) continue;
            for (std::size_t inner_row = 0; inner_row < right.rows(); ++inner_row) {
                for (std::size_t inner_column = 0; inner_column < right.columns(); ++inner_column) {
                    field.mul(result(outer_row * right.rows() + inner_row,
                                     outer_column * right.columns() + inner_column),
                              left(outer_row, outer_column), right(inner_row, inner_column));
                }
            }
        }
    }
    return result;
}

/// The matrix at `index`, counting in base `p` through every entry.
Matrix matrix_at(const Field& field, std::size_t order, std::size_t index) {
    const auto characteristic = static_cast<std::size_t>(field.characteristic());
    Matrix result(order, order);
    for (std::size_t entry = 0; entry < result.entry_count(); ++entry) {
        result.data()[entry] = static_cast<Element>(index % characteristic);
        index /= characteristic;
    }
    return result;
}

/// How many matrices of that order there are, refusing a walk too long to make.
std::size_t matrices_of_order(const Field& field, std::size_t order) {
    const auto characteristic = static_cast<std::size_t>(field.characteristic());
    const std::size_t ceiling = 1u << 20;

    std::size_t count = 1;
    for (std::size_t entry = 0; entry < order * order; ++entry) {
        if (count > ceiling / characteristic) {
            throw std::runtime_error("enumerating every " + std::to_string(order) + "x" +
                                     std::to_string(order) + " matrix over GF(" +
                                     std::to_string(characteristic) +
                                     ") is more than a million of them; the group has to come from "
                                     "a closed form at this size");
        }
        count *= characteristic;
    }
    return count;
}

/// `(Y⁻¹)ᵀ`, the half of the action that undoes a change of coordinates on the
/// shared index.
Matrix inverse_transpose(const Field& field, const Matrix& square) {
    Matrix inverse;
    if (!linear_algebra::invert(field, square, inverse)) {
        throw std::runtime_error("a matrix listed as invertible was not");
    }
    return linear_algebra::transpose<Field>(inverse);
}

}  // namespace

std::vector<Matrix> all_invertible(const Field& field, std::size_t order) {
    const std::size_t candidates = matrices_of_order(field, order);

    std::vector<Matrix> invertible;
    for (std::size_t index = 0; index < candidates; ++index) {
        Matrix candidate = matrix_at(field, order, index);
        if (linear_algebra::rank(field, candidate) == order) invertible.push_back(std::move(candidate));
    }
    return invertible;
}

std::vector<Automorphism> all_automorphisms(const Field& field, std::size_t rows,
                                            std::size_t columns) {
    const std::vector<Matrix> lefts = all_invertible(field, rows);
    const std::vector<Matrix> rights = all_invertible(field, columns);

    require_room("every automorphism of that shape", lefts.size() * rights.size(),
                 bytes_per_matrix(rows * rows) + bytes_per_matrix(columns * columns));

    std::vector<Automorphism> group;
    group.reserve(lefts.size() * rights.size());
    for (const Matrix& left : lefts) {
        for (const Matrix& right : rights) group.push_back({left, right});
    }
    return group;
}

std::vector<Automorphism> matrix_multiplication_symmetries(const Field& field, std::size_t rows,
                                                           std::size_t inner,
                                                           std::size_t columns) {
    const std::vector<Matrix> on_rows = all_invertible(field, rows);
    const std::vector<Matrix> on_inner = all_invertible(field, inner);
    const std::vector<Matrix> on_columns = all_invertible(field, columns);

    const std::size_t count = on_rows.size() * on_inner.size() * on_columns.size();
    require_room("the symmetries of that matrix product", count,
                 bytes_per_matrix(rows * inner * rows * inner) +
                     bytes_per_matrix(inner * columns * inner * columns));

    std::vector<Automorphism> group;
    group.reserve(count);
    for (const Matrix& x : on_rows) {
        for (const Matrix& y : on_inner) {
            const Matrix y_inverse_transpose = inverse_transpose(field, y);
            for (const Matrix& z : on_columns) {
                group.push_back({kronecker(field, x, y_inverse_transpose),
                                 kronecker(field, y, inverse_transpose(field, z))});
            }
        }
    }
    return group;
}

}  // namespace bilinear_rank
