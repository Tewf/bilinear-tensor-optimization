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
///
/// Which encoding states the question is [`rank_question.h`](../rank_question.h);
/// this parses arguments and walks the range.
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

#include "rank_question.h"
#include "types.h"
#include "size_argument.h"
#include "tensor_file.h"
#include "tensor_flattening.h"

namespace {

void usage() {
    std::cerr << "usage: decide-rank-by-sat <tensor-file>\n"
                 "       decide-rank-by-sat <tensor-file> --target k\n"
                 "       decide-rank-by-sat <tensor-file> --from a --to b\n"
                 "\n"
                 "  With no range at all it finds the rank: it sweeps upward from the\n"
                 "  flattening lower bound to the naive upper bound, and the first k it\n"
                 "  can decompose into is the rank, since every smaller one was refused.\n"
                 "\n"
                 "  --emit-cnf <path>   write the question and stop, for any solver\n"
                 "  --plain-cnf         expand parities into clauses\n"
                 "  --break-symmetry    quotient by term order, and by operand scaling over\n"
                 "                      GF(p). Sound, off by default, and worth at least 76x\n"
                 "                      on a question expected to answer no\n"
                 "  --backend cnf|smt   cnf encodes the field into clauses (default); smt\n"
                 "                      hands GF(p) to cvc5's theory of finite fields\n"
                 "  --solver <name>     pin a SAT solver instead of taking the best fit\n"
                 "  --proof <path>      write a DRAT refutation when the answer is no,\n"
                 "                      so the lower bound can be checked independently\n"
                 "  --ceiling N         override the naive upper bound the search starts from\n"
                 "  --probe N           smaller budget for the questions a search asks on\n"
                 "                      the way, so the full timeout is spent once\n"
                 "  --timeout N         seconds per question, 300 by default\n"
                 "  --max-memory 2G     cap on the solver\n";
}

/// What a sweep has established so far.
struct Progress {
    bool found = false;
    /// True while every `k` below the current one came back a definite no. A
    /// single unknown breaks it, and the answer is then a bound rather than a
    /// determination: the decomposition may have been in the part nobody
    /// finished.
    bool all_below_refused = true;
};

/// Ask one `k` and say what came back. True when the sweep should stop.
bool report(const linear_algebra::Tensor& tensor, std::size_t products,
            const satisfiability::Approach& approach, const std::string& emit_to,
            Progress& progress) {
    if (!emit_to.empty()) {
        std::cout << "  k = " << products << ": wrote " << emit_to << ", "
                  << satisfiability::write_question(tensor, products, approach, emit_to) << "\n";
        return false;
    }

    const satisfiability::Answer answer = satisfiability::decide_rank(tensor, products, approach);
    std::cout << "  k = " << products << " [" << answer.solver_name << "]: ";
    switch (answer.verdict) {
        case satisfiability::Verdict::Yes:
            std::cout << "FOUND a decomposition into " << products << "  (" << answer.seconds
                      << " s)\n";
            return true;
        case satisfiability::Verdict::No:
            std::cout << "NO, rank is more than " << products << "  (" << answer.seconds << " s)";
            if (answer.proof == satisfiability::Proof::Verified) {
                std::cout << ", refutation verified";
            } else if (answer.proof_bytes > 0) {
                std::cout << ", refutation " << answer.proof_bytes << " bytes, unchecked";
            }
            std::cout << "\n";
            return false;
        case satisfiability::Verdict::Unknown:
            std::cout << "no answer, gave up after " << answer.seconds << " s\n";
            progress.all_below_refused = false;
            return false;
    }
    return false;
}

int run(int argc, char** argv) {
    if (argc < 2) {
        usage();
        return 2;
    }

    const std::string path = argv[1];
    satisfiability::Approach approach;
    long long target = -1;
    long long from = -1;
    long long to = -1;
    long long given_ceiling = -1;
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
        } else if (option == "--backend" && argument + 1 < argc) {
            approach.use_field_theory = (std::string(argv[++argument]) == "smt");
        } else if (option == "--solver" && argument + 1 < argc) {
            approach.solver = argv[++argument];
        } else if (option == "--ceiling" && argument + 1 < argc) {
            given_ceiling = std::stoll(argv[++argument]);
        } else if (option == "--probe" && argument + 1 < argc) {
            approach.probe_seconds = static_cast<std::size_t>(std::stoull(argv[++argument]));
        } else if (option == "--proof" && argument + 1 < argc) {
            approach.proof_path = argv[++argument];
        } else if (option == "--timeout" && argument + 1 < argc) {
            approach.timeout_seconds = static_cast<std::size_t>(std::stoull(argv[++argument]));
        } else if (option == "--max-memory" && argument + 1 < argc) {
            approach.memory_megabytes = cli::parse_size(argv[++argument]) / (1024 * 1024);
        } else if (option == "--plain-cnf") {
            approach.plain_cnf = true;
        } else if (option == "--break-symmetry") {
            approach.break_symmetry = true;
        } else {
            usage();
            return 2;
        }
    }

    const linear_algebra::Tensor tensor = linear_algebra::read_tensor_file(path);
    std::cout << path << ": " << tensor.slices.size() << " slices of " << tensor.rows() << "x"
              << tensor.columns() << " over GF(" << tensor.characteristic << ")\n";

    // The flattening bound costs one Gaussian elimination and rules out every
    // rank below it. Asking a solver to refute those is pure waste, and this
    // tool was doing exactly that whenever a sweep started from one.
    const satisfiability::Field field(tensor.characteristic);
    const std::size_t floor =
        linear_algebra::flattening_lower_bound(field, tensor.slices);
    std::cout << "  flattening lower bound: rank is at least " << floor << "\n";

    // Rank is at most the smallest product of two of the three dimensions:
    // hold one axis fixed and take that many rank-one terms.
    const std::size_t rows = tensor.rows();
    const std::size_t columns = tensor.columns();
    const std::size_t slices = tensor.slices.size();
    std::size_t ceiling = std::min(rows * columns, std::min(rows * slices, columns * slices));
    if (given_ceiling > 0) ceiling = static_cast<std::size_t>(given_ceiling);

    if (target >= 0) from = to = target;
    if (from < 0) from = static_cast<long long>(floor);
    if (static_cast<std::size_t>(from) < floor && target < 0) {
        std::cout << "  starting at " << floor << " rather than " << from
                  << ", which the flattenings already refute\n";
        from = static_cast<long long>(floor);
    }
    // No range asked for: find the rank, galloping down from the ceiling and
    // bisecting, which spends its questions on the cheap side.
    if (to < 0 && target < 0 && emit_to.empty()) {
        std::cout << "  naive upper bound: rank is at most " << ceiling << "\n";
        const auto bounds =
            satisfiability::find_rank(tensor, approach, floor, ceiling);
        std::cout << "  asked " << bounds.questions_asked << " questions in " << bounds.seconds
                  << " s\n";
        if (bounds.exact) {
            std::cout << "rank is exactly " << bounds.upper << "\n";
        } else {
            std::cout << "rank is between " << bounds.lower << " and " << bounds.upper
                      << ", and a question went unanswered\n";
        }
        return 0;
    }
    if (to < 0) to = static_cast<long long>(ceiling);
    if (to < from) to = from;

    // Whether the sweep began where the flattenings say it must. Starting
    // higher than that leaves ranks untested, so a first success is only a
    // bound rather than the rank.
    Progress progress;
    progress.all_below_refused = static_cast<std::size_t>(from) <= floor;

    for (long long products = from; products <= to; ++products) {
        if (report(tensor, static_cast<std::size_t>(products), approach, emit_to, progress)) {
            if (progress.all_below_refused) {
                std::cout << "rank is exactly " << products
                          << ", since every smaller one was refused\n";
            } else {
                std::cout << "rank is at most " << products << "\n";
            }
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
