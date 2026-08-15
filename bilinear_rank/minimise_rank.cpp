#include "minimise_rank.h"

#include "measures.h"
#include "smallest_basis.h"
#include "span_basis.h"

namespace bilinear_rank {

namespace {

/// The basis of `slices` with one more map thrown in: the answer to "what would
/// adopting this candidate cost?".
std::vector<Matrix> basis_with(const Field& field, const std::vector<Matrix>& slices,
                               const Matrix& candidate) {
    std::vector<Matrix> enlarged = slices;
    enlarged.push_back(candidate);
    return smallest_basis(field, enlarged);
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

    std::vector<Matrix> selected;
    std::vector<Matrix> remaining = candidates;
    for (;;) {
        Span span = linear_algebra::span_of(field, slices);
        for (const Matrix& kept : selected) span.try_add(kept);

        bool pruned = false;
        for (std::size_t index = 0; index < remaining.size(); ++index) {
            if (span.contains(remaining[index])) continue;

            const std::vector<Matrix> attempt = basis_with(field, slices, remaining[index]);
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
        Span span = linear_algebra::span_of(field, slices);
        bool pruned = false;

        for (std::size_t index = 0; index < candidates.size(); ++index) {
            if (span.contains(candidates[index])) continue;

            std::vector<Matrix> attempt = basis_with(field, slices, candidates[index]);
            if (linear_algebra::multiplication_count(field, attempt) <
                linear_algebra::multiplication_count(field, slices)) {
                slices = std::move(attempt);
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
