#pragma once

#include <vector>

#include "bilinear_rank_aliases.h"

/// Steps 2 and 3: recombining a map with rank-one maps from outside its own
/// basis, which is where the heuristic stops guaranteeing anything.
///
/// Adopt a candidate whenever the basis of the enlarged span costs fewer
/// multiplications than the map costs now. Enlarging is allowed: the result only
/// has to *generate* the original map, so a spanning set of more slices but
/// lower total rank is a better answer, which is exactly what Karatsuba's five
/// products for a four-coefficient product is.
///
/// The two steps differ only in the pool they are handed: step 2 uses the map's
/// own rank-one products, step 3 every rank-one map of the shape.
namespace bilinear_rank {

/// Keep only the candidates that, taken one at a time, would improve the map.
///
/// Named for what it does. The original called this `filter`, which shadowed
/// Julia's own `filter`, still called elsewhere in the same file.
std::vector<Matrix> improving_candidates(const Field& field, const std::vector<Matrix>& slices,
                                         const std::vector<Matrix>& candidates);

/// Adopt improvements until no candidate is left that pays.
std::vector<Matrix> minimise_rank(const Field& field, std::vector<Matrix> slices,
                                  std::vector<Matrix> candidates);

}  // namespace bilinear_rank
