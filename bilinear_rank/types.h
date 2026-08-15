#pragma once

#include "linear_algebra.h"

/// Finding a bilinear map's rank: how few multiplications a product needs.
///
/// Three kinds of thing live here, and the filenames say which is which:
///
/// - [`heuristic_search.h`](heuristic_search.h) — greedy, no guarantee, and
///   Mohamed's own method. Its first step is the exception: choosing a
///   minimum-rank basis is a matroid problem, so the greedy is provably optimal
///   for that step.
/// - [`exhaustive_search.h`](exhaustive_search.h) — complete and exponential,
///   an implementation of a pre-existing published algorithm.
/// - [`algorithm_recovery.h`](algorithm_recovery.h) — turning either one's
///   answer back into the algorithm it stands for.
namespace bilinear_rank {

/// One field for the whole strand: the primes in play are tiny.
using Field = linear_algebra::ModularField;
using Matrix = linear_algebra::ModularMatrix;
using Element = Field::Element;

}  // namespace bilinear_rank
