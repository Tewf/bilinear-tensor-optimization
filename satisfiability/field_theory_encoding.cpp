#include "field_theory_encoding.h"

#include <stdexcept>

namespace satisfiability {

namespace {

/// Nest a binary operator over the terms, since `ff.add` and `ff.mul` are
/// binary and an empty sum is the field's zero.
std::string folded(const std::string& op, const std::vector<std::string>& terms,
                   const std::string& empty) {
    if (terms.empty()) return empty;

    std::string built = terms.front();
    for (std::size_t index = 1; index < terms.size(); ++index) {
        built = "(" + op + " " + built + " " + terms[index] + ")";
    }
    return built;
}

std::size_t reduced(int64_t value, int64_t characteristic) {
    return static_cast<std::size_t>((value % characteristic + characteristic) % characteristic);
}

std::size_t value_of(const linear_algebra::SmtModel& model, const std::string& name) {
    const auto found = model.values.find(name);
    return found == model.values.end() ? 0 : found->second;
}

}  // namespace

std::string FieldTheoryEncoding::left_name(std::size_t term, std::size_t row) {
    return "a_" + std::to_string(term) + "_" + std::to_string(row);
}
std::string FieldTheoryEncoding::right_name(std::size_t term, std::size_t column) {
    return "b_" + std::to_string(term) + "_" + std::to_string(column);
}
std::string FieldTheoryEncoding::output_name(std::size_t term, std::size_t slice) {
    return "c_" + std::to_string(term) + "_" + std::to_string(slice);
}

FieldTheoryEncoding encode_field_rank_at_most(const linear_algebra::Tensor& tensor,
                                              std::size_t products) {
    if (tensor.characteristic < 2) {
        throw std::invalid_argument("a finite field needs a characteristic of at least two");
    }
    if (tensor.slices.empty()) throw std::invalid_argument("a tensor with no slices has rank 0");

    FieldTheoryEncoding encoding;
    encoding.characteristic = static_cast<std::size_t>(tensor.characteristic);
    encoding.products = products;
    encoding.rows = tensor.rows();
    encoding.columns = tensor.columns();
    encoding.slices = tensor.slices.size();
    encoding.problem.characteristic = encoding.characteristic;

    for (std::size_t term = 0; term < products; ++term) {
        for (std::size_t row = 0; row < encoding.rows; ++row) {
            encoding.problem.constants.push_back(FieldTheoryEncoding::left_name(term, row));
        }
        for (std::size_t column = 0; column < encoding.columns; ++column) {
            encoding.problem.constants.push_back(FieldTheoryEncoding::right_name(term, column));
        }
        for (std::size_t slice = 0; slice < encoding.slices; ++slice) {
            encoding.problem.constants.push_back(FieldTheoryEncoding::output_name(term, slice));
        }
    }

    const std::string zero = linear_algebra::SmtProblem::literal(0);
    for (std::size_t row = 0; row < encoding.rows; ++row) {
        for (std::size_t column = 0; column < encoding.columns; ++column) {
            for (std::size_t slice = 0; slice < encoding.slices; ++slice) {
                std::vector<std::string> summands;
                summands.reserve(products);
                for (std::size_t term = 0; term < products; ++term) {
                    summands.push_back(folded(
                        "ff.mul",
                        {FieldTheoryEncoding::left_name(term, row),
                         FieldTheoryEncoding::right_name(term, column),
                         FieldTheoryEncoding::output_name(term, slice)},
                        zero));
                }

                const std::size_t wanted =
                    reduced(tensor.slices[slice](row, column), tensor.characteristic);
                encoding.problem.assertions.push_back("(= " + folded("ff.add", summands, zero) +
                                                      " " +
                                                      linear_algebra::SmtProblem::literal(wanted) +
                                                      ")");
            }
        }
    }
    return encoding;
}

std::vector<Matrix> decomposition_from_model(const Field& field,
                                             const FieldTheoryEncoding& encoding,
                                             const linear_algebra::SmtModel& model) {
    if (!model.satisfiable) return {};

    std::vector<Matrix> terms;
    terms.reserve(encoding.products);
    for (std::size_t term = 0; term < encoding.products; ++term) {
        Matrix outer(encoding.rows, encoding.columns);
        for (std::size_t row = 0; row < encoding.rows; ++row) {
            const std::size_t a = value_of(model, FieldTheoryEncoding::left_name(term, row));
            if (a == 0) continue;
            for (std::size_t column = 0; column < encoding.columns; ++column) {
                const std::size_t b =
                    value_of(model, FieldTheoryEncoding::right_name(term, column));
                field.init(outer(row, column), static_cast<int64_t>(a * b));
            }
        }
        terms.push_back(std::move(outer));
    }
    return terms;
}

bool model_reconstructs(const Field& field, const linear_algebra::Tensor& tensor,
                        const FieldTheoryEncoding& encoding,
                        const linear_algebra::SmtModel& model) {
    if (!model.satisfiable) return false;
    const std::vector<Matrix> terms = decomposition_from_model(field, encoding, model);

    for (std::size_t slice = 0; slice < encoding.slices; ++slice) {
        Matrix rebuilt(encoding.rows, encoding.columns);
        for (std::size_t term = 0; term < encoding.products; ++term) {
            const std::size_t weight = value_of(model, FieldTheoryEncoding::output_name(term, slice));
            if (weight == 0) continue;

            Element scalar;
            field.init(scalar, static_cast<int64_t>(weight));
            for (std::size_t entry = 0; entry < rebuilt.entry_count(); ++entry) {
                field.axpyin(rebuilt.data()[entry], scalar, terms[term].data()[entry]);
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
