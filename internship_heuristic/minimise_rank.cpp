#include "minimise_rank.h"

#include "measures.h"
#include "smallest_basis.h"
#include "span_basis.h"

namespace bilinear_rank {

namespace {

/// The basis of `slices` with one more map thrown in: the answer to "what would
/// adopting this candidate cost?".
///
/// `known` is the ranks of the span of `slices`, which every candidate shares:
/// see `smallest_basis`. Half the enumeration over GF(2), and a third over
/// GF(3), is the same work repeated once per candidate without it.
std::vector<Matrix> basis_with(const Field& field, const std::vector<Matrix>& slices,
                               const Matrix& candidate, const std::vector<std::size_t>& known) {
    std::vector<Matrix> enlarged = slices;
    enlarged.push_back(candidate);
    return smallest_basis(field, enlarged, known);
}

/// What is left worth trying after a candidate failed to pay.
///
/// The basis that attempt produced spans everything the candidate could have
/// reached, so any later candidate already inside it would fail the same way and
/// is dropped unexamined. This is the irreversible pruning that makes the step a
/// heuristic: a candidate discarded here is never reconsidered, even though a
/// different order might have adopted it.
std::vector<Matrix> survivors_after(const Field& field, const std::vector<Matrix>& attempt,
                                    const std::vector<Matrix>& also_reached,
                                    const std::vector<Matrix>& candidates, std::size_t index) {
    Span reached = linear_algebra::span_of(field, attempt);
    for (const Matrix& kept : also_reached) reached.try_add(kept);

    std::vector<Matrix> survivors;
    for (std::size_t later = index + 1; later < candidates.size(); ++later) {
        if (!reached.contains(candidates[later])) survivors.push_back(candidates[later]);
    }
    return survivors;
}

}  // namespace

std::vector<Matrix> improving_candidates(const Field& field, const std::vector<Matrix>& slices,
                                         const std::vector<Matrix>& candidates) {
    const std::size_t baseline = linear_algebra::multiplication_count(field, slices);
    const std::vector<std::size_t> known = span_element_ranks(field, slices);

    std::vector<Matrix> selected;
    std::vector<Matrix> remaining = candidates;
    for (;;) {
        Span span = linear_algebra::span_of(field, slices);
        for (const Matrix& kept : selected) span.try_add(kept);

        bool pruned = false;
        for (std::size_t index = 0; index < remaining.size(); ++index) {
            if (span.contains(remaining[index])) continue;

            const std::vector<Matrix> attempt = basis_with(field, slices, remaining[index], known);
            if (linear_algebra::multiplication_count(field, attempt) < baseline) {
                span.try_add(remaining[index]);
                selected.push_back(remaining[index]);
                continue;
            }

            remaining = survivors_after(field, attempt, selected, remaining, index);
            pruned = true;
            break;
        }
        if (!pruned) return selected;
    }
}

std::vector<Matrix> minimise_rank(const Field& field, std::vector<Matrix> slices,
                                  std::vector<Matrix> candidates) {
    for (;;) {
        // Recomputed whenever the map moves, which is once per improvement
        // adopted, against once per candidate tried.
        std::vector<std::size_t> known = span_element_ranks(field, slices);
        Span span = linear_algebra::span_of(field, slices);
        bool pruned = false;

        for (std::size_t index = 0; index < candidates.size(); ++index) {
            if (span.contains(candidates[index])) continue;

            std::vector<Matrix> attempt = basis_with(field, slices, candidates[index], known);
            if (linear_algebra::multiplication_count(field, attempt) <
                linear_algebra::multiplication_count(field, slices)) {
                slices = std::move(attempt);
                known = span_element_ranks(field, slices);
                span = linear_algebra::span_of(field, slices);
                continue;
            }

            candidates = survivors_after(field, attempt, {}, candidates, index);
            pruned = true;
            break;
        }
        if (!pruned) return slices;
    }
}

}  // namespace bilinear_rank
