#include "search.h"

#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace rank_search {

Matrix combine(const Field& field, const std::vector<Matrix>& slices,
               const std::vector<int64_t>& coefficients) {
    if (slices.empty()) return Matrix();
    Matrix result(slices.front().rows(), slices.front().columns());
    for (std::size_t index = 0; index < slices.size(); ++index) {
        if (coefficients[index] == 0) continue;
        for (std::size_t entry = 0; entry < result.entry_count(); ++entry) {
            // r += c * s, in the field. The original accumulated in plain
            // integers and reduced only when it next took a rank.
            field.axpyin(result.data()[entry], coefficients[index], slices[index].data()[entry]);
        }
    }
    return result;
}

std::vector<int64_t> coefficient_vector(std::size_t index, std::size_t count,
                                        int64_t characteristic) {
    std::vector<int64_t> coefficients(count);
    for (std::size_t position = 0; position < count; ++position) {
        coefficients[position] = static_cast<int64_t>(index % static_cast<std::size_t>(characteristic));
        index /= static_cast<std::size_t>(characteristic);
    }
    return coefficients;
}

std::size_t span_size(const Field& field, std::size_t slice_count) {
    const auto characteristic = static_cast<std::size_t>(field.characteristic());
    std::size_t size = 1;
    for (std::size_t step = 0; step < slice_count; ++step) {
        if (size > std::size_t(1) << 40) {
            throw std::runtime_error("span too large to enumerate exhaustively");
        }
        size *= characteristic;
    }
    return size;
}

namespace {

exact::SpanBasis span_of(const Field& field, std::size_t width, const std::vector<Matrix>& parts) {
    exact::SpanBasis span(field, width);
    for (const Matrix& part : parts) span.try_add(part);
    return span;
}

std::size_t entry_width(const std::vector<Matrix>& slices) {
    return slices.empty() ? 0 : slices.front().entry_count();
}

}  // namespace

std::vector<Matrix> smallest_basis(const Field& field, const std::vector<Matrix>& slices) {
    const std::size_t width = entry_width(slices);
    const std::size_t dimension = span_of(field, width, slices).dimension();
    const std::size_t combinations = span_size(field, slices.size());

    // Every element of the span, cheapest first. Index 0 is the zero
    // combination and is skipped: it can never enter a basis.
    struct Candidate {
        std::size_t rank;
        std::size_t index;
        Matrix matrix;
    };
    std::vector<Candidate> candidates;
    candidates.reserve(combinations - 1);
    for (std::size_t index = 1; index < combinations; ++index) {
        Matrix matrix = combine(field, slices, coefficient_vector(index, slices.size(),
                                                                  field.characteristic()));
        candidates.push_back({exact::rank(field, matrix), index, std::move(matrix)});
    }

    // Sort by rank, ties broken by enumeration order. Julia's default sort is
    // not stable, so this pins down a choice the original left to the sorting
    // algorithm.
    std::sort(candidates.begin(), candidates.end(),
              [](const Candidate& left, const Candidate& right) {
                  if (left.rank != right.rank) return left.rank < right.rank;
                  return left.index < right.index;
              });

    std::vector<Matrix> basis;
    exact::SpanBasis span(field, width);
    for (const Candidate& candidate : candidates) {
        if (basis.size() == dimension) break;
        if (span.try_add(candidate.matrix)) {
            basis.push_back(candidate.matrix);
        }
    }
    return basis;
}

std::vector<Matrix> rank_one_candidates(const Field& field, const std::vector<Matrix>& slices) {
    std::vector<Matrix> candidates;
    for (const Matrix& slice : slices) {
        for (Matrix& term : exact::rank_one_decomposition(field, slice)) {
            candidates.push_back(std::move(term));
        }
    }
    return candidates;
}

