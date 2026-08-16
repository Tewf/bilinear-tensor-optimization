/// Decide the rank by asking a solver, which is what NP-completeness is for.
///
/// `decide-rank` enumerates subspaces and is exponential in a quantity it
/// increases. This states "is there a decomposition into k rank-one terms" once
/// and hands it to a program built for questions of that shape. Same question,
/// different machinery, and the two are expected to agree wherever both finish.
///
/// Sweeping `k` upward from a lower bound gives the rank itself; a single
/// `--target` answers one question, and an unsatisfiable answer at `k` is a
/// proof that the rank exceeds `k`, provided the solver finished.
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "binary_encoding.h"
#include "dimacs_file.h"
#include "size_argument.h"
#include "solver_process.h"
#include "tensor_file.h"
#include "timing.h"

namespace {

void usage() {
    std::cerr << "usage: decide-rank-by-sat <tensor-file> --target k\n"
                 "       decide-rank-by-sat <tensor-file> --from a --to b\n"
                 "\n"
                 "  --emit-cnf <path>   write the formula and stop, for any solver\n"
                 "  --plain-cnf         expand parities into clauses, for a solver\n"
                 "                      without native XOR\n"
                 "  --break-symmetry    order the terms; sound, and off by default\n"
                 "  --timeout N         seconds per question, 300 by default\n"
                 "  --max-memory 2G     cap on the solver\n";
}

/// One question: is there a decomposition into `products` terms?
///
/// Returns 1 for yes, 0 for no, and -1 for "the solver did not say", which is
/// a third answer and never folded into no.
int decide(const linear_algebra::Tensor& tensor, std::size_t products, bool plain_cnf,
           bool break_symmetry, std::size_t megabytes, std::size_t timeout,
           const std::string& emit_to) {
    const auto encoding = satisfiability::encode_rank_at_most(tensor, products, break_symmetry);

    if (!emit_to.empty()) {
        std::ofstream out(emit_to);
        if (!out) throw std::runtime_error("cannot write " + emit_to);
        linear_algebra::write_dimacs(out, encoding.formula, !plain_cnf);
        std::cout << "  k = " << products << ": wrote " << emit_to << ", "
                  << encoding.formula.variable_count << " variables, "
                  << encoding.formula.total_clause_count(!plain_cnf) << " clauses\n";
        return -1;
    }

    const auto started = cli::Clock::now();
    const auto run = satisfiability::solve(encoding.formula, !plain_cnf, megabytes, timeout);
    if (!run.solver_found) {
        throw std::runtime_error(
            "no solver on PATH. Install cryptominisat, or use --emit-cnf and run your own");
    }

    std::cout << "  k = " << products << ": ";
    if (!run.answered) {
        std::cout << "no answer in " << timeout << " s\n";
        return -1;
    }
    if (!run.satisfiable) {
        std::cout << "NO, rank is more than " << products << "  ("
                  << cli::elapsed_seconds(started) << " s)\n";
        return 0;
    }

    // A solver is a large program and this check is cheap, so its yes is
    // verified against the tensor rather than believed.
    const satisfiability::Field field(2);
    const bool rebuilt =
        satisfiability::model_reconstructs(field, tensor, encoding, run.model);
    std::cout << "FOUND a decomposition into " << products << "  ("
              << cli::elapsed_seconds(started) << " s)"
              << (rebuilt ? "" : "  -- BUT IT DOES NOT REBUILD THE TENSOR") << "\n";
    if (!rebuilt) throw std::runtime_error("the model does not reconstruct the tensor");
    return 1;
}

int run(int argc, char** argv) {
    if (argc < 2) {
        usage();
        return 2;
    }

    const std::string path = argv[1];
    long long target = -1;
    long long from = -1;
    long long to = -1;
    bool plain_cnf = false;
    bool break_symmetry = false;
    std::size_t timeout = 300;
    std::size_t megabytes = 2048;
    std::string emit_to;

    for (int argument = 2; argument < argc; ++argument) {
        const std::string option = argv[argument];
        if (option == "--target" && argument + 1 < argc) {
            target = std::stoll(argv[++argument]);
        } else if (option == "--from" && argument + 1 < argc) {
            from = std::stoll(argv[++argument]);
        } else if (option == "--to" && argument + 1 < argc) {
            to = std::stoll(argv[++argument]);
        } else if (option == "--emit-cnf" && argument + 1 < argc) {
            emit_to = argv[++argument];
        } else if (option == "--timeout" && argument + 1 < argc) {
            timeout = static_cast<std::size_t>(std::stoull(argv[++argument]));
        } else if (option == "--max-memory" && argument + 1 < argc) {
            megabytes = cli::parse_size(argv[++argument]) / (1024 * 1024);
        } else if (option == "--plain-cnf") {
            plain_cnf = true;
        } else if (option == "--break-symmetry") {
            break_symmetry = true;
        } else {
            usage();
            return 2;
        }
    }

    const linear_algebra::Tensor tensor = linear_algebra::read_tensor_file(path);
    std::cout << path << ": " << tensor.slices.size() << " slices of " << tensor.rows() << "x"
              << tensor.columns() << " over GF(" << tensor.characteristic << ")\n";

    if (target >= 0) {
        from = target;
        to = target;
    }
    if (from < 0) {
        usage();
        return 2;
    }
    if (to < from) to = from;

    for (long long products = from; products <= to; ++products) {
        const int verdict = decide(tensor, static_cast<std::size_t>(products), plain_cnf,
                                   break_symmetry, megabytes, timeout, emit_to);
        if (verdict == 1) {
            std::cout << "rank is at most " << products << "\n";
            return 0;
        }
    }
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        return run(argc, argv);
    } catch (const std::exception& problem) {
        std::cerr << "decide-rank-by-sat: " << problem.what() << "\n";
        return 1;
    }
}
