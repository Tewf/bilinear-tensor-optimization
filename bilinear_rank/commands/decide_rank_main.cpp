/// Decide, rather than improve: is there an algorithm with exactly k products?
///
/// The heuristic in `minimise-rank` answers "can this be made better". This
/// answers "is there one this small", and a negative answer means no such
/// algorithm exists, provided the search ran to exhaustion, which is why the
/// node budget is reported on every line.
#include <iostream>
#include <string>

#include "algorithm_recovery.h"
#include "candidate_pool.h"
#include "dense_matrix_file.h"
#include "exhaustive_search.h"
#include "fewest_products.h"
#include "minimise_rank.h"
#include "smallest_basis.h"
#include "tensor_file.h"
#include "timing.h"

namespace {

void usage() {
    std::cerr << "usage: decide-rank <tensor-file> [--target k] [--anchor map|heuristic]\n"
                 "                   [--node-limit N] [--bottom-up]\n"
                 "\n"
                 "  --anchor map        search from the map itself (default): the answer is\n"
                 "                      the true minimum, and the search is exponential\n"
                 "  --anchor heuristic  run the heuristic first and search from its result:\n"
                 "                      far cheaper, but the answer is the minimum only\n"
                 "                      among algorithms containing that subspace\n";
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        usage();
        return 2;
    }

    const std::string path = argv[1];
    long long target = -1;
    bool anchor_on_heuristic = false;
    bool bottom_up = false;
    std::size_t node_limit = 5'000'000;

    for (int argument = 2; argument < argc; ++argument) {
        const std::string option = argv[argument];
        if (option == "--target" && argument + 1 < argc) {
            target = std::stoll(argv[++argument]);
        } else if (option == "--anchor" && argument + 1 < argc) {
            anchor_on_heuristic = (std::string(argv[++argument]) == "heuristic");
        } else if (option == "--node-limit" && argument + 1 < argc) {
            node_limit = static_cast<std::size_t>(std::stoull(argv[++argument]));
        } else if (option == "--bottom-up") {
            bottom_up = true;
        } else {
            usage();
            return 2;
        }
    }

    const linear_algebra::Tensor tensor = linear_algebra::read_tensor_file(path);
    const bilinear_rank::Field field(tensor.characteristic);

    std::vector<bilinear_rank::Matrix> anchor = tensor.slices;
    if (anchor_on_heuristic) {
        anchor = bilinear_rank::smallest_basis(field, tensor.slices);
        anchor = bilinear_rank::minimise_rank(
            field, anchor,
            bilinear_rank::improving_candidates(
                field, anchor, bilinear_rank::rank_one_candidates(field, anchor)));
        std::cout << "anchored on the heuristic: " << anchor.size() << " slices, "
                  << linear_algebra::multiplication_count(field, anchor) << " multiplications\n";
    }

    const std::vector<bilinear_rank::Matrix> pool =
        bilinear_rank::all_rank_one_maps(field, tensor.rows(), tensor.columns());
    std::cout << path << "\n  pool: " << pool.size() << " rank-one maps of shape " << tensor.rows()
              << "x" << tensor.columns() << "\n";

    bilinear_rank::SearchBudget budget{node_limit};
    std::vector<bilinear_rank::Matrix> products;
    const auto started = cli::Clock::now();

    bool found = false;
    if (bottom_up) {
        found = bilinear_rank::build_bottom_up(field, tensor.slices, pool, budget, products);
    } else if (target >= 0) {
        found = bilinear_rank::expand_subspace(field, anchor, pool, 0,
                                               static_cast<std::size_t>(target), budget, products);
    } else {
        found = bilinear_rank::fewest_products_by_sweep(field, anchor, pool, budget, products);
    }
    const double seconds = cli::elapsed_seconds(started);

    std::cout << "  " << budget.nodes_visited << " nodes in " << seconds << " s\n";

    if (found) {
        std::cout << "  FOUND: " << products.size() << " products\n";
        bilinear_rank::Algorithm algorithm;
        if (!bilinear_rank::recovers_map(field, tensor.slices, products, algorithm)) {
            std::cerr << "FAILED: those products do not compute the map\n";
            return 1;
        }
        std::cout << "  verified: they compute the map\n";
        return 0;
    }

    if (!budget.exhausted) {
        std::cout << "  GAVE UP: the node limit was reached, so nothing is decided.\n"
                     "           Raise --node-limit to search further.\n";
        return 2;
    }
    if (target >= 0) {
        std::cout << "  NO: there is no algorithm with " << target << " products"
                  << (anchor_on_heuristic ? " containing the heuristic's subspace" : "")
                  << ". The search was exhaustive.\n";
    } else {
        std::cout << "  NO decomposition found in the searched range.\n";
    }
    return 1;
}
