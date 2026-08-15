#pragma once

#include <cstddef>
#include <vector>

#include "types.h"

/// The exact search: not "can this be improved?" but "is there one of size k?".
///
/// This is an implementation of a pre-existing algorithm, as the original's own
/// docstring says — unlike [the heuristic](heuristic_search.h), which is
/// Mohamed's. It is complete for the question it asks, and exponential.
///
/// **What it decides, precisely.** Given a subspace `W` already containing the
/// map, it decides whether `W` can be extended to a space of dimension `k` that
/// has a basis made entirely of rank-one maps — which is a `k`-multiplication
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
    std::size_t node_limit = 5'000'000;
    std::size_t nodes_visited = 0;
    bool exhausted = true;  // false once the limit is hit

    bool spend() {
        if (nodes_visited >= node_limit) {
            exhausted = false;
            return false;
        }
        ++nodes_visited;
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

/// Sweep `k` upward from the subspace's dimension. The first hit is the fewest
/// products reachable from `base`.
///
/// Trustworthy but slow: it makes no assumption about how the answer behaves
/// in `k`.
bool fewest_products_by_sweep(const Field& field, const std::vector<Matrix>& base,
                              const std::vector<Matrix>& pool, SearchBudget& budget,
                              std::vector<Matrix>& products);

/// The same question by bisection on `k`.
///
/// Faster, and it assumes what the sweep does not: that a `k`-product solution
/// implies a `k+1`-product one. That holds whenever the pool still has a map
/// outside the current span, which is why both are kept and tested to agree.
/// The original had this too, and it could not run: it called an
/// `auto_expand_subspace` defined nowhere, and passed three arguments to a
/// five-parameter function.
bool fewest_products_by_bisection(const Field& field, const std::vector<Matrix>& base,
                                  const std::vector<Matrix>& pool, SearchBudget& budget,
                                  std::vector<Matrix>& products);

/// Split slices into those of the lowest rank present and all the rest.
std::pair<std::vector<Matrix>, std::vector<Matrix>> lowest_rank_partition(
    const Field& field, const std::vector<Matrix>& slices);

/// Build from nothing, absorbing the map's slices in rank order.
///
/// This is the from-scratch variant, so its answer is not conditioned on a
/// heuristic's output. It is also the expensive one, and the one the original
/// could not run because it depended on the broken bisection.
bool build_bottom_up(const Field& field, const std::vector<Matrix>& map,
                     const std::vector<Matrix>& pool, SearchBudget& budget,
                     std::vector<Matrix>& products);

}  // namespace bilinear_rank
