#include "binary_encoding.h"

#include <stdexcept>
#include <string>

namespace satisfiability {

namespace {

std::size_t budget = 10'000'000;

/// `result ↔ left ∧ right`, in the three clauses that says.
void equate_to_conjunction(linear_algebra::Cnf& formula, int result, int left, int right) {
    formula.add_clause({-result, left});
    formula.add_clause({-result, right});
    formula.add_clause({result, -left, -right});
}

/// Constrain the term at `first` to be lexicographically at most the one at
/// `second`, over the concatenation of their three vectors.
///
/// The usual chain: `equal` tracks "the two agree everywhere so far", and while
/// that holds the earlier term may not have a 1 where the later has a 0.
void order_terms(linear_algebra::Cnf& formula, const std::vector<int>& earlier,
                 const std::vector<int>& later) {
    int equal = formula.new_variable();
    formula.add_clause({equal});

    for (std::size_t index = 0; index < earlier.size(); ++index) {
        const int mine = earlier[index];
        const int theirs = later[index];

        // While still equal, mine <= theirs.
        formula.add_clause({-equal, -mine, theirs});

        if (index + 1 == earlier.size()) break;
        const int still = formula.new_variable();
        // still -> equal, and still -> (mine == theirs).
        formula.add_clause({-still, equal});
        formula.add_clause({-still, -mine, theirs});
        formula.add_clause({-still, mine, -theirs});
        // equal and mine == theirs -> still.
        formula.add_clause({still, -equal, mine, theirs});
        formula.add_clause({still, -equal, -mine, -theirs});
        equal = still;
    }
}

std::vector<int> term_variables(const BinaryEncoding& encoding, std::size_t term) {
    std::vector<int> all;
    all.reserve(encoding.rows + encoding.columns + encoding.slices);
    for (std::size_t index = 0; index < encoding.rows; ++index) {
        all.push_back(encoding.left[term * encoding.rows + index]);
    }
    for (std::size_t index = 0; index < encoding.columns; ++index) {
        all.push_back(encoding.right[term * encoding.columns + index]);
    }
    for (std::size_t index = 0; index < encoding.slices; ++index) {
        all.push_back(encoding.output[term * encoding.slices + index]);
    }
    return all;
}

bool holds(const linear_algebra::Model& model, int variable) {
    const std::size_t index = static_cast<std::size_t>(variable);
    return index < model.values.size() && model.values[index];
}

}  // namespace

std::size_t variable_budget() { return budget; }
void set_variable_budget(std::size_t variables) { budget = variables; }

BinaryEncoding encode_rank_at_most(const linear_algebra::Tensor& tensor, std::size_t products,
                                   bool break_symmetry) {
    if (tensor.characteristic != 2) {
        throw std::invalid_argument(
            "the Boolean encoding is GF(2) only, and this tensor is over GF(" +
            std::to_string(tensor.characteristic) + "); use the prime-field encoding");
    }
    if (tensor.slices.empty()) throw std::invalid_argument("a tensor with no slices has rank 0");

    BinaryEncoding encoding;
    encoding.products = products;
    encoding.rows = tensor.rows();
    encoding.columns = tensor.columns();
    encoding.slices = tensor.slices.size();

    const std::size_t entries = encoding.rows * encoding.columns * encoding.slices;
    if (products != 0 && entries > budget / products) {
        throw std::invalid_argument(
            "encoding " + std::to_string(products) + " products of a " +
            std::to_string(encoding.rows) + "x" + std::to_string(encoding.columns) + "x" +
            std::to_string(encoding.slices) + " tensor wants " + std::to_string(products) + "*" +
            std::to_string(entries) + " product variables, over a budget of " +
            std::to_string(budget));
    }

    linear_algebra::Cnf& formula = encoding.formula;
    for (std::size_t term = 0; term < products; ++term) {
        for (std::size_t index = 0; index < encoding.rows; ++index) {
            encoding.left.push_back(formula.new_variable());
        }
        for (std::size_t index = 0; index < encoding.columns; ++index) {
            encoding.right.push_back(formula.new_variable());
        }
        for (std::size_t index = 0; index < encoding.slices; ++index) {
            encoding.output.push_back(formula.new_variable());
        }
    }

    // q[l][i][j] is a[l][i] and b[l][j], shared across every slice.
    std::vector<int> pair(products * encoding.rows * encoding.columns, 0);
    for (std::size_t term = 0; term < products; ++term) {
        for (std::size_t row = 0; row < encoding.rows; ++row) {
            for (std::size_t column = 0; column < encoding.columns; ++column) {
                const int product = formula.new_variable();
                equate_to_conjunction(formula, product, encoding.left[term * encoding.rows + row],
                                      encoding.right[term * encoding.columns + column]);
                pair[(term * encoding.rows + row) * encoding.columns + column] = product;
            }
        }
    }

    // One parity constraint per entry of the tensor.
    for (std::size_t row = 0; row < encoding.rows; ++row) {
        for (std::size_t column = 0; column < encoding.columns; ++column) {
            for (std::size_t slice = 0; slice < encoding.slices; ++slice) {
                std::vector<int> summands;
                summands.reserve(products);
                for (std::size_t term = 0; term < products; ++term) {
                    const int triple = formula.new_variable();
                    equate_to_conjunction(
                        formula, triple, pair[(term * encoding.rows + row) * encoding.columns + column],
                        encoding.output[term * encoding.slices + slice]);
                    summands.push_back(triple);
                }
                const bool wanted = !Field(2).isZero(tensor.slices[slice](row, column));
                formula.add_parity(std::move(summands), wanted);
            }
        }
    }

    if (break_symmetry) {
        for (std::size_t term = 0; term + 1 < products; ++term) {
            order_terms(formula, term_variables(encoding, term), term_variables(encoding, term + 1));
        }
    }
    return encoding;
}

std::vector<Matrix> decomposition_from_model(const Field& field, const BinaryEncoding& encoding,
                                             const linear_algebra::Model& model) {
    if (!model.satisfiable) return {};

    std::vector<Matrix> terms;
    terms.reserve(encoding.products);
    for (std::size_t term = 0; term < encoding.products; ++term) {
        Matrix outer(encoding.rows, encoding.columns);
        for (std::size_t row = 0; row < encoding.rows; ++row) {
            if (!holds(model, encoding.left[term * encoding.rows + row])) continue;
            for (std::size_t column = 0; column < encoding.columns; ++column) {
                if (!holds(model, encoding.right[term * encoding.columns + column])) continue;
                field.assign(outer(row, column), field.one);
            }
        }
        terms.push_back(std::move(outer));
    }
    return terms;
}

bool model_reconstructs(const Field& field, const linear_algebra::Tensor& tensor,
                        const BinaryEncoding& encoding, const linear_algebra::Model& model) {
    if (!model.satisfiable) return false;
    const std::vector<Matrix> terms = decomposition_from_model(field, encoding, model);

    for (std::size_t slice = 0; slice < encoding.slices; ++slice) {
        Matrix rebuilt(encoding.rows, encoding.columns);
        for (std::size_t term = 0; term < encoding.products; ++term) {
            if (!holds(model, encoding.output[term * encoding.slices + slice])) continue;
            for (std::size_t entry = 0; entry < rebuilt.entry_count(); ++entry) {
                field.addin(rebuilt.data()[entry], terms[term].data()[entry]);
            }
        }
        for (std::size_t entry = 0; entry < rebuilt.entry_count(); ++entry) {
            if (!field.areEqual(rebuilt.data()[entry], tensor.slices[slice].data()[entry])) {
                return false;
            }
        }
    }
    return true;
}

}  // namespace satisfiability
