/// The candidate pools, and how far the automorphism idea actually shrinks one.
#include <iostream>

#include "candidate_pool.h"
#include "check.h"

int main() {
    struct Shape {
        int64_t characteristic;
        std::size_t rows;
        std::size_t columns;
        long long pool;
        long long reduced;
    };
    // |G| = (p^rows - 1)(p^columns - 1) / (p-1)^2, and grouping by row space
    // leaves one per right-vector direction, (p^columns - 1)/(p-1).
    constexpr Shape kShapes[] = {
        {2, 5, 5, 961, 31}, {2, 3, 8, 1785, 255}, {2, 4, 7, 1905, 127}, {3, 3, 6, 4732, 364}};

    for (const Shape& shape : kShapes) {
        const bilinear_rank::Field field(shape.characteristic);
        const std::vector<bilinear_rank::Matrix> pool =
            bilinear_rank::all_rank_one_maps(field, shape.rows, shape.columns);
        check::equal("pool size", static_cast<long long>(pool.size()), shape.pool);

        for (const bilinear_rank::Matrix& map : pool) {
            if (linear_algebra::rank(field, map) != 1) {
                std::cout << "  FAIL  a pool element is not rank one\n";
                ++check::failure_count;
                break;
            }
        }

        const std::vector<bilinear_rank::Matrix> representatives =
            bilinear_rank::one_per_row_space(field, pool);
        check::equal("one per row space", static_cast<long long>(representatives.size()),
                     shape.reduced);
    }

    return check::report("candidate pool");
}
