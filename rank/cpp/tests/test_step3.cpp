/// Step 3: minimisation over every rank-one map of the shape, not just the ones
/// already inside the map.
///
/// One fixture per invocation, because these are the expensive runs and a
/// failure should say which map it was.
#include <chrono>
#include <iostream>
#include <string>

#include "check.h"
#include "search.h"
#include "tensor.h"

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "usage: test_step3 <fixtures-directory> <name> <expected-multiplications>\n";
        return 2;
    }
    const std::string directory = argv[1];
    const std::string name = argv[2];
    const long long expected = std::stoll(argv[3]);

    const exact::Tensor tensor = exact::read_tensor_file(directory + "/" + name + ".tensor");
    const exact::Field field(tensor.characteristic);

    const auto started = std::chrono::steady_clock::now();

    std::vector<exact::Matrix> current = rank_search::smallest_basis(field, tensor.slices);
    current = rank_search::minimise_rank(
        field, current, rank_search::improving_candidates(
                            field, current, rank_search::rank_one_candidates(field, current)));

    const std::vector<exact::Matrix> pool =
        rank_search::all_rank_one_maps(field, tensor.rows(), tensor.columns());
    current = rank_search::minimise_rank(
        field, current, rank_search::improving_candidates(field, current, pool));

    const double seconds =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
    std::cout << name << ": pool of " << pool.size() << " rank-one maps, " << seconds << " s\n";

    check::equal(name + " after step 3",
                 static_cast<long long>(exact::multiplication_count(field, current)), expected);

    if (!exact::spans_all(field, current, tensor.slices)) {
        std::cout << "  FAIL  " << name << ": the result no longer generates the map\n";
        ++check::failure_count;
    }

    return check::report("step 3 " + name);
}
