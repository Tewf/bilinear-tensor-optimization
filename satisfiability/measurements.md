# What it costs, measured

Every number here was produced on 2026-08-16 on one core of an i5-12450H, by
the commands in [the README](README.md), against the exhaustive searches in
[`../bilinear_rank/`](../bilinear_rank/) on the same fixtures. How the encodings
work is [`method.md`](method.md); this is only what they cost.

## Which GF(p) backend survives

**The one-hot CNF encoder, and this time on merit.** Ubuntu's `cvc5` 1.1.2 is
built without CoCoALib and cannot run its finite-field solver at all, but the
upstream 1.3.4 GPL build can, so the comparison the two backends were built for
actually happened. Ground truth from the exhaustive search: `GF(9)` and F₃
2×2-term both rank 3, F₃ 2×3-term rank 4.

| Question | one-hot CNF | cvc5 finite fields |
|---|---|---|
| `GF(9)` find 3 | **0.010 s** | 5.44 s |
| `GF(9)` rule out 2 | **0.008 s** | 0.085 s |
| F₃ 2×2 find 3 | **0.014 s** | 3.00 s |
| F₃ 2×2 rule out 2 | **0.011 s** | 0.022 s |
| F₃ 2×3 find 4 | **0.051 s** | no answer in 150 s |
| F₃ 2×3 rule out 3 | **0.099 s** | 2.22 s |

**Every verdict they both produced agrees**, and agrees with the exhaustive
search. That is what the second backend was for, and it did its job: the
hand-written multiplication table, addition chain and one-hot constraints are
corroborated by an encoding that shares none of them.

So `cvc5` stays, demoted to exactly that role. It is not dead code and it is not
a rival; it is the independent check on arithmetic that would otherwise be
mine alone, and it is reachable with `--backend smt`. Neither backend settles
F₃ 3×6 at its known rank of ten within five minutes.

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

## Two defaults that were wrong

**Symmetry breaking off.** Ruling out six products for `⟨2,2,2⟩` went from *no
answer in 120 s* to 1.57 s with the ordering on, at least seventy-six times and
the difference between an answer and none. It ships off because an over-strong
constraint would turn a satisfiable instance into UNSAT, which is a wrong lower
bound; it was checked first against all six fixtures of known rank, and every
one is still found. Use it for any question expected to answer no. It is not
implemented for the one-hot GF(p) encoding, and F₃ 3×6 not answering is
probably that.

**CryptoMiniSat preferred.** A GF(2) tensor equation is a parity constraint, so
a solver taking it as one line rather than four clauses ought to win. On these
instances native XOR is worth **nothing measurable**: 1.559 s against 1.563 s on
the same question. Kissat, which cannot read an XOR clause at all, is worth
**five times**: 0.31 s on that question, and 34.2 s against 167.9 s on the next
one up. The reasoning was sound and the measurement disagreed, so Kissat is now
tried first.
