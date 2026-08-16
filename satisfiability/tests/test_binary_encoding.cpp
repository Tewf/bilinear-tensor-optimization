/// The GF(2) encoding, checked without a solver anywhere in sight.
///
/// The trick is that a decomposition we already know gives the assignment the
/// formula is supposed to accept. So: build a tensor as a sum of rank-one terms,
/// write down the truth assignment those terms stand for, and assert every
/// clause and every parity holds. If the encoding says otherwise it is wrong,
/// and no solver was needed to find that out.
///
/// The negative is the same instrument: change one entry of the tensor and the
/// same assignment must now break something. An encoding that accepts anything
/// would pass the first check and fail this one.
#include <algorithm>
#include <string>
#include <vector>

#include "binary_encoding.h"
#include "check.h"

namespace {

using linear_algebra::Cnf;
using linear_algebra::Model;
using satisfiability::BinaryEncoding;
using satisfiability::Field;
using satisfiability::Matrix;

/// One rank-one term of a decomposition, as three GF(2) vectors.
struct Term {
    std::vector<bool> left;
    std::vector<bool> right;
    std::vector<bool> output;
};

linear_algebra::Tensor tensor_from(const Field& field, const std::vector<Term>& terms,
                                   std::size_t rows, std::size_t columns, std::size_t slices) {
    linear_algebra::Tensor tensor;
    tensor.characteristic = 2;
    tensor.slices.assign(slices, Matrix(rows, columns));

    for (const Term& term : terms) {
        for (std::size_t slice = 0; slice < slices; ++slice) {
            if (!term.output[slice]) continue;
            for (std::size_t row = 0; row < rows; ++row) {
                if (!term.left[row]) continue;
                for (std::size_t column = 0; column < columns; ++column) {
                    if (!term.right[column]) continue;
                    field.addin(tensor.slices[slice](row, column), field.one);
                }
            }
        }
    }
    return tensor;
}

/// The truth assignment those terms stand for, in the encoding's numbering.
///
/// Only the a, b and c variables are set here; every Tseitin variable is then
/// forced, so filling them in is mechanical and done below.
Model model_for(const BinaryEncoding& encoding, const std::vector<Term>& terms) {
    Model model;
    model.answered = true;
    model.satisfiable = true;
    model.values.assign(encoding.formula.variable_count + 1, false);

    for (std::size_t term = 0; term < terms.size(); ++term) {
        for (std::size_t index = 0; index < encoding.rows; ++index) {
            model.values[static_cast<std::size_t>(encoding.left[term * encoding.rows + index])] =
                terms[term].left[index];
        }
        for (std::size_t index = 0; index < encoding.columns; ++index) {
            model.values[static_cast<std::size_t>(encoding.right[term * encoding.columns + index])] =
                terms[term].right[index];
        }
        for (std::size_t index = 0; index < encoding.slices; ++index) {
            model.values[static_cast<std::size_t>(encoding.output[term * encoding.slices + index])] =
                terms[term].output[index];
        }
    }
    return model;
}

/// Propagate the definitional clauses so every Tseitin variable takes the value
/// its definition forces. Repeated to a fixed point, which is enough because the
/// definitions form a DAG.
void complete(const Cnf& formula, Model& model) {
    for (int pass = 0; pass < 4; ++pass) {
        for (const std::vector<int>& clause : formula.clauses) {
            if (clause.size() != 3) continue;
            // The shape written by equate_to_conjunction: (r, -l, -r').
            if (clause[0] <= 0 || clause[1] >= 0 || clause[2] >= 0) continue;
            const bool left = model.values[static_cast<std::size_t>(-clause[1])];
            const bool right = model.values[static_cast<std::size_t>(-clause[2])];
            model.values[static_cast<std::size_t>(clause[0])] = left && right;
        }
    }
}

bool literal_holds(const Model& model, int literal) {
    const bool value = model.values[static_cast<std::size_t>(std::abs(literal))];
    return literal > 0 ? value : !value;
}

/// How many clauses and parities the assignment breaks.
std::size_t violations(const Cnf& formula, const Model& model) {
    std::size_t broken = 0;
    for (const std::vector<int>& clause : formula.clauses) {
        bool any = false;
        for (int literal : clause) any = any || literal_holds(model, literal);
        if (!any) ++broken;
    }
    for (const Cnf::Parity& parity : formula.parities) {
        bool odd = false;
        for (int literal : parity.literals) odd = odd != literal_holds(model, literal);
        if (odd != parity.value) ++broken;
    }
    return broken;
}

std::vector<Term> karatsuba_terms() {
    // Two-by-two polynomial multiplication over GF(2) in three products:
    // a0b0, a1b1, and (a0+a1)(b0+b1). Slices are the three coefficients.
    return {Term{{true, false}, {true, false}, {true, false, true}},
            Term{{false, true}, {false, true}, {false, false, true}},
            Term{{true, true}, {true, true}, {false, true, false}}};
}

}  // namespace

