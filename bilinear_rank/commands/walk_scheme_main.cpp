/// Move a decomposition rather than build one: the flip graph, over any GF(p).
///
/// Every other command here assembles an algorithm out of a pool of rank-one
/// maps. This one starts from the naive algorithm, which always exists, and
/// walks it. It proves nothing: each seed's result is checked against the map it
/// must compute before its count is printed, and a count from an unchecked
/// scheme is never printed at all.
#include <iostream>
#include <stdexcept>
#include <string>

#include "algorithm_recovery.h"
#include "candidate_pool.h"
#include "fewest_products.h"
#include "flip_graph.h"
#include "tensor_file.h"
#include "timing.h"

namespace {

void usage() {
    std::cerr << "usage: walk-scheme <tensor-file> [--steps N] [--seeds N] [--from k]\n"
                 "\n"
                 "  --steps N   flips per seed, 20000 by default\n"
                 "  --seeds N   independent walks, 8 by default; each is reproducible\n"
                 "              from its own seed number\n";
}

/// Does this scheme still compute the map it started from? Asked of every
/// result, because a scheme that does not is not a cheaper algorithm.
bool computes(const bilinear_rank::Field& field,
              const std::vector<bilinear_rank::Matrix>& target,
              const bilinear_rank::Scheme& scheme) {
    const bilinear_rank::Algorithm algorithm = bilinear_rank::algorithm_of(scheme);
    if (algorithm.product_count() == 0) return false;
    bilinear_rank::Algorithm recovered;
    return bilinear_rank::recovers_map(
        field, target, bilinear_rank::encoded_products(field, algorithm.left, algorithm.right),
        recovered);
}

int run(int argc, char** argv) {
    if (argc < 2) {
        usage();
        return 2;
    }

    std::size_t steps = 20000;
    std::size_t seeds = 8;
    for (int argument = 2; argument < argc; ++argument) {
        const std::string flag = argv[argument];
        if (flag == "--steps" && argument + 1 < argc) {
            steps = std::stoul(argv[++argument]);
        } else if (flag == "--seeds" && argument + 1 < argc) {
            seeds = std::stoul(argv[++argument]);
        } else {
            usage();
            return 2;
        }
    }

    const linear_algebra::Tensor tensor = linear_algebra::read_tensor_file(argv[1]);
    const bilinear_rank::Field field(tensor.characteristic);

    bilinear_rank::Algorithm naive;
    if (!bilinear_rank::recovers_map(
            field, tensor.slices, bilinear_rank::rank_one_candidates(field, tensor.slices), naive)) {
        std::cerr << "no naive algorithm for this map, so nothing to walk from\n";
        return 1;
    }

    const bilinear_rank::Scheme start = bilinear_rank::scheme_of(naive);
    std::cout << "GF(" << tensor.characteristic << "), naive scheme: " << start.size()
              << " products\n";

    const cli::Clock::time_point started = cli::Clock::now();
    std::size_t best = start.size();
    for (std::size_t seed = 1; seed <= seeds; ++seed) {
        bilinear_rank::FlipReport report;
        const bilinear_rank::Scheme walked = bilinear_rank::walk(field, start, steps, seed, &report);
        if (walked.size() >= best) continue;
        if (!computes(field, tensor.slices, walked)) {
            std::cout << "  seed " << seed << ": " << walked.size()
                      << " products, DISCARDED, does not compute the map\n";
            continue;
        }
        best = walked.size();
        std::cout << "  seed " << seed << ": " << best << " products after " << report.flips
                  << " flips and " << report.reductions << " reductions, " << cli::elapsed_seconds(started)
                  << " s\n";
    }
    const std::size_t bound = bilinear_rank::starting_target(field, tensor.slices);
    std::cout << "best over " << seeds << " seeds: " << bilinear_rank::gap_report(best, bound)
              << "\n";
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        return run(argc, argv);
    } catch (const std::exception& failure) {
        std::cerr << failure.what() << "\n";
        return 1;
    }
}
