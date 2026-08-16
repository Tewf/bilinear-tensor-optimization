#include "rank_question.h"

#include <fstream>
#include <stdexcept>

#include "binary_encoding.h"
#include "field_theory_encoding.h"
#include "prime_field_encoding.h"
#include "solver_process.h"

namespace satisfiability {

namespace {

/// Refuse the combinations that do not mean anything, by name.
void check_applicable(const linear_algebra::Tensor& tensor, const Approach& approach) {
    if (approach.break_symmetry && tensor.characteristic != 2) {
        throw std::invalid_argument(
            "ordering the terms is only implemented for GF(2); over GF(p) use the field theory");
    }
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
        throw std::runtime_error("the model does not reconstruct the tensor");
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
    BinaryEncoding boolean_form;
    PrimeFieldEncoding prime_form;
    if (binary) {
        boolean_form = encode_rank_at_most(tensor, products, approach.break_symmetry);
    } else {
        prime_form = encode_prime_rank_at_most(tensor, products);
    }
    const linear_algebra::Cnf& formula = binary ? boolean_form.formula : prime_form.formula;

    const SatSolver solver =
        find_sat_solver(!approach.plain_cnf && !formula.parities.empty(), approach.solver);
    const auto run =
        solve(formula, solver, approach.memory_megabytes, approach.timeout_seconds);
    if (!run.solver_found) {
        throw std::runtime_error(
            "no SAT solver on PATH. Install kissat or cryptominisat, or write the question out");
    }

    Answer answer;
    answer.solver_name = run.solver_name;
    answer.seconds = run.seconds;
    if (!run.answered) return answer;
    if (!run.satisfiable) {
        answer.verdict = Verdict::No;
        return answer;
    }

    const Field field(tensor.characteristic);
    const bool rebuilt = binary ? model_reconstructs(field, tensor, boolean_form, run.model)
                                : model_reconstructs(field, tensor, prime_form, run.model);
    if (!rebuilt) throw std::runtime_error("the model does not reconstruct the tensor");

    answer.verdict = Verdict::Yes;
    answer.decomposition = binary ? decomposition_from_model(field, boolean_form, run.model)
                                  : decomposition_from_model(field, prime_form, run.model);
    return answer;
}

}  // namespace

Answer decide_rank(const linear_algebra::Tensor& tensor, std::size_t products,
                   const Approach& approach) {
    check_applicable(tensor, approach);
    return approach.use_field_theory ? from_field_theory(tensor, products, approach)
                                     : from_clauses(tensor, products, approach);
}

std::string write_question(const linear_algebra::Tensor& tensor, std::size_t products,
                           const Approach& approach, const std::string& path) {
    check_applicable(tensor, approach);
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
               : encode_prime_rank_at_most(tensor, products).formula;

    const bool native = !approach.plain_cnf;
    linear_algebra::write_dimacs(out, formula, native);
    return std::to_string(formula.total_variable_count(native)) + " variables, " +
           std::to_string(formula.total_clause_count(native)) + " clauses";
}

}  // namespace satisfiability