int main() {
    const Field field(2);
    const std::vector<Term> terms = karatsuba_terms();
    const auto tensor = tensor_from(field, terms, 2, 2, 3);

    // Sanity: that really is 2x2-term polynomial multiplication.
    check::equal("slice 0 is a0b0", field.isOne(tensor.slices[0](0, 0)) ? 1 : 0, 1);
    check::equal("slice 1 has a0b1", field.isOne(tensor.slices[1](0, 1)) ? 1 : 0, 1);
    check::equal("slice 2 is a1b1", field.isOne(tensor.slices[2](1, 1)) ? 1 : 0, 1);

    auto encoding = satisfiability::encode_rank_at_most(tensor, 3);
    check::equal("three terms of a 2x2x3 tensor use 21 literal variables",
                 static_cast<long long>(encoding.left.size() + encoding.right.size() +
                                        encoding.output.size()),
                 21);
    check::equal("one parity per tensor entry",
                 static_cast<long long>(encoding.formula.parities.size()), 12);

    Model model = model_for(encoding, terms);
    complete(encoding.formula, model);
    check::equal("Karatsuba satisfies its own encoding",
                 static_cast<long long>(violations(encoding.formula, model)), 0);
    check::equal("and the model rebuilds the tensor",
                 satisfiability::model_reconstructs(field, tensor, encoding, model) ? 1 : 0, 1);

    // The decomposition read back out is three rank-one matrices.
    const auto recovered = satisfiability::decomposition_from_model(field, encoding, model);
    check::equal("three terms recovered", static_cast<long long>(recovered.size()), 3);

    // Change one entry and the same assignment must break a parity.
    auto perturbed = tensor;
    field.addin(perturbed.slices[0](1, 1), field.one);
    auto other = satisfiability::encode_rank_at_most(perturbed, 3);
    Model same = model_for(other, terms);
    complete(other.formula, same);
    check::equal("a changed tensor is no longer satisfied",
                 violations(other.formula, same) > 0 ? 1 : 0, 1);

    // The expansion of parities into ordinary clauses must agree with them.
    const Cnf expanded = linear_algebra::with_parities_expanded(encoding.formula);
    check::equal("expansion leaves no parities",
                 static_cast<long long>(expanded.parities.size()), 0);
    Model widened = model;
    widened.values.resize(expanded.variable_count + 1, false);
    // Tseitin xor variables are forced; solve them by one sweep in order.
    for (std::size_t pass = 0; pass < 2; ++pass) {
        for (const std::vector<int>& clause : expanded.clauses) {
            if (clause.size() != 3 || clause[0] >= 0) continue;
            const std::size_t result = static_cast<std::size_t>(-clause[0]);
            if (result <= encoding.formula.variable_count) continue;
            widened.values[result] = literal_holds(widened, clause[1]) != literal_holds(widened, clause[2]);
        }
    }
    check::equal("the expanded formula is satisfied by the same decomposition",
                 static_cast<long long>(violations(expanded, widened)), 0);

    // GF(3) must be refused rather than silently encoded as if it were GF(2).
    auto ternary = tensor;
    ternary.characteristic = 3;
    bool threw = false;
    try {
        satisfiability::encode_rank_at_most(ternary, 3);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    check::equal("GF(3) is refused by the Boolean encoding", threw ? 1 : 0, 1);

    // An encoding that cannot fit is refused before it is built.
    satisfiability::set_variable_budget(10);
    bool refused = false;
    try {
        satisfiability::encode_rank_at_most(tensor, 3);
    } catch (const std::invalid_argument&) {
        refused = true;
    }
    satisfiability::set_variable_budget(10'000'000);
    check::equal("an oversized encoding is refused", refused ? 1 : 0, 1);

    return check::report("binary encoding");
}
