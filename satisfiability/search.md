# Locating the rank between the bounds

Two bounds come free. The flattening rank is a lower bound in polynomial time,
and the smallest product of two of the three dimensions is an upper one. What is
left is to find the rank between them, by asking a solver a sequence of
decision questions, and there is more than one way to walk that range.

How the questions are encoded is [`method.md`](method.md); what the whole thing
costs against the exhaustive search is [`measurements.md`](measurements.md).

## Which way to walk the range

Four ways, measured on nine fixtures at two ceilings, three runs each with the
minimum taken, since another session held a core. Seconds. The names are the
MaxSAT literature's (`[morgado2013]`), because this is that problem: find an
optimum by a sequence of decision queries.

| fixture | ceiling | **ascending** | descending | bisection | gallop |
|---|---|---|---|---|---|
| f2_2x2 | naive | **0.007** | 0.016 | 0.007 | 0.016 |
| f2_2x2 | loose | **0.007** | 0.140 | 0.040 | 0.074 |
| f2_2x3 | naive | **0.036** | 0.056 | 0.037 | 0.056 |
| f2_2x3 | loose | **0.036** | 0.461 | 0.065 | 0.177 |
| gf8 | naive | 0.173 | 0.237 | **0.161** | 0.220 |
| gf8 | loose | **0.166** | 1.104 | 0.243 | 0.435 |
| matmul_2x2x2 | naive | **1.117** | 1.932 | 1.331 | 1.164 |
| matmul_2x2x2 | loose | **1.143** | 4.383 | 1.340 | 1.720 |
| f3_2x3 | naive | **0.084** | 0.236 | 0.155 | 0.234 |
| f3_2x3 | loose | **0.084** | 1.208 | 0.326 | 0.447 |

And once where the cost is real rather than milliseconds, GF(16) at rank 9:
**ascending 164.3 s**, gallop 175.1 s.

**Ascending wins, and the reason is not the one the literature would predict.**
The survey describes linear UNSAT-SAT as the arrangement where "all calls to a
SAT solver but the last will return unsatisfiable", which sounds like the worst
possible choice when refusals are the dear questions. It wins here because of
the free lower bound: the flattening rank is within three of the answer on every
fixture, so there are only a handful of refusals and they are the cheap ones far
from the rank. **A polynomial-time lower bound changes which search strategy
wins**, and without one ascending would be the worst of the four.

**It is also the only stable one.** Loosen the ceiling to three times the naive
bound and descending degrades up to nine times, gallop up to three, bisection up
to five, and ascending does not move at all, because it never reads the ceiling.
Stability here is insensitivity to how loose the inputs are, which is what
actually varies in use; run-to-run variance would be the wrong measure, since
kissat is deterministic.

The other three are deleted rather than kept behind a flag. This table is why.

**A correction, since the wrong answer is on the record.** An earlier commit
made gallop-and-bisect the default on a single instance measured while two other
solvers held two cores, and reported it as 2.3x faster than ascending. Re-run
properly it is slower on almost every cell. One instance on a contended machine
is not evidence, and I published it as though it were.

## Not implemented: choosing probes by their timing

Solve time peaks sharply just below the rank, so a secant-style search could in
principle locate the rank by extrapolating from timings. It is not implemented,
for a measured reason rather than a taste: on GF(16) the whole search costs
113.5 s and **the single unavoidable refusal at eight is 108.7 of them**.
Perfect probe placement is worth four percent, and it would be bought with a
signal that varies 13% run to run on this chassis from thermal throttling alone.

Its stable half is in, as `--probe`: a small budget for the questions asked on
the way and the full one for the question that cannot be avoided. An exhausted
probe is evidence without being an answer, and moves no bound.
