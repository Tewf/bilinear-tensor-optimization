/// Decide rank with a mixed integer programme, so the MILP solvers can be
/// compared against the SAT solvers and the tree search on the same question.
///
/// `--symmetry matmul n m k` asks the question once per orbit of the rank-one
/// pool with the first term pinned to that orbit's representative. Every
/// decomposition can be moved onto some representative, so the question is
/// answered completely: a yes from any one of them is a yes, and only a no from
/// all of them is a no.
#include <iostream>
#include <stdexcept>
#include <string>

#include "algorithm_recovery.h"
#include "integer_programme_encoding.h"
#include "pool_orbits.h"
#include "solver_chain.h"
#include "tensor_file.h"
#include "timing.h"

namespace {

void usage() {
    std::cerr << "usage: decide-rank-by-ilp <tensor-file> --target k\n"
                 "                          [--symmetry matmul <n> <m> <k>]  one programme per orbit\n"
                 "                          [--solver gurobi|cbc|glpk|lp_solve|built-in]\n"
                 "                          [--node-limit N]  bounds the built-in only\n"
                 "\n"
                 "GF(2) only: over a larger field a product of operands is not a\n"
                 "conjunction, and that is a different encoding rather than a bigger one.\n";
}

/// Is this programme satisfiable, and does its answer compute the map?
bool answered(const bilinear_rank::Field& field, const std::vector<bilinear_rank::Matrix>& slices,
              const bilinear_rank::EncodedRank& encoded, bool chosen, optimisation::Backend backend,
              std::size_t node_limit, std::size_t& products) {
    const optimisation::Solution solution =
        chosen ? optimisation::solve_with(backend, encoded.programme, node_limit)
               : optimisation::solve(encoded.programme, node_limit);
    if (solution.status != optimisation::Status::Optimal) return false;

    const std::vector<bilinear_rank::Matrix> found =
        bilinear_rank::products_of(field, encoded, solution.values);
    bilinear_rank::Algorithm algorithm;
    if (!bilinear_rank::recovers_map(field, slices, found, algorithm)) {
        std::cerr << "  a solution came back that does not compute the map; discarded\n";
        return false;
    }
    products = found.size();
    return true;
}

int run(int argc, char** argv) {
    if (argc < 2) {
        usage();
        return 2;
    }

    std::size_t target = 0;
    std::size_t node_limit = 200000;
    std::size_t shape[3] = {0, 0, 0};
    bool by_orbit = false;
    bool chosen_backend = false;
    optimisation::Backend backend = optimisation::Backend::BuiltIn;

    for (int argument = 2; argument < argc; ++argument) {
        const std::string flag = argv[argument];
        if (flag == "--target" && argument + 1 < argc) {
            target = std::stoul(argv[++argument]);
        } else if (flag == "--node-limit" && argument + 1 < argc) {
            node_limit = std::stoul(argv[++argument]);
        } else if (flag == "--solver" && argument + 1 < argc) {
            backend = optimisation::backend_named(argv[++argument], chosen_backend);
            if (!chosen_backend) {
                std::cerr << "unknown solver name\n";
                return 2;
            }
        } else if (flag == "--symmetry" && argument + 4 < argc &&
                   std::string(argv[argument + 1]) == "matmul") {
            by_orbit = true;
            for (int index = 0; index < 3; ++index) shape[index] = std::stoul(argv[argument + 2 + index]);
            argument += 4;
        } else {
            usage();
            return 2;
        }
    }
    if (target == 0) {
        usage();
        return 2;
    }

    const linear_algebra::Tensor tensor = linear_algebra::read_tensor_file(argv[1]);
    const bilinear_rank::Field field(tensor.characteristic);
    std::cout << argv[1] << ": " << tensor.slices.size() << " slices of " << tensor.rows() << "x"
              << tensor.columns() << " over GF(" << tensor.characteristic << ")\n";

    const cli::Clock::time_point started = cli::Clock::now();
    std::size_t products = 0;

    if (!by_orbit) {
        const bilinear_rank::EncodedRank encoded =
            bilinear_rank::encode_rank_question(field, tensor.slices, target);
        std::cout << "  one programme: " << encoded.programme.variables.size() << " variables, "
                  << encoded.programme.constraints.size() << " constraints\n";
        const bool yes =
            answered(field, tensor.slices, encoded, chosen_backend, backend, node_limit, products);
        std::cout << "  k = " << target << ": " << (yes ? "FOUND" : "no") << ", "
                  << cli::elapsed_seconds(started) << " s\n";
        return 0;
    }

    const auto representatives =
        bilinear_rank::matrix_multiplication_orbit_vectors(field, shape[0], shape[1], shape[2]);
    std::cout << "  " << representatives.size() << " orbits, one programme each\n";
    for (std::size_t orbit = 0; orbit < representatives.size(); ++orbit) {
        bilinear_rank::EncodedRank encoded =
            bilinear_rank::encode_rank_question(field, tensor.slices, target);
        bilinear_rank::fix_first_term(encoded, representatives[orbit].first,
                                      representatives[orbit].second);
        if (answered(field, tensor.slices, encoded, chosen_backend, backend, node_limit, products)) {
            std::cout << "  k = " << target << ": FOUND in orbit " << orbit << " of "
                      << representatives.size() << ", " << cli::elapsed_seconds(started) << " s\n";
            return 0;
        }
    }
    std::cout << "  k = " << target << ": no, all " << representatives.size()
              << " orbits refuted, " << cli::elapsed_seconds(started) << " s\n";
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
