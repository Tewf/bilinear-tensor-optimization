#pragma once

#include <utility>
#include <vector>

#include "exhaustive_search.h"
#include "types.h"

/// Which `k` to ask about, given a search that can decide one `k` at a time.
///
/// [`exhaustive_search.h`](exhaustive_search.h) answers "is there an algorithm
/// with exactly this many products?". Turning that into "how few products are
/// there?" is a separate decision with a separate cost, and three answers to it
/// live here: sweep upward, bisect, or build from nothing.
namespace bilinear_rank {

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
