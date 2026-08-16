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
#include "field_theory_encoding.h"
#include "prime_field_encoding.h"
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
                 "  --break-symmetry    order the terms; GF(2) only. Sound, off by default,\n"
                 "                      and worth at least 76x on a question answering no\n"
                 "  --backend cnf|smt   cnf encodes the field into clauses (default); smt\n"
                 "                      hands GF(p) to cvc5's theory of finite fields\n"
                 "  --solver <name>     pin a SAT solver instead of taking the best fit\n"
                 "  --timeout N         seconds per question, 300 by default\n"
                 "  --max-memory 2G     cap on the solver\n";
}

/// One question: is there a decomposition into `products` terms?
///
/// Returns 1 for yes, 0 for no, and -1 for "the solver did not say", which is
/// a third answer and never folded into no.
int decide(const linear_algebra::Tensor& tensor, std::size_t products, bool plain_cnf,
           bool break_symmetry, bool use_smt, const std::string& solver_name,
           std::size_t megabytes, std::size_t timeout, const std::string& emit_to) {
    const satisfiability::Field field(tensor.characteristic);

    // The field theory route: no encoding at all, the solver already has GF(p).
    if (use_smt) {
        const auto encoding = satisfiability::encode_field_rank_at_most(tensor, products);
        if (!emit_to.empty()) {
            std::ofstream out(emit_to);
            if (!out) throw std::runtime_error("cannot write " + emit_to);
            linear_algebra::write_smtlib(out, encoding.problem);
            std::cout << "  k = " << products << ": wrote " << emit_to << ", "
                      << encoding.problem.constants.size() << " constants, "
                      << encoding.problem.assertions.size() << " assertions\n";
            return -1;
        }

        const auto run = satisfiability::solve_in_field(encoding.problem, megabytes, timeout);
        if (!run.solver_found) throw std::runtime_error("no cvc5 on PATH");

        std::cout << "  k = " << products << " [" << run.solver_name << "]: ";
        if (!run.answered) {
            std::cout << "no answer, gave up after " << run.seconds << " s\n";
            return -1;
        }
        if (!run.satisfiable) {
            std::cout << "NO, rank is more than " << products << "  (" << run.seconds << " s)\n";
            return 0;
        }
        const bool rebuilt =
            satisfiability::model_reconstructs(field, tensor, encoding, run.field_model);
        std::cout << "FOUND a decomposition into " << products << "  (" << run.seconds << " s)"
                  << (rebuilt ? "" : "  -- BUT IT DOES NOT REBUILD THE TENSOR") << "\n";
        if (!rebuilt) throw std::runtime_error("the model does not reconstruct the tensor");
        return 1;
    }

    // GF(2) gets the cheap encoding where a Boolean is a field element; any
    // larger prime gets the field built out of Booleans.
    const bool binary = tensor.characteristic == 2;
    satisfiability::BinaryEncoding boolean_form;
    satisfiability::PrimeFieldEncoding prime_form;
    if (binary) {
        boolean_form = satisfiability::encode_rank_at_most(tensor, products, break_symmetry);
    } else {
        if (break_symmetry) {
            throw std::runtime_error(
                "--break-symmetry is only implemented for GF(2); use --backend smt for GF(p)");
        }
        prime_form = satisfiability::encode_prime_rank_at_most(tensor, products);
    }
    const linear_algebra::Cnf& formula = binary ? boolean_form.formula : prime_form.formula;

    const satisfiability::SatSolver solver =
        satisfiability::find_sat_solver(!plain_cnf && !formula.parities.empty(), solver_name);
    const bool native = !emit_to.empty() ? !plain_cnf : solver.native_xor;

    if (!emit_to.empty()) {
        std::ofstream out(emit_to);
        if (!out) throw std::runtime_error("cannot write " + emit_to);
        linear_algebra::write_dimacs(out, formula, native);
        std::cout << "  k = " << products << ": wrote " << emit_to << ", "
                  << formula.total_variable_count(native) << " variables, "
                  << formula.total_clause_count(native) << " clauses\n";
        return -1;
    }

    const auto run = satisfiability::solve(formula, solver, megabytes, timeout);
    if (!run.solver_found) {
        throw std::runtime_error(
            "no SAT solver on PATH. Install cryptominisat or kissat, or use --emit-cnf");
    }

    std::cout << "  k = " << products << " [" << run.solver_name << "]: ";
    if (!run.answered) {
        // Elapsed, not the configured cap: the solver may also have been killed
        // from outside, and printing the cap would describe a wait that did not
        // happen. Either way this is not a no.
        std::cout << "no answer, gave up after " << run.seconds << " s\n";
        return -1;
    }
    if (!run.satisfiable) {
        std::cout << "NO, rank is more than " << products << "  (" << run.seconds << " s)\n";
        return 0;
    }

    // A solver is a large program and this check is cheap, so its yes is
    // verified against the tensor rather than believed.
    const bool rebuilt =
        binary ? satisfiability::model_reconstructs(field, tensor, boolean_form, run.model)
               : satisfiability::model_reconstructs(field, tensor, prime_form, run.model);
    std::cout << "FOUND a decomposition into " << products << "  (" << run.seconds << " s)"
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
    bool use_smt = false;
    std::string solver_name;
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
        } else if (option == "--backend" && argument + 1 < argc) {
            use_smt = (std::string(argv[++argument]) == "smt");
        } else if (option == "--solver" && argument + 1 < argc) {
            solver_name = argv[++argument];
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
                                   break_symmetry, use_smt, solver_name, megabytes, timeout,
                                   emit_to);
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
