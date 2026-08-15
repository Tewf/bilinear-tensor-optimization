#include "smallest_basis.h"

#include <algorithm>

#include "measures.h"
#include "span_basis.h"
#include "span_enumeration.h"

namespace bilinear_rank {

std::vector<Matrix> smallest_basis(const Field& field, const std::vector<Matrix>& slices) {
    const std::size_t width = linear_algebra::flattened_width<Field>(slices);
    const std::size_t dimension = linear_algebra::span_of(field, slices).dimension();
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
        Matrix matrix = combine(field, slices,
                                coefficient_vector(index, slices.size(), field.characteristic()));
        candidates.push_back({linear_algebra::rank(field, matrix), index, std::move(matrix)});
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
    Span span(field, width);
    for (const Candidate& candidate : candidates) {
        if (basis.size() == dimension) break;
        if (span.try_add(candidate.matrix)) {
            basis.push_back(candidate.matrix);
        }
    }
    return basis;
}

}  // namespace bilinear_rank
