/// The closed form has to be right about the orbits, not merely produce the
/// right number of them.
///
/// A cube pins the first term of a decomposition. If the representatives missed
/// an orbit, every cube would refuse the decompositions living there and the
/// search would report a lower bound that is false, with nothing downstream able
/// to catch it. So: their orbits must be disjoint, and together they must be the
/// whole pool.
#include <array>
#include <iostream>
#include <numeric>
#include <string>

#include "automorphism.h"
#include "candidate_pool.h"
#include "check.h"
#include "group_construction.h"
#include "orbit_cubes.h"
#include "pool_orbits.h"

namespace {

void check_representatives_partition_the_pool(const bilinear_rank::Field& field, std::size_t rows,
                                              std::size_t inner, std::size_t columns) {
    const std::string what = "<" + std::to_string(rows) + "," + std::to_string(inner) + "," +
                             std::to_string(columns) + ">";
    const std::vector<bilinear_rank::Matrix> pool =
        bilinear_rank::all_rank_one_maps(field, rows * inner, inner * columns);
    const std::vector<bilinear_rank::Automorphism> generators =
        bilinear_rank::matrix_multiplication_symmetry_generators(field, rows, inner, columns);
    const std::vector<std::vector<std::uint32_t>> action =
        bilinear_rank::action_on(field, generators, pool);

    // Where each pool element sits, so a representative can be looked up.
    std::vector<std::uint32_t> everything(pool.size());
    std::iota(everything.begin(), everything.end(), std::uint32_t(0));
    const std::vector<std::uint32_t> computed = bilinear_rank::one_per_orbit(action, everything);

    const std::vector<bilinear_rank::Matrix> closed =
        bilinear_rank::matrix_multiplication_pool_orbits(field, rows, inner, columns);
    check::equal(what + " closed form counts the orbits",
                 static_cast<long long>(closed.size()),
                 static_cast<long long>(computed.size()));

    // Every representative's orbit, marked. Disjoint means no element is marked
    // twice; covering means none is left unmarked.
    std::vector<int> marked_by(pool.size(), -1);
    for (std::size_t index = 0; index < closed.size(); ++index) {
        std::size_t start = pool.size();
        for (std::size_t position = 0; position < pool.size(); ++position) {
            bool same = true;
            for (std::size_t entry = 0; entry < pool[position].entry_count() && same; ++entry) {
                same = field.areEqual(pool[position].data()[entry], closed[index].data()[entry]);
            }
            if (same) { start = position; break; }
        }
        if (start == pool.size()) {
            std::cout << "  FAIL  " << what << ": a representative is not in the pool\n";
            ++check::failure_count;
            return;
        }

        std::vector<std::uint32_t> frontier{static_cast<std::uint32_t>(start)};
        marked_by[start] = static_cast<int>(index);
        while (!frontier.empty()) {
            const std::uint32_t reached = frontier.back();
            frontier.pop_back();
            for (const std::vector<std::uint32_t>& permutation : action) {
                const std::uint32_t image = permutation[reached];
                if (marked_by[image] == static_cast<int>(index)) continue;
                if (marked_by[image] != -1) {
                    std::cout << "  FAIL  " << what << ": two representatives share an orbit\n";
                    ++check::failure_count;
                    return;
                }
                marked_by[image] = static_cast<int>(index);
                frontier.push_back(image);
            }
        }
    }

    std::size_t uncovered = 0;
    for (const int owner : marked_by) {
        if (owner == -1) ++uncovered;
    }
    check::equal(what + " orbits cover the pool", static_cast<long long>(uncovered), 0);
}

/// A cube must pin every coordinate of both operands, once each.
void check_cubes_are_well_formed(const bilinear_rank::Field& field) {
    const std::size_t rows = 2, inner = 2, columns = 2;
    std::vector<int> left(rows * inner), right(inner * columns);
    std::iota(left.begin(), left.end(), 1);
    std::iota(right.begin(), right.end(), static_cast<int>(left.size()) + 1);

    const std::vector<std::vector<int>> cubes =
        bilinear_rank::orbit_cubes(field, rows, inner, columns, left, right);
    check::equal("<2,2,2> one cube per orbit", static_cast<long long>(cubes.size()), 5);
    for (const std::vector<int>& cube : cubes) {
        check::equal("a cube pins every coordinate", static_cast<long long>(cube.size()),
                     static_cast<long long>(left.size() + right.size()));
    }
}

}  // namespace

int main() {
    const bilinear_rank::Field over_two(2);
    check_representatives_partition_the_pool(over_two, 2, 2, 2);
    check_representatives_partition_the_pool(over_two, 2, 2, 3);
    check_cubes_are_well_formed(over_two);
    return check::report("orbit cubes");
}
