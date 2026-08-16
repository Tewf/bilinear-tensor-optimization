# What it costs, measured

Every number here was produced on 2026-08-16 on one core of an i5-12450H, by
the commands in [the README](README.md), against the exhaustive searches in
[`../bilinear_rank/`](../bilinear_rank/) on the same fixtures. How the encodings
work is [`method.md`](method.md); this is only what they cost.

## The rank itself, not one `k` at a time

Sweeping upward turns the decision procedure into a rank finder, and the whole
of `⟨2,2,2⟩` costs less than a second from nothing:

```
k = 5: NO, rank is more than 5   (0.10 s)
k = 6: NO, rank is more than 6   (0.32 s)
k = 7: FOUND a decomposition     (0.18 s)      0.60 s in total
```

Strassen's seven products and Winograd's proof that six are impossible, neither
assumed, and the seven checked against the map they compute. `F₂ 2×3` the same
way is 5 after ruling out 1 through 4, in 50 ms.

## What it costs against the exhaustive search

Measured 2026-08-16 on one core, against the exhaustive searches on the same
fixtures. Every rank agrees in both directions, which is the point of having two
methods.

| Question | Exhaustive | SAT | |
|---|---|---|---|
| `⟨2,2,2⟩` find 7 | 7 436 nodes | **0.48 s** | both find Strassen |
| `⟨2,2,2⟩` rule out 6 | 25 399 nodes, 0.41 s | **0.31 s** | |
| `⟨2,2,3⟩` rule out 8 | 446 923 nodes, 53.1 s | **34.2 s** | 1.6x |
| GF(16) find 9 | not reachable | **36.7 s** | |
| GF(16) rule out 8 | 105 600 301 nodes, 2328 s | **108.7 s** | **21x** |
| GF(8) rule out 5 | | **4.1 s** | |
| Karatsuba, GF(4), W state | | under 0.02 s | |
| F₂ 5×5 rule out 12 | 146 402 553 nodes, 3610 s on 8 threads | **unresolved** | no answer in 700 s |

The last row is the one open comparison and is recorded as open. 700 s of one
core is 2.4% of the core-time the exhaustive search spent on it, so it says
nothing except that the question is bigger than the budget it was given.

**The advantage grows with the instance**, which is the interesting part: level
on `⟨2,2,2⟩`, 1.6 times on `⟨2,2,3⟩`, twenty-one times on GF(16). The exhaustive
search prunes subspaces and, with the orbit quotient, whole orbits at once; the
solver learns clauses, and the harder the instance the more there is to learn.

**This table replaces one that said the opposite.** The first measurements had
this method losing badly, and both reasons were defaults of mine rather than
properties of the method. They are worth stating because they are the whole
lesson:

Which backend, which solver and which flags produced these numbers, and
the reasoning that measurement overturned to get there:
[`choices.md`](choices.md).
