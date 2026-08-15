/// Sparsify the operator of a fast multiplication algorithm and report what
/// each method achieved.
///
/// The original's entry point asked, at a prompt, for the matrix and then for
/// which algorithm to run; one of the two answers ran the other algorithm.
#include <chrono>
#include <iostream>
#include <string>

#include "dense_matrix_file.h"
#include "oracle_sparsifier.h"
#include "sms_file.h"

namespace {

double seconds_since(std::chrono::steady_clock::time_point started) {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
}

/// Report a result only once it is known to be the same operator. Sparsity is
/// trivial to improve by returning something else entirely.
void report(const matrix_sparsification::Field& field, const std::string& method,
            const matrix_sparsification::Matrix& original, const matrix_sparsification::Matrix& sparsified,
            double seconds, bool show_matrix) {
    const bool equivalent = linear_algebra::same_row_space(field, linear_algebra::transpose<matrix_sparsification::Field>(original),
                                                  linear_algebra::transpose<matrix_sparsification::Field>(sparsified));
    std::cout << "  " << method << ": " << linear_algebra::nonzero_count(field, sparsified)
              << " nonzeros, " << seconds << " s"
              << (equivalent ? "" : "   *** NOT THE SAME OPERATOR ***") << "\n";
    if (show_matrix) std::cout << linear_algebra::to_string(sparsified);
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: sparsify-operator <matrix-file|.sms> [--show]\n";
        return 2;
    }
    const std::string path = argv[1];
    const bool show_matrix = (argc > 2 && std::string(argv[2]) == "--show");

    const matrix_sparsification::Field field;
    // The original offered a choice of its own row-by-row format or SMS; the
    // extension says which, so nothing has to be answered at a prompt.
    const bool is_sms = path.size() > 4 && path.compare(path.size() - 4, 4, ".sms") == 0;
    const matrix_sparsification::Matrix operator_matrix =
        is_sms ? linear_algebra::read_sms_file(path)
               : linear_algebra::read_rational_matrix_file(path);
    const matrix_sparsification::Matrix transposed = linear_algebra::transpose<matrix_sparsification::Field>(operator_matrix);

    std::cout << path << "\n  as given: " << linear_algebra::nonzero_count(field, operator_matrix)
              << " nonzeros, " << operator_matrix.rows() << "x" << operator_matrix.columns()
              << "\n";

    auto started = std::chrono::steady_clock::now();
    const matrix_sparsification::Matrix sparsifier = matrix_sparsification::row_basis_sparsifier(field, operator_matrix);
    report(field, "row-basis heuristic",
           operator_matrix, linear_algebra::multiply(field, operator_matrix, sparsifier),
           seconds_since(started), show_matrix);

    started = std::chrono::steady_clock::now();
    const matrix_sparsification::Matrix exhaustive = matrix_sparsification::sparsify_exhaustive(field, transposed);
    report(field, "exact oracle, bottom-up", operator_matrix,
           linear_algebra::transpose<matrix_sparsification::Field>(exhaustive), seconds_since(started), show_matrix);

    started = std::chrono::steady_clock::now();
    const matrix_sparsification::Matrix top_down = matrix_sparsification::sparsify_top_down(field, transposed);
    report(field, "exact oracle, top-down", operator_matrix,
           linear_algebra::transpose<matrix_sparsification::Field>(top_down), seconds_since(started), show_matrix);

    return 0;
}
