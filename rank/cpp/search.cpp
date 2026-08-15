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

std::vector<Matrix> smallest_basis(const Field& field, const std::vector<Matrix>& slices) {
    const std::size_t dimension = exact::rank_of_span(field, slices);
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
    for (const Candidate& candidate : candidates) {
        if (basis.size() == dimension) break;
        if (exact::raises_rank(field, basis, candidate.matrix)) {
            basis.push_back(candidate.matrix);
        }
    }
    return basis;
}

}  // namespace rank_search