namespace {

/// Nonzero vectors whose leading nonzero entry is 1: exactly one per scalar
/// class, so their outer products enumerate the rank-one maps without repeats.
std::vector<std::vector<int64_t>> normalised_vectors(const Field& field, std::size_t length) {
    const auto characteristic = static_cast<std::size_t>(field.characteristic());
    std::vector<std::vector<int64_t>> vectors;
    for (std::size_t leading = 0; leading < length; ++leading) {
        std::size_t combinations = 1;
        for (std::size_t position = leading + 1; position < length; ++position) {
            combinations *= characteristic;
        }
        for (std::size_t index = 0; index < combinations; ++index) {
            std::vector<int64_t> vector(length, 0);
            vector[leading] = 1;
            std::size_t remaining = index;
            for (std::size_t position = leading + 1; position < length; ++position) {
                vector[position] = static_cast<int64_t>(remaining % characteristic);
                remaining /= characteristic;
            }
            vectors.push_back(std::move(vector));
        }
    }
    return vectors;
}

}  // namespace

std::vector<Matrix> all_rank_one_maps(const Field& field, std::size_t rows, std::size_t columns) {
    const std::vector<std::vector<int64_t>> lefts = normalised_vectors(field, rows);
    const std::vector<std::vector<int64_t>> rights = normalised_vectors(field, columns);

    std::vector<Matrix> maps;
    maps.reserve(lefts.size() * rights.size());
    for (const std::vector<int64_t>& left : lefts) {
        for (const std::vector<int64_t>& right : rights) {
            Matrix map(rows, columns);
            for (std::size_t row = 0; row < rows; ++row) {
                for (std::size_t column = 0; column < columns; ++column) {
                    field.mul(map(row, column), left[row], right[column]);
                }
            }
            maps.push_back(std::move(map));
        }
    }
    return maps;
}

std::vector<Matrix> improving_candidates(const Field& field, const std::vector<Matrix>& slices,
                                         const std::vector<Matrix>& candidates) {
    const std::size_t width = entry_width(slices);
    const std::size_t baseline = exact::multiplication_count(field, slices);

    std::vector<Matrix> selected;
    std::vector<Matrix> remaining = candidates;
    for (;;) {
        exact::SpanBasis span = span_of(field, width, slices);
        for (const Matrix& kept : selected) span.try_add(kept);

        bool pruned = false;
        for (std::size_t index = 0; index < remaining.size(); ++index) {
            if (span.contains(remaining[index])) continue;

            std::vector<Matrix> enlarged = slices;
            enlarged.push_back(remaining[index]);
            const std::vector<Matrix> rewritten = smallest_basis(field, enlarged);

            if (exact::multiplication_count(field, rewritten) < baseline) {
                span.try_add(remaining[index]);
                selected.push_back(remaining[index]);
                continue;
            }

            // This one does not pay. Drop everything already spanned by the
            // basis it produced, and start again on what is left.
            exact::SpanBasis reached = span_of(field, width, rewritten);
            for (const Matrix& kept : selected) reached.try_add(kept);

            std::vector<Matrix> survivors;
            for (std::size_t later = index + 1; later < remaining.size(); ++later) {
                if (!reached.contains(remaining[later])) survivors.push_back(remaining[later]);
            }
            remaining = std::move(survivors);
            pruned = true;
            break;
        }
        if (!pruned) return selected;
    }
}

std::vector<Matrix> minimise_rank(const Field& field, std::vector<Matrix> slices,
                                  std::vector<Matrix> candidates) {
    const std::size_t width = entry_width(slices);

    for (;;) {
        exact::SpanBasis span = span_of(field, width, slices);
        bool pruned = false;

        for (std::size_t index = 0; index < candidates.size(); ++index) {
            if (span.contains(candidates[index])) continue;

            std::vector<Matrix> enlarged = slices;
            enlarged.push_back(candidates[index]);
            std::vector<Matrix> rewritten = smallest_basis(field, enlarged);

            if (exact::multiplication_count(field, rewritten) <
                exact::multiplication_count(field, slices)) {
                slices = std::move(rewritten);
                span = span_of(field, width, slices);
                continue;
            }

            exact::SpanBasis reached = span_of(field, width, rewritten);
            std::vector<Matrix> survivors;
            for (std::size_t later = index + 1; later < candidates.size(); ++later) {
                if (!reached.contains(candidates[later])) survivors.push_back(candidates[later]);
            }
            candidates = std::move(survivors);
            pruned = true;
            break;
        }
        if (!pruned) return slices;
    }
}

}  // namespace rank_search
