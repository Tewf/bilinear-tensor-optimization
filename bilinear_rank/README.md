# How few multiplications does a product need?

The rank of a bilinear map is the number of multiplications needed to compute
it. Strassen doing 2×2 matrix multiplication in seven instead of eight is where
fast matrix multiplication comes from, and finding such decompositions in
general is open.

There are four ways to go at it here, and the filenames say which is which: a
heuristic that descends, a complete search that decides, a walk that moves a
decomposition sideways, and an encoding that hands the question to somebody else's
solver. Each has a different guarantee, and the whole list of modules with one
sentence each is **[`modules.md`](modules.md)**. Every claim below is one of those
four earning or failing to earn its keep.

## The tools

```sh
minimise-rank      fixtures/f3_3x6.tensor          # heuristic: make it better
minimise-rank      fixtures/f2_5x5.tensor --emit-operators out   # ...and write L and R
decide-rank        fixtures/f2_5x5.tensor --target 11  # exact: is there one this small?
walk-scheme        fixtures/f3_3x6.tensor --from 10 # walk on from the heuristic's answer
decide-rank-by-ilp fixtures/f2_2x2.tensor --target 3 # the same question as a MILP
make-tensor        --field 2 1 1 1                 # build GF(4) multiplication
```

## What the heuristic reaches

Times are cumulative and sequential on one core of an i5-12450H at 2.2 GHz, the
machine Table 1 was measured on. Numbers: [`results.json`](results.json).

| Map | Naive | Step 1 | Step 2 | Step 3 | Internship |
|---|---|---|---|---|---|
| F2 5×5 | 25 | 16 | 14 | **14** · 1.41 s | 14 · 14.42 s |
| F2 3×8 | 24 | 19 | 16 | **15** · 3.35 s | 15 · 3460.54 s |
| F2 4×7 | 28 | 19 | 16 | **16** · 9.69 s | 16 · 5044.06 s |
| F3 3×6 | 18 | 12 | 11 | **10** · 7.49 s | 11, *did not finish* |

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
| F2 5×5 | **exactly 13** | no 9, 10, 11 or 12 exists, each ruled out exhaustively, and 13 is reached in the literature |

The last is settled now. The internship could bound 5×5 from above only. Ruling
out 12 took **146 402 553 nodes and 3 610 s on eight threads**, which puts the
rank at 13 or more, and Table 1 of Barbulescu, Detrey, Estibals and Zimmermann
(2012) gives 27 decompositions at 13. So the rank is exactly 13, and the
heuristic's 14 is not optimal. [`method.md`](method.md) says where the cost is
and what would cut it.

## Step 1 is not a heuristic

Choosing a basis of `span(T)` with the least total rank is a **matroid**
problem: independence of vectors is a matroid, and greedy-by-ascending-weight
gives a minimum-weight basis (Rado-Edmonds). So `16, 19, 19, 12` are not merely
good, they are the minima over all bases of those spans, and no tie-break
changes them. What is heuristic is the *constraint* that the answer be a basis
of `span(T)` at all, which is exactly what steps 2 and 3 relax.

## What makes a result trustworthy

A search that quietly loses a slice reports excellent numbers, so after every
step, in the tools and not only in the tests, the result must still generate the
map it came from. Every result here does, and the recovered ⟨L, R, P⟩ is
rebuilt and compared against the input map.

## Beyond polynomial multiplication

The same two searches, run on the tensors the complexity literature argues
about: **[`famous_tensors.md`](famous_tensors.md)**. The short version is that
rank ⟨2,2,2⟩ = 7 is decided here in half a second, Strassen and Winograd both,
and that the heuristic cannot improve a matrix multiplication tensor at all.

## Where this stops

The heuristic proves nothing optimal. The exact search proves a great deal but
only where it can finish, and from scratch it is `C(|pool|, k)`. Nothing here
settles the bilinear rank problem, which is still open.
