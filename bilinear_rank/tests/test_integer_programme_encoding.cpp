/// The rank question as an integer programme: that a solver's answer decodes
/// back into a decomposition that actually computes the map, and that "no" means
/// no.
#include <iostream>
#include <string>
#include <vector>

#include "algorithm_recovery.h"
#include "check.h"
#include "integer_programme_encoding.h"
#include "solver_chain.h"
#include "tensor_file.h"

namespace {

using bilinear_rank::EncodedRank;
using bilinear_rank::Field;
using bilinear_rank::Matrix;

constexpr std::size_t kNodeLimit = 200000;

struct Answer {
    bool found = false;
    std::size_t products = 0;
    bool computes = false;
};

/// Ask the chain, then check the answer against the map rather than believe it.
Answer ask(const Field& field, const std::vector<Matrix>& slices, const EncodedRank& encoded) {
    const optimisation::Solution solution =
        optimisation::solve(encoded.programme, kNodeLimit);
    if (solution.status != optimisation::Status::Optimal) return {};

    const std::vector<Matrix> products = bilinear_rank::products_of(field, encoded, solution.values);
    bilinear_rank::Algorithm algorithm;
    return {true, products.size(),
            bilinear_rank::recovers_map(field, slices, products, algorithm)};
}

void check_map(const std::string& directory, const char* name, std::size_t rank, bool expected) {
    const linear_algebra::Tensor tensor =
        linear_algebra::read_tensor_file(directory + "/" + name + ".tensor");
    const Field field(tensor.characteristic);
    const EncodedRank encoded = bilinear_rank::encode_rank_question(field, tensor.slices, rank);

    const std::string label = std::string(name) + " at rank " + std::to_string(rank);
    std::cout << "  note  " << label << ": " << encoded.programme.variables.size()
              << " variables, " << encoded.programme.constraints.size() << " constraints\n";

    const Answer answer = ask(field, tensor.slices, encoded);
    check::equal(label + " is answered", answer.found, expected);
    // A point that does not compute the map is worse than no point at all, so
    // this is checked whenever one comes back.
    if (answer.found) {
        check::equal(label + " decodes to an algorithm computing the map", answer.computes, 1);
        check::equal(label + " uses no more terms than asked",
                     answer.products <= rank, 1);
    }
}

/// The orbit reduction, as this encoding takes it: pin the first term to one
/// representative. Every decomposition moves onto some representative, so a
/// question that is satisfiable stays satisfiable for at least one of them.
void check_orbit_restriction(const std::string& directory) {
    const linear_algebra::Tensor tensor =
        linear_algebra::read_tensor_file(directory + "/f2_2x2.tensor");
    const Field field(tensor.characteristic);

    std::size_t satisfiable = 0;
    std::size_t asked = 0;
    for (std::size_t row = 0; row < tensor.rows(); ++row) {
        for (std::size_t column = 0; column < tensor.columns(); ++column) {
            std::vector<bilinear_rank::Element> left(tensor.rows(), field.zero);
            std::vector<bilinear_rank::Element> right(tensor.columns(), field.zero);
            left[row] = field.one;
            right[column] = field.one;

            EncodedRank encoded = bilinear_rank::encode_rank_question(field, tensor.slices, 3);
            bilinear_rank::fix_first_term(encoded, left, right);
            ++asked;
            const Answer answer = ask(field, tensor.slices, encoded);
            if (!answer.found) continue;
            ++satisfiable;
            check::equal("pinned first term still computes the map", answer.computes, 1);
        }
    }
    std::cout << "  note  " << satisfiable << " of " << asked
              << " pinned first terms admit a rank-3 decomposition\n";
    check::equal("pinning does not rule out every first term", satisfiable > 0, 1);
}

void check_refusal(const std::string& directory) {
    const linear_algebra::Tensor tensor =
        linear_algebra::read_tensor_file(directory + "/f3_3x6.tensor");
    const Field field(tensor.characteristic);
    bool refused = false;
    try {
        bilinear_rank::encode_rank_question(field, tensor.slices, 12);
    } catch (const std::exception&) {
        refused = true;
    }
    check::equal("GF(3) is refused rather than encoded wrongly", refused, 1);
}

}  // namespace

int main(int argc, char** argv) {
    if (argc <= 1) {
        std::cout << "usage: test_integer_programme_encoding <fixtures>\n";
        return 1;
    }
    const std::string directory = argv[1];

    check_map(directory, "f2_2x2", 3, true);   // Karatsuba
    check_map(directory, "f2_2x2", 2, false);  // and no smaller
    check_orbit_restriction(directory);
    check_refusal(directory);
    return check::report("integer programme encoding");
}
