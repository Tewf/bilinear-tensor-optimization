#pragma once

#include <atomic>
#include <cstddef>
#include <vector>

#include "bilinear_rank_aliases.h"

/// The exact search: not "can this be improved?" but "is there one of size k?".
///
/// This is an implementation of a pre-existing algorithm, as the original's own
/// docstring says, unlike [the heuristic](minimise_rank.h), which is
/// Mohamed's. It is complete for the question it asks, and exponential.
///
/// **What it decides, precisely.** Given a subspace `W` already containing the
/// map, it decides whether `W` can be extended to a space of dimension `k` that
/// has a basis made entirely of rank-one maps, which is a `k`-multiplication
/// algorithm. Sweeping `k` upward gives the fewest products *among
/// decompositions containing `W`*. Starting from `W = {}` would give the true
/// minimum, and costs `C(|pool|, k)`: around 10^30 for the fixtures here, so it
/// is offered and guarded rather than promised.
namespace bilinear_rank {

/// How much of the tree a search was allowed, and how much it used.
///
/// Without this an infeasible question is indistinguishable from a slow
/// machine. `exhausted` false on a negative answer means "no solution"; true
/// means "gave up", and those are very different claims.
struct SearchBudget {
    explicit SearchBudget(std::size_t limit = 5'000'000) : node_limit(limit) {}

    std::size_t node_limit;
    std::atomic<std::size_t> nodes_visited{0};
    std::atomic<bool> exhausted{true};  // false once the limit is hit

    /// Atomic because workers share one budget, and written as a compare and
    /// exchange rather than a fetch and add so that a refused node is not
    /// counted: the node totals this repository publishes have to mean the same
    /// thing on one thread and on twelve.
    bool spend() {
        std::size_t seen = nodes_visited.load(std::memory_order_relaxed);
        do {
            if (seen >= node_limit) {
                exhausted.store(false, std::memory_order_relaxed);
                return false;
            }
        } while (!nodes_visited.compare_exchange_weak(seen, seen + 1, std::memory_order_relaxed));
        return true;
    }
};

/// The rank-one maps of `pool` lying inside the span of `subspace`, taken
/// greedily so they stay independent.
///
/// When there are as many as the subspace has dimensions, they are a rank-one
/// basis of it, and a rank-one basis is an algorithm.
std::vector<Matrix> rank_one_maps_within(const Field& field, const std::vector<Matrix>& subspace,
                                         const std::vector<Matrix>& pool);

/// Extend `subspace` with maps from `pool` (index `from` onward) to dimension
/// exactly `target`, such that the result has a rank-one basis.
///
/// On success `products` holds that basis. The original returned the extended
/// subspace from its base case and the rank-one basis from its recursive one;
/// only the second is usable, since the caller needs the products.
bool expand_subspace(const Field& field, const std::vector<Matrix>& subspace,
                     const std::vector<Matrix>& pool, std::size_t from, std::size_t target,
                     SearchBudget& budget, std::vector<Matrix>& products);

}  // namespace bilinear_rank
