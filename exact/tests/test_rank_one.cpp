/// Rank-one decomposition: as many terms as the rank, each of rank one, and
/// they add back up to what they came from.
#include <iostream>
#include <random>
#include <string>

#include "check.h"
#include "linear_algebra.h"
#include "tensor.h"

namespace {

void check_decomposition(const exact::Field& field, const exact::Matrix& matrix,
                         const std::string& what) {
    const std::size_t expected_terms = exact::rank(field, matrix);
    const std::vector<exact::Matrix> terms = exact::rank_one_decomposition(field, matrix);

    if (terms.size() != expected_terms) {
        std::cout << "  FAIL  " << what << ": " << terms.size() << " terms for rank "
                  << expected_terms << "\n";
        ++check::failure_count;
        return;
    }

    exact::Matrix total(matrix.rows(), matrix.columns());
    for (const exact::Matrix& term : terms) {
        if (exact::rank(field, term) != 1) {
            std::cout << "  FAIL  " << what << ": a term has rank "
                      << exact::rank(field, term) << ", not 1\n";
            ++check::failure_count;
            return;
        }
        for (std::size_t entry = 0; entry < total.entry_count(); ++entry) {
            field.addin(total.data()[entry], term.data()[entry]);
        }
    }
    for (std::size_t entry = 0; entry < total.entry_count(); ++entry) {
        if (total.data()[entry] != matrix.data()[entry]) {
            std::cout << "  FAIL  " << what << ": terms do not sum back to the matrix\n";
            ++check::failure_count;
            return;
        }
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: test_rank_one <fixtures-directory>\n";
        return 2;
    }
    const std::string directory = argv[1];

    for (const char* name : {"f2_5x5", "f2_3x8", "f2_4x7", "f3_3x6"}) {
        const exact::Tensor tensor = exact::read_tensor_file(directory + "/" + std::string(name) +
                                                             ".tensor");
        const exact::Field field(tensor.characteristic);
        for (std::size_t index = 0; index < tensor.slices.size(); ++index) {
            check_decomposition(field, tensor.slices[index],
                                std::string(name) + " slice " + std::to_string(index));
        }
    }
    check::equal("fixture slices decomposed", check::failure_count, 0);

    // The fixture slices are sparse and structured. Dense matrices over larger
    // primes exercise the arithmetic the structured ones never reach.
    std::mt19937 generator(20240521);
    for (int64_t characteristic : {2, 3, 5, 7, 13}) {
        const exact::Field field(characteristic);
        std::uniform_int_distribution<int64_t> entries(0, characteristic - 1);
        for (int trial = 0; trial < 40; ++trial) {
            const std::size_t rows = 1 + generator() % 6;
            const std::size_t columns = 1 + generator() % 6;
            exact::Matrix matrix(rows, columns);
            for (std::size_t entry = 0; entry < matrix.entry_count(); ++entry) {
                matrix.data()[entry] = entries(generator);
            }
            check_decomposition(field, matrix,
                                "random GF(" + std::to_string(characteristic) + ") trial " +
                                    std::to_string(trial));
        }
    }
    check::equal("random matrices decomposed", check::failure_count, 0);

    return check::report("rank-one decomposition");
}
