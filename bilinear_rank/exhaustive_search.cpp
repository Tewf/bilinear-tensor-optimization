#include "exhaustive_search.h"

#include <algorithm>

#include "span_basis.h"

namespace bilinear_rank {

namespace {

/// The rank-one maps of `pool` inside a span already built, taken greedily so
/// they stay independent.
std::vector<Matrix> rank_one_maps_within_span(const Field& field,
                                              const Span& reachable,
                                              std::size_t width, const std::vector<Matrix>& pool,
                                              std::size_t needed,
                                              std::vector<Element>& scratch) {
    std::vector<Matrix> found;
    Span independent(field, width);
    for (std::size_t index = 0; index < pool.size(); ++index) {
        // Once what is left cannot reach the target, the answer is already no.
        if (found.size() + (pool.size() - index) < needed) break;
        if (!reachable.contains(pool[index], scratch)) continue;
        if (independent.try_add(pool[index])) {
            found.push_back(pool[index]);
            if (found.size() == needed) break;
        }
    }
    return found;
}

}  // namespace

std::vector<Matrix> rank_one_maps_within(const Field& field, const std::vector<Matrix>& subspace,
                                         const std::vector<Matrix>& pool) {
    if (subspace.empty()) return {};
    const std::size_t width = linear_algebra::flattened_width<Field>(subspace);
    std::vector<Element> scratch;
    return rank_one_maps_within_span(field, linear_algebra::span_of(field, subspace), width, pool,
                                     pool.size(), scratch);
}

namespace {

/// The search proper, carrying the span down instead of rebuilding it.
///
/// Three things were being redone at every node: the span of the whole subspace,
/// the span again inside the candidate loop, and a full scan of the pool. Only
/// the last is needed, and only at a node that has reached the target
/// dimension. Removing the other two is what makes the depth reachable.
bool expand_subspace_impl(const Field& field, Span span,
                          std::size_t width, const std::vector<Matrix>& pool, std::size_t from,
                          std::size_t target, SearchBudget& budget, std::vector<Element>& scratch,
                          std::vector<Matrix>& products) {
    if (!budget.spend()) return false;

    const std::size_t dimension = span.dimension();
    if (dimension > target) return false;
    if (dimension == target) {
        std::vector<Matrix> within =
            rank_one_maps_within_span(field, span, width, pool, target, scratch);
        if (within.size() != target) return false;
        products = std::move(within);  // a rank-one basis of the span: the products
        return true;
    }

    for (std::size_t index = from; index < pool.size(); ++index) {
        if (span.contains(pool[index], scratch)) continue;
        Span extended = span;
        extended.try_add(pool[index]);
        if (expand_subspace_impl(field, std::move(extended), width, pool, index + 1, target, budget,
                                 scratch, products)) {
            return true;
        }
        if (!budget.exhausted) return false;  // gave up rather than ruled out
    }
    return false;
}

}  // namespace

bool expand_subspace(const Field& field, const std::vector<Matrix>& subspace,
                     const std::vector<Matrix>& pool, std::size_t from, std::size_t target,
                     SearchBudget& budget, std::vector<Matrix>& products) {
    if (subspace.empty()) return false;
    const std::size_t width = linear_algebra::flattened_width<Field>(subspace);
    std::vector<Element> scratch;
    return expand_subspace_impl(field, linear_algebra::span_of(field, subspace), width, pool, from, target,
                                budget, scratch, products);
}

bool fewest_products_by_sweep(const Field& field, const std::vector<Matrix>& base,
                              const std::vector<Matrix>& pool, SearchBudget& budget,
                              std::vector<Matrix>& products) {
    if (base.empty()) return false;
    const std::size_t lowest = linear_algebra::span_of(field, base).dimension();
    const std::size_t highest = linear_algebra::multiplication_count(field, base);

    for (std::size_t target = lowest; target <= highest; ++target) {
        if (expand_subspace(field, base, pool, 0, target, budget, products)) return true;
        if (!budget.exhausted) return false;
    }
    return false;
}

bool fewest_products_by_bisection(const Field& field, const std::vector<Matrix>& base,
                                  const std::vector<Matrix>& pool, SearchBudget& budget,
                                  std::vector<Matrix>& products) {
    if (base.empty()) return false;
    std::size_t low = linear_algebra::span_of(field, base).dimension();
    std::size_t high = linear_algebra::multiplication_count(field, base);

    std::vector<Matrix> best;
    bool found = false;
    while (low <= high) {
        const std::size_t middle = low + (high - low) / 2;
        std::vector<Matrix> attempt;
        if (expand_subspace(field, base, pool, 0, middle, budget, attempt)) {
            best = std::move(attempt);
            found = true;
            if (middle == 0) break;
            high = middle - 1;
        } else {
            if (!budget.exhausted) return false;
            low = middle + 1;
        }
    }
    if (found) products = std::move(best);
    return found;
}

std::pair<std::vector<Matrix>, std::vector<Matrix>> lowest_rank_partition(
    const Field& field, const std::vector<Matrix>& slices) {
    std::vector<Matrix> lowest;
    std::vector<Matrix> rest;
    if (slices.empty()) return {lowest, rest};

    std::size_t smallest = linear_algebra::rank(field, slices.front());
    for (const Matrix& slice : slices) {
        smallest = std::min(smallest, linear_algebra::rank(field, slice));
    }
    for (const Matrix& slice : slices) {
        if (linear_algebra::rank(field, slice) == smallest) {
            lowest.push_back(slice);
        } else {
            rest.push_back(slice);
        }
    }
    return {lowest, rest};
}

bool build_bottom_up(const Field& field, const std::vector<Matrix>& map,
                     const std::vector<Matrix>& pool, SearchBudget& budget,
                     std::vector<Matrix>& products) {
    // Absorb the map's slices in rank order, cheapest first, minimising after
    // each level. Nothing here is conditioned on a heuristic's answer.
    std::vector<Matrix> absorbed;
    auto [level, remaining] = lowest_rank_partition(field, map);

    for (;;) {
        for (const Matrix& slice : level) absorbed.push_back(slice);

        std::vector<Matrix> attempt;
        if (!fewest_products_by_bisection(field, absorbed, pool, budget, attempt)) {
            if (!budget.exhausted) return false;
        } else {
            absorbed = attempt;
        }
        if (remaining.empty()) break;
        std::tie(level, remaining) = lowest_rank_partition(field, remaining);
    }

    products = absorbed;
    return !products.empty();
}

}  // namespace bilinear_rank
