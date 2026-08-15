/// Steps 1 and 2 of Table 1, and the property the table does not state: that
/// what comes out still generates the map that went in.
#include <chrono>
#include <iostream>
#include <string>

#include "check.h"
#include "search.h"
#include "span_basis.h"
#include "tensor.h"

namespace {

struct Expectation {
    const char* name;
    long long naive;
    long long after_step_1;
    long long after_step_2;
};

constexpr Expectation kExpectations[] = {
    {"f2_5x5", 25, 16, 14},
    {"f2_3x8", 24, 19, 16},
    {"f2_4x7", 28, 19, 16},
    {"f3_3x6", 18, 12, 11},
};

double seconds_since(std::chrono::steady_clock::time_point started) {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
}

/// The whole point of a rewrite: it must still compute the original map.
bool generates(const exact::Field& field, const std::vector<exact::Matrix>& rewritten,
               const std::vector<exact::Matrix>& original) {
    if (rewritten.empty()) return original.empty();
    exact::SpanBasis span(field, rewritten.front().entry_count());
    for (const exact::Matrix& slice : rewritten) span.try_add(slice);
    for (const exact::Matrix& slice : original) {
        if (!span.contains(slice)) return false;
    }
    return true;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: test_pipeline <fixtures-directory>\n";
        return 2;
    }
    const std::string directory = argv[1];

    for (const Expectation& expected : kExpectations) {
        const std::string name = expected.name;
        const exact::Tensor tensor = exact::read_tensor_file(directory + "/" + name + ".tensor");
        const exact::Field field(tensor.characteristic);
        std::cout << name << "\n";

        check::equal(name + " naive", static_cast<long long>(
                                          exact::multiplication_count(field, tensor.slices)),
                     expected.naive);

        auto started = std::chrono::steady_clock::now();
        const std::vector<exact::Matrix> step_1 = rank_search::smallest_basis(field, tensor.slices);
        const double step_1_seconds = seconds_since(started);

        check::equal(name + " after step 1",
                     static_cast<long long>(exact::multiplication_count(field, step_1)),
                     expected.after_step_1);

        started = std::chrono::steady_clock::now();
        const std::vector<exact::Matrix> own_products =
            rank_search::rank_one_candidates(field, step_1);
        const std::vector<exact::Matrix> shortlist =
            rank_search::improving_candidates(field, step_1, own_products);
        const std::vector<exact::Matrix> step_2 =
            rank_search::minimise_rank(field, step_1, shortlist);
        const double step_2_seconds = step_1_seconds + seconds_since(started);

        check::equal(name + " after step 2",
                     static_cast<long long>(exact::multiplication_count(field, step_2)),
                     expected.after_step_2);

        if (!generates(field, step_1, tensor.slices) || !generates(field, step_2, tensor.slices)) {
            std::cout << "  FAIL  " << name << ": the rewrite no longer generates the map\n";
            ++check::failure_count;
        }

        std::cout << "        step 1 " << step_1_seconds << " s, step 2 " << step_2_seconds
                  << " s cumulative, " << step_1.size() << " then " << step_2.size()
                  << " slices\n";
    }

    return check::report("pipeline");
}
