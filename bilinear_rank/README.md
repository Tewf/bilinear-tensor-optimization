# How few multiplications does a product need?

The rank of a bilinear map is the number of multiplications needed to compute
it. Strassen doing 2×2 matrix multiplication in seven instead of eight is where
fast matrix multiplication comes from, and finding such decompositions in
general is open.

There are two ways to go at it here, and the filenames say which is which.

| | Guarantee | Whose |
|---|---|---|
| [`heuristic_search.*`](heuristic_search.h) | None. Greedy, first-improvement, irreversible pruning. **Except step 1**, which is provably optimal — see below | Mohamed's |
| [`exhaustive_search.*`](exhaustive_search.h) | Complete. A "no" that ran to exhaustion is a fact about the problem | An implementation of a pre-existing published algorithm |
| [`algorithm_recovery.*`](algorithm_recovery.h) | — | Turns either answer into the algorithm ⟨L, R, P⟩ it stands for |
| [`candidate_pool.*`](candidate_pool.h) · [`map_construction.*`](map_construction.h) | — | The rank-one maps to search over, and the maps to search on |

## The tools

```sh
minimise-rank fixtures/f3_3x6.tensor              # heuristic: make it better
minimise-rank fixtures/f2_5x5.tensor --emit-operators out   # ...and write L and R
decide-rank   fixtures/f2_5x5.tensor --target 11  # exact: is there one this small?
make-tensor   --field 2 1 1 1                     # build GF(4) multiplication
```

## What the heuristic reaches

Times are cumulative and sequential on one core of an i5-12450H at 2.2 GHz — the
machine Table 1 was measured on. Numbers: [`results.json`](results.json).

| Map | Naive | Step 1 | Step 2 | Step 3 | Internship |
|---|---|---|---|---|---|
| F2 5×5 | 25 | 16 | 14 | **14** · 2.52 s | 14 · 14.42 s |
| F2 3×8 | 24 | 19 | 16 | **15** · 5.95 s | 15 · 3460.54 s |
| F2 4×7 | 28 | 19 | 16 | **16** · 17.80 s | 16 · 5044.06 s |
| F3 3×6 | 18 | 12 | 11 | **10** · 9.92 s | 11, *did not finish* |

**F3 3×6 improves on what was published**, because the internship's step 3 on
that map never terminated and the 11 is a step 2 figure from an abandoned run.

**Step 3 earns very little.** Across the four it improved the answer once. On
4×7 it spent 5020 of the internship's 5044 seconds returning the rank step 2
already had.

## What the exact search decides

| Map | Answer | |
|---|---|---|
| F2 2×2 | **exactly 3** | Karatsuba, and `2n−1` says no fewer is possible |
| F2 2×3 | **exactly 5** | the write-up's own worked example, "five instead of six" |
| GF(4) over GF(2) | **exactly 3** | classical |
| GF(8) over GF(2) | **exactly 6** | classical |
| F2 5×5 | **at least 12** | no 9, 10 or 11 exists; each ruled out exhaustively |

The last is new. The internship could bound 5×5 from above only; the true rank
is now known to lie in **12 ≤ rank ≤ 14**. Deciding 12 is `C(961, 3)` nodes,
about seven hours at the rate the 11 was ruled out at — long, not impossible,
and the next real result available here. [`method.md`](method.md) says where the
cost is and what would cut it.

## Step 1 is not a heuristic

Choosing a basis of `span(T)` with the least total rank is a **matroid**
problem: independence of vectors is a matroid, and greedy-by-ascending-weight
gives a minimum-weight basis (Rado–Edmonds). So `16, 19, 19, 12` are not merely
good, they are the minima over all bases of those spans, and no tie-break
changes them. What is heuristic is the *constraint* that the answer be a basis
of `span(T)` at all — which is exactly what steps 2 and 3 relax.

## What makes a result trustworthy

A search that quietly loses a slice reports excellent numbers, so after every
step, in the tools and not only in the tests, the result must still generate the
map it came from. Every result here does, and the recovered ⟨L, R, P⟩ is
rebuilt and compared against the input map.

## Where this stops

The heuristic proves nothing optimal. The exact search proves a great deal but
only where it can finish, and from scratch it is `C(|pool|, k)`. Nothing here
settles the bilinear rank problem, which is still open.
