#include "integer_programme_encoding.h"

#include <array>
#include <stdexcept>
#include <string>

namespace bilinear_rank {

using optimisation::Coefficient;
using optimisation::Constraint;
using optimisation::Number;
using optimisation::Relation;
using optimisation::Variable;

namespace {

Variable binary(const std::string& name) {
    Variable variable;
    variable.name = name;
    variable.integral = true;
    variable.bounded_below = true;
    variable.lower = Number(0);
    variable.bounded_above = true;
    variable.upper = Number(1);
    return variable;
}

/// `conjunction ≤ each part`, and `conjunction ≥ sum of parts − 2`, which
/// together say the conjunction is one exactly when all three parts are.
void pin_to_conjunction(optimisation::IntegerProgramme& programme, std::size_t conjunction,
                        const std::array<std::size_t, 3>& parts) {
    for (const std::size_t part : parts) {
        Constraint bound;
        bound.terms = {{conjunction, Number(1)}, {part, Number(-1)}};
        bound.relation = Relation::LessOrEqual;
        bound.bound = Number(0);
        programme.constraints.push_back(std::move(bound));
    }

    Constraint together;
    together.terms = {{conjunction, Number(1)}};
    for (const std::size_t part : parts) together.terms.push_back({part, Number(-1)});
    together.relation = Relation::GreaterOrEqual;
    together.bound = Number(-2);
    programme.constraints.push_back(std::move(together));
}

}  // namespace

std::size_t EncodedRank::left_variable(std::size_t term, std::size_t row) const {
    return term * rows + row;
}

std::size_t EncodedRank::right_variable(std::size_t term, std::size_t column) const {
    return rank * rows + term * columns + column;
}

std::size_t EncodedRank::output_variable(std::size_t term, std::size_t slice) const {
    return rank * (rows + columns) + term * slices + slice;
}

EncodedRank encode_rank_question(const Field& field, const std::vector<Matrix>& slices,
                                 std::size_t rank) {
    if (field.characteristic() != 2) {
        throw std::runtime_error(
            "the integer programme encoding is GF(2) only; over GF(p) a product of operands is "
            "not a conjunction and needs a different encoding, not a larger one");
    }
    if (slices.empty()) throw std::runtime_error("no slices to decompose");

    EncodedRank encoded;
    encoded.rank = rank;
    encoded.rows = slices.front().rows();
    encoded.columns = slices.front().columns();
    encoded.slices = slices.size();

    const std::size_t products = rank * encoded.rows * encoded.columns * encoded.slices;
    const std::size_t product_base = rank * (encoded.rows + encoded.columns + encoded.slices);
    const std::size_t carry_base = product_base + products;

    optimisation::IntegerProgramme& programme = encoded.programme;
    for (std::size_t term = 0; term < rank; ++term) {
        for (std::size_t row = 0; row < encoded.rows; ++row) {
            programme.variables.push_back(binary("u" + std::to_string(term * encoded.rows + row)));
        }
    }
    for (std::size_t term = 0; term < rank; ++term) {
        for (std::size_t column = 0; column < encoded.columns; ++column) {
            programme.variables.push_back(binary("v"));
        }
    }
    for (std::size_t term = 0; term < rank; ++term) {
        for (std::size_t slice = 0; slice < encoded.slices; ++slice) {
            programme.variables.push_back(binary("w"));
        }
    }
    for (std::size_t index = 0; index < products; ++index) programme.variables.push_back(binary("z"));

    // One carry per equation. A parity over GF(2) is an integer equation once
    // the number of times it wrapped is a variable, and it cannot wrap more than
    // `rank / 2` times because there are only `rank` terms to add.
    for (std::size_t index = 0; index < encoded.rows * encoded.columns * encoded.slices; ++index) {
        Variable carry;
        carry.name = "carry";
        carry.integral = true;
        carry.bounded_below = true;
        carry.lower = Number(0);
        carry.bounded_above = true;
        carry.upper = Number(static_cast<long>(rank / 2));
        programme.variables.push_back(carry);
    }
    programme.objective.assign(programme.variables.size(), Number(0));

    const auto product_variable = [&](std::size_t term, std::size_t row, std::size_t column,
                                      std::size_t slice) {
        return product_base +
               ((term * encoded.rows + row) * encoded.columns + column) * encoded.slices + slice;
    };

    for (std::size_t term = 0; term < rank; ++term) {
        for (std::size_t row = 0; row < encoded.rows; ++row) {
            for (std::size_t column = 0; column < encoded.columns; ++column) {
                for (std::size_t slice = 0; slice < encoded.slices; ++slice) {
                    pin_to_conjunction(programme, product_variable(term, row, column, slice),
                                       {encoded.left_variable(term, row),
                                        encoded.right_variable(term, column),
                                        encoded.output_variable(term, slice)});
                }
            }
        }
    }

    for (std::size_t row = 0; row < encoded.rows; ++row) {
        for (std::size_t column = 0; column < encoded.columns; ++column) {
            for (std::size_t slice = 0; slice < encoded.slices; ++slice) {
                Constraint equation;
                for (std::size_t term = 0; term < rank; ++term) {
                    equation.terms.push_back(
                        {product_variable(term, row, column, slice), Number(1)});
                }
                const std::size_t carry =
                    carry_base + (row * encoded.columns + column) * encoded.slices + slice;
                equation.terms.push_back({carry, Number(-2)});
                equation.relation = Relation::Equal;
                equation.bound =
                    Number(field.isZero(slices[slice](row, column)) ? 0 : 1);
                programme.constraints.push_back(std::move(equation));
            }
        }
    }
    return encoded;
}

void fix_first_term(EncodedRank& encoded, const std::vector<Element>& left,
                    const std::vector<Element>& right) {
    const auto pin = [&](std::size_t variable, const Element& value) {
        Constraint fixed;
        fixed.terms = {{variable, Number(1)}};
        fixed.relation = Relation::Equal;
        fixed.bound = Number(value == 0 ? 0 : 1);
        encoded.programme.constraints.push_back(std::move(fixed));
    };
    for (std::size_t row = 0; row < left.size() && row < encoded.rows; ++row) {
        pin(encoded.left_variable(0, row), left[row]);
    }
    for (std::size_t column = 0; column < right.size() && column < encoded.columns; ++column) {
        pin(encoded.right_variable(0, column), right[column]);
    }
}

std::vector<Matrix> products_of(const Field& field, const EncodedRank& encoded,
                                const std::vector<optimisation::Number>& values) {
    std::vector<Matrix> products;
    for (std::size_t term = 0; term < encoded.rank; ++term) {
        Matrix product(encoded.rows, encoded.columns);
        bool anything = false;
        for (std::size_t row = 0; row < encoded.rows; ++row) {
            const bool left = values[encoded.left_variable(term, row)] != optimisation::Number(0);
            if (!left) continue;
            for (std::size_t column = 0; column < encoded.columns; ++column) {
                if (values[encoded.right_variable(term, column)] == optimisation::Number(0)) continue;
                product(row, column) = field.one;
                anything = true;
            }
        }
        if (anything) products.push_back(std::move(product));
    }
    return products;
}

}  // namespace bilinear_rank
