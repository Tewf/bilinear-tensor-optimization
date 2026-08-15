# Fixtures

The four bilinear maps the internship measured, written out in full so that the
C++ and the Julia are checked against the same bytes rather than against two
implementations of the same loop.

Each is the tensor of polynomial multiplication: `A(x)` with `n` coefficients
times `B(x)` with `m` coefficients over `GF(p)`, giving `n+m-1` output
coefficients. Slice `i` is the matrix of the bilinear form producing `c_i`, so it
carries a 1 at `(j,k)` exactly when `j+k == i`. Computing every product
separately costs `n*m` multiplications, which is the rank to beat.

The format is deliberately dull: `field p`, `shape slices rows cols`, then the
slices as dense rows separated by blank lines, `#` for comments. The shift
structure is meant to be visible when you open the file.

## What the original measured

Table 1 of [`../original/Bilinear_Rank_over_Finite_Field/bilinear_rank_problem_heuristic.pdf`](../original/Bilinear_Rank_over_Finite_Field/bilinear_rank_problem_heuristic.pdf),
in Julia, single core of a 12th Gen i5-12450H at 2.2 GHz. Times are cumulative
totals, so each step includes the ones before it.

| Fixture | Field | Product | Naive | Step 1 | Step 2 | Step 3 |
|---|---|---|---|---|---|---|
| `f2_5x5` | F2 | 5×5 | 25 | 16 · 0.20 s | 14 · 5.48 s | **14** · 14.42 s |
| `f2_3x8` | F2 | 3×8 | 24 | 19 · 0.15 s | 16 · 28.83 s | **15** · 3460.54 s |
| `f2_4x7` | F2 | 4×7 | 28 | 19 · 0.19 s | 16 · 24.0 s | **16** · 5044.06 s |
| `f3_3x6` | F3 | 3×6 | 18 | 12 · 0.69 s | 11 · 18.05 s | — · *did not finish* |

The three steps are greedy smallest-basis (1), rank minimisation over the rank-1
maps already inside `T` (2), and rank minimisation over the full generated set
`G` of rank-1 maps (3).

## What that table actually says

**Step 3 improved the answer once in four attempts.** It found 16→15 on `f2_3x8`
and paid about 3432 seconds for it. On `f2_4x7` it ran 5020 seconds and returned
the 16 that Step 2 already had; on `f2_5x5` it confirmed 14; on `f3_3x6` it never
terminated, so the published 11 is a Step 2 figure from a run that was abandoned
rather than a finished result.

So Step 3 accounts for essentially all of the cost and almost none of the
answer. Any continuation that only makes Step 3 faster is optimising the part
that mostly does not pay. That is the finding these fixtures exist to hold
still.

## Using them

The expected numbers live here rather than in the test code, so a test that
disagrees with the internship is a visible disagreement with this table. Step 1
and Step 2 are cheap enough to assert everywhere. Step 3 is not: `f2_3x8` and
`f2_4x7` are tagged `slow` and stay out of CI, and `f3_3x6` has no Step 3 target
to assert at all.
