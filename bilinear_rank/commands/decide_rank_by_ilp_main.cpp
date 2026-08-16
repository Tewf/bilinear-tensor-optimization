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
#include "requested_group.h"
#include "solver_chain.h"
#include "symmetry_argument.h"
#include "tensor_file.h"
#include "timing.h"

namespace {

void usage() {
    std::cerr << "usage: decide-rank-by-ilp <tensor-file> --target k\n"
                 "                          [-s|--symmetry none|auto|matmul <n> <m> <k>]\n"
                 "                          [--solver gurobi|cbc|glpk|lp_solve|built-in]\n"
                 "                          [--node-limit N]  bounds the built-in only\n"
                 "\n"
                 "GF(2) only: over a larger field a product of operands is not a\n"
                 "conjunction, and that is a different encoding rather than a bigger one.\n";
}

/// Refuted and gave-up are different answers, and collapsing them is how a
/// budget running out becomes a lower bound nobody proved.
enum class Verdict { Found, Refuted, Undecided };

/// Ask one programme, and check any point it returns against the map.
Verdict ask(const bilinear_rank::Field& field, const std::vector<bilinear_rank::Matrix>& slices,
            const bilinear_rank::EncodedRank& encoded, bool chosen, optimisation::Backend backend,
            std::size_t node_limit, std::size_t& products) {
    const optimisation::Solution solution =
        chosen ? optimisation::solve_with(backend, encoded.programme, node_limit)
               : optimisation::solve(encoded.programme, node_limit);

    if (solution.status == optimisation::Status::Infeasible) return Verdict::Refuted;
    if (solution.status != optimisation::Status::Optimal) return Verdict::Undecided;

    const std::vector<bilinear_rank::Matrix> found =
        bilinear_rank::products_of(field, encoded, solution.values);
    bilinear_rank::Algorithm algorithm;
    if (!bilinear_rank::recovers_map(field, slices, found, algorithm)) {
        std::cerr << "# a solution came back that does not compute the map; discarded\n";
        return Verdict::Undecided;
    }
    products = found.size();
    return Verdict::Found;
}

/// One first term per orbit, as the two vectors whose outer product it is.
///
/// `matmul` has them in closed form; `auto` takes them from the map's own
/// stabiliser and factors each representative back into its operands with
/// `recover_operands`, which is the same routine the searches use.
std::vector<std::pair<std::vector<bilinear_rank::Element>, std::vector<bilinear_rank::Element>>>
first_terms(const bilinear_rank::Field& field, const linear_algebra::Tensor& tensor,
            const cli::Symmetry& symmetry) {
    if (symmetry.kind == cli::SymmetryKind::MatrixMultiplication) {
        return bilinear_rank::matrix_multiplication_orbit_vectors(
            field, symmetry.shape[0], symmetry.shape[1], symmetry.shape[2]);
    }

    const std::vector<bilinear_rank::Automorphism> stabiliser = bilinear_rank::stabiliser_of(
        field, tensor.slices,
        bilinear_rank::requested_ambient_group(field, tensor.slices, symmetry));
    const std::vector<bilinear_rank::Matrix> representatives =
        bilinear_rank::rank_one_orbit_representatives(field, stabiliser, tensor.rows(),
                                                      tensor.columns());

    bilinear_rank::Matrix left;
    bilinear_rank::Matrix right;
    if (!bilinear_rank::recover_operands(field, representatives, left, right)) {
        throw std::runtime_error("an orbit representative was not rank one");
    }

    std::vector<std::pair<std::vector<bilinear_rank::Element>, std::vector<bilinear_rank::Element>>>
        terms;
    for (std::size_t index = 0; index < representatives.size(); ++index) {
        terms.push_back({left.row(index), right.row(index)});
    }
    return terms;
}

int run(int argc, char** argv) {
    if (argc < 2) {
        usage();
        return 2;
    }

    std::size_t target = 0;
    std::size_t node_limit = 200000;
    cli::Symmetry symmetry;
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
                std::cerr << "# unknown solver name\n";
                return 2;
            }
        } else if (flag == "--symmetry" || flag == "-s") {
            symmetry = cli::parse_symmetry(argc, argv, argument);
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
    std::cerr << "# " << argv[1] << ": " << tensor.slices.size() << " slices of " << tensor.rows()
              << "x" << tensor.columns() << " over GF(" << tensor.characteristic << ")\n";

    const cli::Clock::time_point started = cli::Clock::now();
    std::size_t products = 0;

    if (symmetry.kind == cli::SymmetryKind::None) {
        const bilinear_rank::EncodedRank encoded =
            bilinear_rank::encode_rank_question(field, tensor.slices, target);
        std::cerr << "# one programme: " << encoded.programme.variables.size() << " variables, "
                  << encoded.programme.constraints.size() << " constraints\n";
        const Verdict verdict =
            ask(field, tensor.slices, encoded, chosen_backend, backend, node_limit, products);
        std::cout << "k = " << target << ": "
                  << (verdict == Verdict::Found     ? "FOUND"
                      : verdict == Verdict::Refuted ? "NO"
                                                    : "UNDECIDED")
                  << ", " << cli::elapsed_seconds(started) << " s\n";
        return verdict == Verdict::Found ? 0 : (verdict == Verdict::Refuted ? 1 : 3);
    }

    const auto representatives = first_terms(field, tensor, symmetry);
    std::cerr << "# " << representatives.size() << " orbits, one programme each\n";

    // A yes in any orbit is a yes. A no needs every orbit to refuse, and a single
    // orbit that gave up makes the whole answer undecided rather than no: that is
    // the difference between a lower bound and a lower bound nobody proved.
    bool any_undecided = false;
    for (std::size_t orbit = 0; orbit < representatives.size(); ++orbit) {
        bilinear_rank::EncodedRank encoded =
            bilinear_rank::encode_rank_question(field, tensor.slices, target);
        bilinear_rank::fix_first_term(encoded, representatives[orbit].first,
                                      representatives[orbit].second);
        const Verdict verdict =
            ask(field, tensor.slices, encoded, chosen_backend, backend, node_limit, products);
        if (verdict == Verdict::Found) {
            std::cout << "k = " << target << ": FOUND in orbit " << orbit << " of "
                      << representatives.size() << ", " << cli::elapsed_seconds(started) << " s\n";
            return 0;
        }
        if (verdict == Verdict::Undecided) any_undecided = true;
    }

    std::cout << "k = " << target << ": " << (any_undecided ? "UNDECIDED" : "NO") << ", "
              << representatives.size() << " orbits, " << cli::elapsed_seconds(started) << " s\n";
    return any_undecided ? 3 : 1;
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
