/// Sparsify the operator of a fast multiplication algorithm and report what
/// each method achieved.
///
/// The original's entry point asked, at a prompt, for the matrix and then for
/// which algorithm to run; one of the two answers ran the other algorithm.
#include <chrono>
#include <iostream>
#include <string>

#include "rational_matrix_io.h"
#include "sparsify.h"

namespace {

double seconds_since(std::chrono::steady_clock::time_point started) {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
}

/// Report a result only once it is known to be the same operator. Sparsity is
/// trivial to improve by returning something else entirely.
void report(const sparsify::Field& field, const std::string& method,
            const sparsify::Matrix& original, const sparsify::Matrix& sparsified,
            double seconds, bool show_matrix) {
    const bool equivalent = exact::same_row_space(field, exact::transpose<sparsify::Field>(original),
                                                  exact::transpose<sparsify::Field>(sparsified));
    std::cout << "  " << method << ": " << exact::nonzero_count(field, sparsified)
              << " nonzeros, " << seconds << " s"
              << (equivalent ? "" : "   *** NOT THE SAME OPERATOR ***") << "\n";
    if (show_matrix) std::cout << exact::to_string(sparsified);
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: sparsify <matrix-file> [--show]\n";
        return 2;
    }
    const std::string path = argv[1];
    const bool show_matrix = (argc > 2 && std::string(argv[2]) == "--show");

    const sparsify::Field field;
    const sparsify::Matrix operator_matrix = exact::read_rational_matrix_file(path);
    const sparsify::Matrix transposed = exact::transpose<sparsify::Field>(operator_matrix);

    std::cout << path << "\n  as given: " << exact::nonzero_count(field, operator_matrix)
              << " nonzeros, " << operator_matrix.rows() << "x" << operator_matrix.columns()
              << "\n";

    auto started = std::chrono::steady_clock::now();
    const sparsify::Matrix sparsifier = sparsify::row_basis_sparsifier(field, operator_matrix);
    report(field, "row-basis heuristic",
           operator_matrix, exact::multiply(field, operator_matrix, sparsifier),
           seconds_since(started), show_matrix);

    started = std::chrono::steady_clock::now();
    const sparsify::Matrix exhaustive = sparsify::sparsify_exhaustive(field, transposed);
    report(field, "exact oracle, bottom-up", operator_matrix,
           exact::transpose<sparsify::Field>(exhaustive), seconds_since(started), show_matrix);

    started = std::chrono::steady_clock::now();
    const sparsify::Matrix top_down = sparsify::sparsify_top_down(field, transposed);
    report(field, "exact oracle, top-down", operator_matrix,
           exact::transpose<sparsify::Field>(top_down), seconds_since(started), show_matrix);

    return 0;
}
