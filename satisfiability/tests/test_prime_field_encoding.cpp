/// The one-hot GF(p) encoding, checked the same way as the Boolean one and for
/// the same reason: this is the backend whose field arithmetic is hand-written,
/// so it is the one that can be confidently wrong.
///
/// A decomposition we already know gives the assignment the formula must
/// accept. Propagating the definitional clauses fills in every derived group,
/// and then nothing may be violated. If the tensor is altered, the value forced
/// by the definitions and the value demanded by the final clause disagree, two
/// members of one one-hot group are true at once, and the at-most-one clause
/// catches it, which is how a wrong answer becomes a failing test rather than
/// a plausible number.
#include <cstdlib>
#include <vector>

#include "binary_encoding.h"
#include "check.h"
#include "prime_field_encoding.h"

namespace {

using linear_algebra::Cnf;
using linear_algebra::Model;
using satisfiability::Field;
using satisfiability::Matrix;
using satisfiability::PrimeFieldEncoding;

/// A rank-one term over GF(p), as three vectors of field values.
struct Term {
    std::vector<std::size_t> left;
    std::vector<std::size_t> right;
    std::vector<std::size_t> output;
};

/// Add a plain integer into a field element.
void Element_add(const Field& field, satisfiability::Element& target, std::size_t value) {
    satisfiability::Element addend;
    field.init(addend, static_cast<int64_t>(value));
    field.addin(target, addend);
}

linear_algebra::Tensor tensor_from(const Field& field, std::size_t characteristic,
                                   const std::vector<Term>& terms, std::size_t rows,
                                   std::size_t columns, std::size_t slices) {
    linear_algebra::Tensor tensor;
    tensor.characteristic = static_cast<int64_t>(characteristic);
    tensor.slices.assign(slices, Matrix(rows, columns));

    for (const Term& term : terms) {
        for (std::size_t slice = 0; slice < slices; ++slice) {
            for (std::size_t row = 0; row < rows; ++row) {
                for (std::size_t column = 0; column < columns; ++column) {
                    const std::size_t value =
                        (term.left[row] * term.right[column] * term.output[slice]) % characteristic;
                    Element_add(field, tensor.slices[slice](row, column), value);
                }
            }
        }
    }
    return tensor;
}

/// Set the variables standing for the known field values, then let the
/// definitional clauses force everything else.
Model model_for(const PrimeFieldEncoding& encoding, const std::vector<Term>& terms) {
    Model model;
    model.answered = true;
    model.satisfiable = true;
    model.values.assign(encoding.formula.variable_count + 1, false);

    const std::size_t p = encoding.characteristic;
    for (std::size_t term = 0; term < terms.size(); ++term) {
        for (std::size_t index = 0; index < encoding.rows; ++index) {
            model.values[static_cast<std::size_t>(
                encoding.left[(term * encoding.rows + index) * p + terms[term].left[index]])] = true;
        }
        for (std::size_t index = 0; index < encoding.columns; ++index) {
            model.values[static_cast<std::size_t>(
                encoding.right[(term * encoding.columns + index) * p +
                               terms[term].right[index]])] = true;
        }
        for (std::size_t index = 0; index < encoding.slices; ++index) {
            model.values[static_cast<std::size_t>(
                encoding.output[(term * encoding.slices + index) * p +
                                terms[term].output[index]])] = true;
        }
    }
    return model;
}

/// Propagate unit clauses and the `(-x, -y, z)` definitions to a fixed point.
void complete(const Cnf& formula, Model& model, std::size_t passes) {
    for (std::size_t pass = 0; pass < passes; ++pass) {
        for (const std::vector<int>& clause : formula.clauses) {
            if (clause.size() == 1 && clause[0] > 0) {
                model.values[static_cast<std::size_t>(clause[0])] = true;
            } else if (clause.size() == 3 && clause[0] < 0 && clause[1] < 0 && clause[2] > 0) {
                const bool first = model.values[static_cast<std::size_t>(-clause[0])];
                const bool second = model.values[static_cast<std::size_t>(-clause[1])];
                if (first && second) model.values[static_cast<std::size_t>(clause[2])] = true;
            }
        }
    }
}

bool literal_holds(const Model& model, int literal) {
    const bool value = model.values[static_cast<std::size_t>(std::abs(literal))];
    return literal > 0 ? value : !value;
}

std::size_t violations(const Cnf& formula, const Model& model) {
    std::size_t broken = 0;
    for (const std::vector<int>& clause : formula.clauses) {
        bool any = false;
        for (int literal : clause) any = any || literal_holds(model, literal);
        if (!any) ++broken;
    }
    return broken;
}

}  // namespace

int main() {
    // GF(3), two rank-one terms of a 2x2 map with two slices.
    const std::size_t characteristic = 3;
    const Field field(static_cast<int64_t>(characteristic));
    const std::vector<Term> terms = {Term{{1, 0}, {1, 0}, {1, 0}}, Term{{0, 2}, {0, 1}, {0, 1}}};
    const auto tensor = tensor_from(field, characteristic, terms, 2, 2, 2);

    auto encoding = satisfiability::encode_prime_rank_at_most(tensor, 2);
    check::equal("one-hot groups for every unknown",
                 static_cast<long long>(encoding.left.size() + encoding.right.size() +
                                        encoding.output.size()),
                 static_cast<long long>(2 * (2 + 2 + 2) * characteristic));

    Model model = model_for(encoding, terms);
    complete(encoding.formula, model, 8);
    check::equal("the known GF(3) decomposition satisfies its encoding",
                 static_cast<long long>(violations(encoding.formula, model)), 0);
    check::equal("and the model rebuilds the tensor",
                 satisfiability::model_reconstructs(field, tensor, encoding, model) ? 1 : 0, 1);

    // Alter one entry: the definitions force one value and the final clause
    // demands another, so a one-hot group has two members true.
    auto perturbed = tensor;
    satisfiability::Element one;
    field.init(one, 1);
    field.addin(perturbed.slices[0](0, 0), one);
    auto other = satisfiability::encode_prime_rank_at_most(perturbed, 2);
    Model same = model_for(other, terms);
    complete(other.formula, same, 8);
    check::equal("a changed tensor is no longer satisfied",
                 violations(other.formula, same) > 0 ? 1 : 0, 1);

    // GF(2) through the general encoder must accept what the Boolean one does.
    // The two agreeing there is the cheapest evidence the tables are right.
    const Field binary(2);
    const std::vector<Term> karatsuba = {Term{{1, 0}, {1, 0}, {1, 0, 1}},
                                         Term{{0, 1}, {0, 1}, {0, 0, 1}},
                                         Term{{1, 1}, {1, 1}, {0, 1, 0}}};
    const auto binary_tensor = tensor_from(binary, 2, karatsuba, 2, 2, 3);
    auto general = satisfiability::encode_prime_rank_at_most(binary_tensor, 3);
    Model shared = model_for(general, karatsuba);
    complete(general.formula, shared, 10);
    check::equal("Karatsuba satisfies the general encoder too",
                 static_cast<long long>(violations(general.formula, shared)), 0);
    check::equal("and it rebuilds the same tensor",
                 satisfiability::model_reconstructs(binary, binary_tensor, general, shared) ? 1 : 0,
                 1);

    // A composite characteristic has no field to write a table for.
    auto composite = tensor;
    composite.characteristic = 4;
    bool threw = false;
    try {
        satisfiability::encode_prime_rank_at_most(composite, 2);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    check::equal("GF(4) is refused, it is not a prime field", threw ? 1 : 0, 1);

    return check::report("prime field encoding");
}
