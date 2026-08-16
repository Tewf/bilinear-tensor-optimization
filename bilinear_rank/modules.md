# Which file does what

Every module of the rank strand, one sentence each. The header is the authority in
all cases: this is a map, not a second copy of it. What the strand reaches and
whether the numbers can be trusted is [`README.md`](README.md); the algorithms
stated precisely with their costs are [`method.md`](method.md).

## The two searches, and choosing what to ask them

| File | Role |
|---|---|
| [`smallest_basis.h`](smallest_basis.h) | Step 1: the minimum-rank basis of a fixed span, exact because the problem is a matroid |
| [`minimise_rank.h`](minimise_rank.h) | Steps 2 and 3: first-improvement descent that relaxes step 1's constraint, guaranteeing nothing |
| [`exhaustive_search.h`](exhaustive_search.h) | The complete decision for one `k`, so a refusal is a fact about the problem |
| [`fewest_products.h`](fewest_products.h) | Which `k` to ask about: the flattening bound to start at, sweep or bisection or from nothing, and the gap a finder reports |
| [`rank_one_basis.h`](rank_one_basis.h) | The question at every leaf of both searches: has this subspace a basis of rank-one maps? |
| [`span_enumeration.h`](span_enumeration.h) | Walking a map's `p^k` span, which is what decides how far step 1 scales |
| [`candidate_pool.h`](candidate_pool.h) | The rank-one maps to search over |
| [`map_construction.h`](map_construction.h) | The maps to search on |
| [`algorithm_recovery.h`](algorithm_recovery.h) | Turning either search's answer back into the algorithm ⟨L, R, P⟩ it stands for |

## Symmetry, which is where the saving is

A change of coordinates on each operand preserves rank, so if it also fixes the
target subspace it maps solutions to solutions and only one per orbit need be
visited. Seven files, because the group, the orbits and each consumer of them are
separate jobs.

| File | Role |
|---|---|
| [`automorphism.h`](automorphism.h) | The group itself: the rank-preserving action, and the stabiliser of a subspace |
| [`group_construction.h`](group_construction.h) | Where the groups come from, by brute force and by closed form, each checking the other |
| [`pool_orbits.h`](pool_orbits.h) | The orbits of the rank-one pool, found on the operand vectors rather than on their products |
| [`orbit_search.h`](orbit_search.h) | The exact search with its tree quotiented: one branch per orbit |
| [`orbit_heuristic.h`](orbit_heuristic.h) | Steps 2 and 3 against a quotiented pool |
| [`orbit_cubes.h`](orbit_cubes.h) | The first term fixed to one representative per orbit, for a solver to split on. Its boundary with the encoder is a contract: [`orbit_cube_boundary.md`](orbit_cube_boundary.md) |
| [`orbit_plan.md`](orbit_plan.md) | Why the family is arranged this way |

## Moving a decomposition instead of building one

| File | Role |
|---|---|
| [`flip_graph.h`](flip_graph.h) | The flip graph of `[kauers2023]`: start from a decomposition that works and walk it |
| [`plateau_search.h`](plateau_search.h) | New here. A walk allowed to cross plateaus, filed apart because it guarantees something different from steps 2 and 3 |

## Handing the question to somebody else's solver

| File | Role |
|---|---|
| [`integer_programme_encoding.h`](integer_programme_encoding.h) | Brent's equations as an integer programme, for a MILP solver. The only part of this strand that links [`../optimisation/`](../optimisation/), and kept out of the library so nothing else does |

The SAT and SMT route is a strand of its own,
[`../satisfiability/`](../satisfiability/).

## Plumbing both searches need

| File | Role |
|---|---|
| [`memory_budget.h`](memory_budget.h) | The one place that decides how much a run may ask for, since both bulk allocations grow exponentially in the shape |
| [`parallel.h`](parallel.h) | Independent work across cores, one worker by default so a run reproduces what was published |
| [`types.h`](types.h) | The field and matrix aliases the strand spells once |
