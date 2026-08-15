/// Step 1 of Table 1: the greedy smallest basis, and the ranks it reaches.
#include <chrono>
#include <iostream>
#include <string>

#include "check.h"
#include "search.h"
#include "tensor.h"

namespace {

struct Expectation {
    const char* name;
    long long naive;
    long long after_step_1;
};

constexpr Expectation kExpectations[] = {
    {"f2_5x5", 25, 16},
    {"f2_3x8", 24, 19},
    {"f2_4x7", 28, 19},
    {"f3_3x6", 18, 12},
};

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: test_step1 <fixtures-directory>\n";
        return 2;
    }
    const std::string directory = argv[1];

    for (const Expectation& expected : kExpectations) {
        const std::string name = expected.name;
        const exact::Tensor tensor = exact::read_tensor_file(directory + "/" + name + ".tensor");
        const exact::Field field(tensor.characteristic);

        const auto started = std::chrono::steady_clock::now();
        const std::vector<exact::Matrix> basis = rank_search::smallest_basis(field, tensor.slices);
        const double seconds =
            std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();

        std::cout << name << "  (" << seconds << " s)\n";
        // A basis of the same space: same size, and it still spans the original.
        check::equal(name + " basis size", static_cast<long long>(basis.size()),
                     static_cast<long long>(tensor.slices.size()));
        for (const exact::Matrix& slice : tensor.slices) {
            if (exact::raises_rank(field, basis, slice)) {
                std::cout << "  FAIL  " << name << " basis does not span the original map\n";
                return 1;
            }
        }
        check::equal(name + " naive multiplications",
                     static_cast<long long>(exact::multiplication_count(field, tensor.slices)),
                     expected.naive);
        check::equal(name + " after step 1",
                     static_cast<long long>(exact::multiplication_count(field, basis)),
                     expected.after_step_1);
    }

    return check::report("step 1");
}
