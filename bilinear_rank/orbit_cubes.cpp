#include "orbit_cubes.h"

#include <stdexcept>

#include "pool_orbits.h"

namespace bilinear_rank {

std::vector<std::vector<int>> orbit_cubes(const Field& field, std::size_t rows, std::size_t inner,
                                          std::size_t columns,
                                          const std::vector<int>& left_variables,
                                          const std::vector<int>& right_variables) {
    if (field.characteristic() != 2) {
        // Over larger fields a rank-one map is a whole scalar class of operand
        // pairs, so pinning one pair would forbid the others and refuse
        // solutions that exist. The representatives are still right; only this
        // way of writing them down is not.
        throw std::runtime_error("orbit cubes are written for GF(2)");
    }
    if (left_variables.size() != rows * inner || right_variables.size() != inner * columns) {
        throw std::runtime_error("the variables given do not match the shape of the product");
    }

    std::vector<std::vector<int>> cubes;
    for (const auto& [left, right] :
         matrix_multiplication_orbit_vectors(field, rows, inner, columns)) {
        std::vector<int> cube;
        cube.reserve(left.size() + right.size());
        for (std::size_t index = 0; index < left.size(); ++index) {
            cube.push_back(field.isZero(left[index]) ? -left_variables[index]
                                                     : left_variables[index]);
        }
        for (std::size_t index = 0; index < right.size(); ++index) {
            cube.push_back(field.isZero(right[index]) ? -right_variables[index]
                                                      : right_variables[index]);
        }
        cubes.push_back(std::move(cube));
    }
    return cubes;
}

}  // namespace bilinear_rank
