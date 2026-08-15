/// Write out a bilinear map as a tensor file.
///
/// The original could only build these behind an interactive prompt, so the
/// GF(pⁿ) maps it supported were lost the moment anyone wrote the fixtures out
/// by hand.
#include <iostream>
#include <string>
#include <vector>

#include "map_construction.h"
#include "tensor_file.h"

namespace {

void usage() {
    std::cerr << "usage: make-tensor --polynomial <p> <left-terms> <right-terms>\n"
                 "       make-tensor --field <p> <modulus coefficients, highest degree first>\n"
                 "\n"
                 "  --polynomial 2 5 5     multiplying two 5-term polynomials over GF(2)\n"
                 "  --field 2 1 1 1        multiplying in GF(2^2), modulus x^2 + x + 1\n"
                 "\n"
                 "Writes a tensor file on standard output.\n";
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 3) {
        usage();
        return 2;
    }
    const std::string mode = argv[1];
    const int64_t characteristic = std::stoll(argv[2]);
    const bilinear_rank::Field field(characteristic);

    std::vector<bilinear_rank::Matrix> slices;
    std::string description;

    try {
        if (mode == "--polynomial" && argc == 5) {
            const auto left = static_cast<std::size_t>(std::stoull(argv[3]));
            const auto right = static_cast<std::size_t>(std::stoull(argv[4]));
            slices = bilinear_rank::polynomial_multiplication_tensor(left, right);
            description = "Polynomial multiplication of " + std::string(argv[3]) +
                          " coefficients by " + argv[4] + ", over GF(" + argv[2] + ").";
        } else if (mode == "--field" && argc > 4) {
            bilinear_rank::Polynomial modulus;
            for (int argument = 3; argument < argc; ++argument) {
                modulus.push_back(std::stoll(argv[argument]));
            }
            slices = bilinear_rank::field_multiplication_tensor(field, modulus);
            description = "Multiplication in GF(" + std::string(argv[2]) + "^" +
                          std::to_string(modulus.size() - 1) + "), modulus given highest degree" +
                          " first as the original's prompt asked.";
        } else {
            usage();
            return 2;
        }
    } catch (const std::exception& problem) {
        std::cerr << "make-tensor: " << problem.what() << "\n";
        return 1;
    }

    if (slices.empty()) {
        std::cerr << "make-tensor: that produced no slices\n";
        return 1;
    }

    std::cout << "# " << description << "\n";
    std::cout << "# Naive cost " << linear_algebra::multiplication_count(field, slices)
              << " multiplications, written by make-tensor.\n";
    std::cout << "field " << characteristic << "\n";
    std::cout << "shape " << slices.size() << " " << slices.front().rows() << " "
              << slices.front().columns() << "\n";
    for (const bilinear_rank::Matrix& slice : slices) {
        std::cout << "\n";
        for (std::size_t row = 0; row < slice.rows(); ++row) {
            for (std::size_t column = 0; column < slice.columns(); ++column) {
                std::cout << (column == 0 ? "" : " ") << slice(row, column);
            }
            std::cout << "\n";
        }
    }
    return 0;
}
