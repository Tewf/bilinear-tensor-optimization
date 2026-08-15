/// Building maps, and deciding two whose answers are classical.
///
/// The bilinear complexity of multiplication in GF(4) over GF(2) is 3, and in
/// GF(8) it is 6. Both are known independently of anything here, so they check
/// the construction and the exact search at once.
#include <iostream>
#include <string>

#include "candidate_pool.h"
#include "check.h"
#include "exhaustive_search.h"
#include "map_construction.h"

namespace {

void check_irreducibility(const bilinear_rank::Field& field) {
    // Over GF(2): x^2+x+1 and x^3+x+1 are irreducible, x^2+1 = (x+1)^2 is not.
    check::equal("x^2+x+1 irreducible over F2",
                 bilinear_rank::is_irreducible(field, {1, 1, 1}) ? 1 : 0, 1);
    check::equal("x^2+1 reducible over F2",
                 bilinear_rank::is_irreducible(field, {1, 0, 1}) ? 1 : 0, 0);
    check::equal("x^3+x+1 irreducible over F2",
                 bilinear_rank::is_irreducible(field, {1, 0, 1, 1}) ? 1 : 0, 1);
}

void check_polynomial_tensor() {
    const std::vector<bilinear_rank::Matrix> tensor =
        bilinear_rank::polynomial_multiplication_tensor(5, 5);
    check::equal("5x5 polynomial tensor slices", static_cast<long long>(tensor.size()), 9);
    const bilinear_rank::Field field(2);
    check::equal("5x5 polynomial tensor naive cost",
                 static_cast<long long>(linear_algebra::multiplication_count(field, tensor)), 25);
}

/// GF(4) multiplication: c1 = a0b1 + a1b0 + a1b1, c0 = a0b0 + a1b1, because
/// x^2 = x + 1.
void check_gf4_tensor(const bilinear_rank::Field& field) {
    const std::vector<bilinear_rank::Matrix> tensor =
        bilinear_rank::field_multiplication_tensor(field, {1, 1, 1});
    check::equal("GF(4) tensor slices", static_cast<long long>(tensor.size()), 2);
    if (tensor.size() != 2) return;

    const int64_t expected_x[2][2] = {{0, 1}, {1, 1}};
    const int64_t expected_1[2][2] = {{1, 0}, {0, 1}};
    for (std::size_t row = 0; row < 2; ++row) {
        for (std::size_t column = 0; column < 2; ++column) {
            check::equal("GF(4) x coefficient", tensor[0](row, column), expected_x[row][column]);
            check::equal("GF(4) constant coefficient", tensor[1](row, column),
                         expected_1[row][column]);
        }
    }
    check::equal("GF(4) naive cost",
                 static_cast<long long>(linear_algebra::multiplication_count(field, tensor)), 4);
}

/// The exact search must reach the classical value from nothing.
void check_field_complexity(const bilinear_rank::Field& field,
                            const bilinear_rank::Polynomial& modulus, const std::string& what,
                            long long expected) {
    const std::vector<bilinear_rank::Matrix> tensor =
        bilinear_rank::field_multiplication_tensor(field, modulus);
    const std::vector<bilinear_rank::Matrix> pool =
        bilinear_rank::all_rank_one_maps(field, tensor.front().rows(), tensor.front().columns());

    bilinear_rank::SearchBudget budget;
    std::vector<bilinear_rank::Matrix> products;
    if (!bilinear_rank::fewest_products_by_sweep(field, tensor, pool, budget, products)) {
        std::cout << "  FAIL  " << what << ": no decomposition found\n";
        ++check::failure_count;
        return;
    }
    check::equal(what + " bilinear complexity", static_cast<long long>(products.size()), expected);
}

}  // namespace

int main() {
    const bilinear_rank::Field over_two(2);

    check_irreducibility(over_two);
    check_polynomial_tensor();
    check_gf4_tensor(over_two);
    check_field_complexity(over_two, {1, 1, 1}, "GF(4) over GF(2)", 3);
    check_field_complexity(over_two, {1, 1, 0, 1}, "GF(8) over GF(2)", 6);

    return check::report("map construction");
}
