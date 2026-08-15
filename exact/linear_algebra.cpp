#include "linear_algebra.h"

#include <fflas-ffpack/fflas-ffpack.h>

namespace exact {

std::size_t rank(const Field& field, const Matrix& matrix) {
    if (matrix.rows() == 0 || matrix.columns() == 0) return 0;
    Matrix scratch = matrix;  // FFPACK::Rank overwrites its input
    return FFPACK::Rank(field, matrix.rows(), matrix.columns(), scratch.data(), matrix.columns());
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

}  // namespace exact
