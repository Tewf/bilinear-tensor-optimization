#pragma once

#include <cstddef>
#include <utility>
#include <vector>

#include "fields.h"
#include "span_basis.h"

namespace exact {

/// Exact rank: the number of independent rows.
///
/// The original computed this with a hand-written elimination that reduced
/// modulo p only after finishing, which overflows a machine integer and leaves
/// negative residues, and then decided invertibility from the result. On the
/// rational side it used a floating-point SVD tolerance.
template <class Field>
std::size_t rank(const Field& field, const MatrixOver<Field>& matrix) {
    if (matrix.rows() == 0 || matrix.columns() == 0) return 0;
    SpanBasis<Field> span(field, matrix.columns());
    for (std::size_t row = 0; row < matrix.rows(); ++row) span.try_add(matrix.row(row));
    return span.dimension();
}

/// The number of multiplications a set of slices costs, which is what the rank
/// search exists to reduce: the sum of their ranks.
template <class Field>
std::size_t multiplication_count(const Field& field,
                                 const std::vector<MatrixOver<Field>>& slices) {
    std::size_t total = 0;
    for (const MatrixOver<Field>& slice : slices) total += rank(field, slice);
    return total;
}

/// How many entries are not zero, which is what the sparsification search
/// exists to reduce.
///
/// Worth being exact about where the original went wrong here, because it is
/// not where it looks. Its reported count ran every entry through
/// `Fraction(x).limit_denominator()`, which snaps 1.85e-18 to zero, so the
/// number it printed was right. The search objective inside its algorithm 3 did
/// not: it counted zeros with `== 0` on the raw doubles, and on the operator it
/// was run against that sees 86 zeros where 144 exist. The count was sound and
/// the thing being maximised was not.
template <class Field>
std::size_t nonzero_count(const Field& field, const MatrixOver<Field>& matrix) {
    std::size_t total = 0;
    for (std::size_t entry = 0; entry < matrix.entry_count(); ++entry) {
        if (!field.isZero(matrix.data()[entry])) ++total;
    }
    return total;
}

/// Whether every one of `targets` lies in the span of `spanning_set`.
///
/// This is what makes a rewrite a rewrite rather than a different, cheaper map,
/// and it is checked on every run: a search bug that quietly dropped a slice
/// would otherwise report an excellent number.
template <class Field>
bool spans_all(const Field& field, const std::vector<MatrixOver<Field>>& spanning_set,
               const std::vector<MatrixOver<Field>>& targets) {
    if (targets.empty()) return true;
    if (spanning_set.empty()) return false;
    SpanBasis<Field> span(field, spanning_set.front().entry_count());
    for (const MatrixOver<Field>& element : spanning_set) span.try_add(element);
    for (const MatrixOver<Field>& target : targets) {
        if (!span.contains(target)) return false;
    }
    return true;
}

/// Whether `candidate` lies outside the span of `basis`.
///
/// The original wrote this as two branches reducing to the same predicate, with
/// a zero case that could never hold. Adding one vector raises the rank by at
/// most one, so a strict increase says it all.
template <class Field>
bool raises_rank(const Field& field, const std::vector<MatrixOver<Field>>& basis,
                 const MatrixOver<Field>& candidate) {
    SpanBasis<Field> span(field, candidate.entry_count());
    for (const MatrixOver<Field>& element : basis) span.try_add(element);
    return !span.contains(candidate);
}

/// Solve `coefficients * rows = target` for rows that are linearly independent,
/// so the solution is unique when it exists. False when `target` is outside
/// their span.
///
/// Every operation goes through the field, which is the point: the original
/// solved the same system in machine integers on one side and in doubles on the
/// other, reducing afterwards in both cases.
template <class Field>
bool solve_in_row_space(const Field& field,
                        const std::vector<std::vector<typename Field::Element>>& rows,
                        const std::vector<typename Field::Element>& target,
                        std::vector<typename Field::Element>& coefficients) {
    using Element = typename Field::Element;
    const std::size_t unknowns = rows.size();
    if (unknowns == 0) {
        coefficients.clear();
        for (const Element& entry : target) {
            if (!field.isZero(entry)) return false;
        }
        return true;
    }
    const std::size_t equations = target.size();

    // Augmented system: column j is rows[j] read down the equations, and the
    // last column is the target.
    MatrixOver<Field> system(equations, unknowns + 1);
    for (std::size_t equation = 0; equation < equations; ++equation) {
        for (std::size_t unknown = 0; unknown < unknowns; ++unknown) {
            system(equation, unknown) = rows[unknown][equation];
        }
        system(equation, unknowns) = target[equation];
    }

    std::vector<std::size_t> pivot_of_unknown(unknowns, equations);  // equations means "none"
    std::size_t pivot_row = 0;
    for (std::size_t unknown = 0; unknown < unknowns && pivot_row < equations; ++unknown) {
        std::size_t found = equations;
        for (std::size_t row = pivot_row; row < equations; ++row) {
            if (!field.isZero(system(row, unknown))) {
                found = row;
                break;
            }
        }
        if (found == equations) continue;

        for (std::size_t column = 0; column <= unknowns; ++column) {
            std::swap(system(pivot_row, column), system(found, column));
        }
        Element scale;
        field.inv(scale, system(pivot_row, unknown));
        for (std::size_t column = 0; column <= unknowns; ++column) {
            field.mulin(system(pivot_row, column), scale);
        }
        for (std::size_t row = 0; row < equations; ++row) {
            if (row == pivot_row || field.isZero(system(row, unknown))) continue;
            Element factor;
            field.neg(factor, system(row, unknown));
            for (std::size_t column = 0; column <= unknowns; ++column) {
                field.axpyin(system(row, column), factor, system(pivot_row, column));
            }
        }
        pivot_of_unknown[unknown] = pivot_row;
        ++pivot_row;
    }

    // An equation left reading 0 == nonzero means the target is outside the span.
    for (std::size_t row = pivot_row; row < equations; ++row) {
        if (!field.isZero(system(row, unknowns))) return false;
    }

    coefficients.assign(unknowns, Element());
    for (std::size_t unknown = 0; unknown < unknowns; ++unknown) {
        if (pivot_of_unknown[unknown] != equations) {
            coefficients[unknown] = system(pivot_of_unknown[unknown], unknowns);
        }
    }
    return true;
}

/// Whether two matrices have the same row space.
///
/// Sparsifying an operator means rewriting it as U V for an invertible V, which
/// on the transposed form is exactly replacing rows by other vectors spanning
/// the same space. A search that returned something genuinely sparser but no
/// longer equivalent would otherwise look like a triumph.
template <class Field>
bool same_row_space(const Field& field, const MatrixOver<Field>& left,
                    const MatrixOver<Field>& right) {
    if (left.columns() != right.columns()) return false;

    SpanBasis<Field> left_span(field, left.columns());
    for (std::size_t row = 0; row < left.rows(); ++row) left_span.try_add(left.row(row));
    SpanBasis<Field> right_span(field, right.columns());
    for (std::size_t row = 0; row < right.rows(); ++row) right_span.try_add(right.row(row));
    if (left_span.dimension() != right_span.dimension()) return false;

    for (std::size_t row = 0; row < right.rows(); ++row) {
        if (!left_span.contains(right.row(row))) return false;
    }
    for (std::size_t row = 0; row < left.rows(); ++row) {
        if (!right_span.contains(left.row(row))) return false;
    }
    return true;
}

template <class Field>
MatrixOver<Field> transpose(const MatrixOver<Field>& matrix) {
    MatrixOver<Field> result(matrix.columns(), matrix.rows());
    for (std::size_t row = 0; row < matrix.rows(); ++row) {
        for (std::size_t column = 0; column < matrix.columns(); ++column) {
            result(column, row) = matrix(row, column);
        }
    }
    return result;
}

template <class Field>
MatrixOver<Field> multiply(const Field& field, const MatrixOver<Field>& left,
                           const MatrixOver<Field>& right) {
    MatrixOver<Field> result(left.rows(), right.columns());
    for (std::size_t row = 0; row < left.rows(); ++row) {
        for (std::size_t inner = 0; inner < left.columns(); ++inner) {
            if (field.isZero(left(row, inner))) continue;
            for (std::size_t column = 0; column < right.columns(); ++column) {
                field.axpyin(result(row, column), left(row, inner), right(inner, column));
            }
        }
    }
    return result;
}

/// The columns named by `chosen`, in the order given.
template <class Field>
MatrixOver<Field> select_columns(const MatrixOver<Field>& matrix,
                                 const std::vector<std::size_t>& chosen) {
    MatrixOver<Field> result(matrix.rows(), chosen.size());
    for (std::size_t row = 0; row < matrix.rows(); ++row) {
        for (std::size_t column = 0; column < chosen.size(); ++column) {
            result(row, column) = matrix(row, chosen[column]);
        }
    }
    return result;
}

/// The rows named by `chosen`, in the order given.
template <class Field>
MatrixOver<Field> select_rows(const MatrixOver<Field>& matrix,
                              const std::vector<std::size_t>& chosen) {
    MatrixOver<Field> result(chosen.size(), matrix.columns());
    for (std::size_t row = 0; row < chosen.size(); ++row) {
        for (std::size_t column = 0; column < matrix.columns(); ++column) {
            result(row, column) = matrix(chosen[row], column);
        }
    }
    return result;
}

/// Exact inverse of a square matrix, false if it is singular.
///
/// Row j of the inverse is the combination of the rows of `square` that makes
/// the j-th standard basis vector, so this is the solver above run once per
/// row rather than a second elimination that could disagree with it.
template <class Field>
bool invert(const Field& field, const MatrixOver<Field>& square, MatrixOver<Field>& inverse) {
    using Element = typename Field::Element;
    const std::size_t order = square.rows();
    if (order != square.columns()) return false;

    std::vector<std::vector<Element>> rows;
    rows.reserve(order);
    for (std::size_t row = 0; row < order; ++row) rows.push_back(square.row(row));

    inverse = MatrixOver<Field>(order, order);
    for (std::size_t index = 0; index < order; ++index) {
        std::vector<Element> target(order, Element());
        field.assign(target[index], field.one);
        std::vector<Element> coefficients;
        if (!solve_in_row_space(field, rows, target, coefficients)) return false;
        for (std::size_t column = 0; column < order; ++column) {
            inverse(index, column) = coefficients[column];
        }
    }
    return true;
}

/// Write `matrix` as a sum of exactly rank(matrix) rank-one matrices.
///
/// A rank-one bilinear form is one multiplication, so this is what turns a map
/// into the products that compute it.
template <class Field>
std::vector<MatrixOver<Field>> rank_one_decomposition(const Field& field,
                                                      const MatrixOver<Field>& matrix) {
    using Element = typename Field::Element;

    // A maximal independent set of rows, in order: their span is the row space,
    // so every row is a combination of them.
    std::vector<std::vector<Element>> basis_rows;
    for (std::size_t row = 0; row < matrix.rows(); ++row) {
        std::vector<Element> entries = matrix.row(row);
        std::vector<Element> unused;
        if (!solve_in_row_space(field, basis_rows, entries, unused)) {
            basis_rows.push_back(std::move(entries));
        }
    }

    // matrix == coefficients * basis_rows, so term j is column j of the
    // coefficients against basis row j: an outer product, hence rank one.
    std::vector<MatrixOver<Field>> terms(basis_rows.size(),
                                         MatrixOver<Field>(matrix.rows(), matrix.columns()));
    for (std::size_t row = 0; row < matrix.rows(); ++row) {
        std::vector<Element> coefficients;
        solve_in_row_space(field, basis_rows, matrix.row(row), coefficients);
        for (std::size_t term = 0; term < basis_rows.size(); ++term) {
            for (std::size_t column = 0; column < matrix.columns(); ++column) {
                field.axpyin(terms[term](row, column), coefficients[term],
                             basis_rows[term][column]);
            }
        }
    }
    return terms;
}

}  // namespace exact
