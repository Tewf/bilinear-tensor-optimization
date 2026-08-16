#include "rank_question.h"

#include <fstream>
#include <stdexcept>

#include "binary_encoding.h"
#include "exit_code.h"
#include "field_theory_encoding.h"
#include "model_decomposition.h"
#include "prime_field_encoding.h"
#include "solver_process.h"

namespace satisfiability {

namespace {

/// Refuse the combinations that do not mean anything, by name.
void check_applicable(const linear_algebra::Tensor& tensor, const Approach& approach) {
    if (approach.break_symmetry && approach.use_field_theory) {
        throw std::invalid_argument("the field theory encoding has no ordering constraint");
    }
}

Answer from_field_theory(const linear_algebra::Tensor& tensor, std::size_t products,
                         const Approach& approach) {
    const auto encoding = encode_field_rank_at_most(tensor, products);
    const auto run =
        solve_in_field(encoding.problem, approach.memory_megabytes, approach.timeout_seconds);
    if (!run.solver_found) throw std::runtime_error("no cvc5 on PATH");

    Answer answer;
    answer.solver_name = run.solver_name;
    answer.seconds = run.seconds;
    if (!run.answered) return answer;
    if (!run.satisfiable) {
        answer.verdict = Verdict::No;
        return answer;
    }

    const Field field(tensor.characteristic);
    if (!model_reconstructs(field, tensor, encoding, run.field_model)) {
        throw cli::CheckFailed("the model does not reconstruct the tensor");
    }
    answer.verdict = Verdict::Yes;
    answer.decomposition = decomposition_from_model(field, encoding, run.field_model);
    return answer;
}

/// The CNF route, over either field. The two encodings differ in type but not
/// in what happens to them, so the shape below is written once per stage rather
/// than once per field.
Answer from_clauses(const linear_algebra::Tensor& tensor, std::size_t products,
                    const Approach& approach) {
    const bool binary = tensor.characteristic == 2;
    // Read from the same field the unit clauses below come from, so the flag and
    // the cube cannot disagree. Asking `cubes` instead looks equivalent and is
    // not: `decide_rank` clears it before handing one cube over, so the flag was
    // never once true and the ordering was never once moved off term 0.
    const bool pinned = !approach.cube_literals.empty();
    BinaryEncoding boolean_form;
    PrimeFieldEncoding prime_form;
    if (binary) {
        boolean_form = encode_rank_at_most(tensor, products, approach.break_symmetry, pinned);
    } else {
        prime_form = encode_prime_rank_at_most(tensor, products, approach.break_symmetry);
    }
    linear_algebra::Cnf& formula = binary ? boolean_form.formula : prime_form.formula;
    for (int literal : approach.cube_literals) formula.add_clause({literal});

    const SatSolver solver =
        find_sat_solver(!approach.plain_cnf && !formula.parities.empty(), approach.solver);
    const auto run = solve(formula, solver, approach.memory_megabytes, approach.timeout_seconds,
                           approach.proof_path, approach.tuning);
    if (!run.solver_found) {
        throw std::runtime_error(
            "no SAT solver on PATH. Install kissat or cryptominisat, or write the question out");
    }

    Answer answer;
    answer.solver_name = run.solver_name;
    answer.seconds = run.seconds;
    answer.proof_bytes = run.proof_bytes;
    answer.proof = run.proof;
    if (!run.answered) return answer;
    if (!run.satisfiable) {
        // A refutation that does not check is not a lower bound. It means the
        // encoding or the solver is wrong, and nothing downstream could tell.
        if (run.proof == Proof::Refuted) {
            throw cli::CheckFailed("the solver's own refutation did not verify");
        }
        answer.verdict = Verdict::No;
        return answer;
    }

    const Field field(tensor.characteristic);
    const bool rebuilt = binary ? model_reconstructs(field, tensor, boolean_form, run.model)
                                : model_reconstructs(field, tensor, prime_form, run.model);
    if (!rebuilt) throw cli::CheckFailed("the model does not reconstruct the tensor");

    answer.verdict = Verdict::Yes;
    answer.decomposition = binary ? decomposition_from_model(field, boolean_form, run.model)
                                  : decomposition_from_model(field, prime_form, run.model);
    return answer;
}

}  // namespace

Answer decide_rank(const linear_algebra::Tensor& tensor, std::size_t products,
                   const Approach& approach) {
    check_applicable(tensor, approach);
    if (approach.use_field_theory) return from_field_theory(tensor, products, approach);
    if (approach.cubes.empty()) return from_clauses(tensor, products, approach);

    // One instance per cube. A yes anywhere is a yes; a no needs every cube to
    // refuse, and a single cube that gave up makes the whole answer unknown,
    // because the decomposition may have been in the part nobody finished.
    Answer combined;
    combined.verdict = Verdict::No;
    for (const std::vector<int>& cube : approach.cubes) {
        Approach one = approach;
        one.cubes.clear();
        one.cube_literals = cube;

        const Answer piece = from_clauses(tensor, products, one);
        combined.solver_name = piece.solver_name;
        combined.seconds += piece.seconds;
        combined.proof_bytes += piece.proof_bytes;
        if (piece.verdict == Verdict::Yes) {
            combined.verdict = Verdict::Yes;
            combined.decomposition = piece.decomposition;
            return combined;
        }
        if (piece.verdict == Verdict::Unknown) combined.verdict = Verdict::Unknown;
    }
    return combined;
}

namespace {

/// Walk up from the floor to the first satisfiable rank. False if a question
/// went unanswered, which moves no bound: an unknown is not a no, and treating
/// it as one would invent a lower bound.
bool narrow(const linear_algebra::Tensor& tensor, const Approach& approach, std::size_t budget,
            RankBounds& bounds) {
    Approach limited = approach;
    limited.timeout_seconds = budget;

    for (std::size_t k = bounds.lower; k <= bounds.upper; ++k) {
        const Answer answer = decide_rank(tensor, k, limited);
        ++bounds.questions_asked;
        bounds.seconds += answer.seconds;

        if (answer.verdict == Verdict::Unknown) return false;
        if (answer.verdict == Verdict::Yes) {
            bounds.upper = k;
            bounds.decomposition = answer.decomposition;
            return true;
        }
        if (answer.proof == Proof::Verified) ++bounds.refutations_verified;
        bounds.lower = k + 1;
    }
    return true;
}

}  // namespace

RankBounds find_rank(const linear_algebra::Tensor& tensor, const Approach& approach,
                     std::size_t floor, std::size_t ceiling) {
    RankBounds bounds;
    bounds.lower = floor;
    bounds.upper = ceiling;

    // A cheap pass first when one is asked for. Whatever it settles is settled
    // soundly, and whatever it gives up on is left for the pass that can afford
    // it, so the large budget is spent on a bracket rather than on a guess.
    if (approach.probe_seconds > 0 && approach.probe_seconds < approach.timeout_seconds) {
        narrow(tensor, approach, approach.probe_seconds, bounds);
    }

    bounds.exact = narrow(tensor, approach, approach.timeout_seconds, bounds) &&
                   bounds.lower == bounds.upper;
    return bounds;
}

std::string write_question(const linear_algebra::Tensor& tensor, std::size_t products,
                           const Approach& approach, const std::string& path) {
    check_applicable(tensor, approach);
    // A cube split is one question per cube and this writes one file. Dropping
    // the cubes would write a file that answers a different question, and the
    // difference is invisible in the file, so say so instead.
    if (!approach.cubes.empty()) {
        throw std::invalid_argument("a cube split is " + std::to_string(approach.cubes.size()) +
                                    " questions and this writes one file; ask for them one cube "
                                    "at a time, or write the question without cubes");
    }
    std::ofstream out(path);
    if (!out) throw std::runtime_error("cannot write " + path);

    if (approach.use_field_theory) {
        const auto encoding = encode_field_rank_at_most(tensor, products);
        linear_algebra::write_smtlib(out, encoding.problem);
        return std::to_string(encoding.problem.constants.size()) + " constants, " +
               std::to_string(encoding.problem.assertions.size()) + " assertions";
    }

    const bool binary = tensor.characteristic == 2;
    const linear_algebra::Cnf formula =
        binary ? encode_rank_at_most(tensor, products, approach.break_symmetry).formula
               : encode_prime_rank_at_most(tensor, products, approach.break_symmetry).formula;

    const bool native = !approach.plain_cnf;
    linear_algebra::write_dimacs(out, formula, native);
    return std::to_string(formula.total_variable_count(native)) + " variables, " +
           std::to_string(formula.total_clause_count(native)) + " clauses";
}

}  // namespace satisfiability
