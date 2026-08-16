# Tensor rank and satisfiability are the same problem

Håstad proved it in 1990: deciding the rank of a three-dimensional tensor is
**NP-complete over every finite field**, and NP-hard over the rationals
(`[hastad1990]`; keys are [`../references.md`](../references.md)). That has two
directions, and this folder runs both.

| | |
|---|---|
| **Rank is at least as hard as SAT** | [`formula_to_tensor.h`](formula_to_tensor.h) turns a 3SAT formula into a tensor of rank `4n + 2m` exactly when it is satisfiable |
| **Rank is no harder than SAT** | the encoders turn `rank(T) ≤ r` into a formula, and a solver answers it |

The second is why this exists, and the measurements say something more
interesting than "a solver is faster". It is not faster. It is differently
shaped: **finding a decomposition is what a solver is good at, and proving there
is none is where it loses.** Strassen's seven products come out of nothing in
under half a second; ruling out eight products for `⟨2,2,3⟩` takes 168 seconds
against the exhaustive search's 53. Full table in [`method.md`](method.md).

So this is a second opinion and a different instrument, not a replacement for
[the exhaustive search](../bilinear_rank/exhaustive_search.h). Where both
finish they agree, on every fixture, in both directions.

```sh
decide-rank-by-sat fixtures/matmul_2x2x2.tensor --target 7    # Strassen
decide-rank-by-sat fixtures/f2_5x5.tensor --from 9 --to 14    # sweep for the rank
decide-rank-by-sat fixtures/f3_3x6.tensor --target 10 --emit-cnf out.cnf
```

Needs `cryptominisat` on `PATH`, or `--emit-cnf` and your own solver. **Nothing
links against a solver**: the build depends on Givaro and nothing else, and a
machine without one still builds and passes its tests.

## The three encoders

| | Field | What the unknowns are made of |
|---|---|---|
| [`binary_encoding.h`](binary_encoding.h) | GF(2) | Booleans. A literal *is* a field element, so a tensor equation is one parity constraint |
| [`prime_field_encoding.h`](prime_field_encoding.h) | GF(p) | `p` Booleans, exactly one true, with the field's tables written out as clauses |
| [`field_theory_encoding.h`](field_theory_encoding.h) | GF(p) | nothing. cvc5 has a theory of prime fields, so the equations go across as equations |

GF(2) is settled: parity constraints are what CryptoMiniSat takes natively, and
the encoding has nothing in it to get wrong.

**GF(p) has two backends, and the contest between them never happened.**
Ubuntu's `cvc5` 1.1.2 is built without CoCoALib, so its finite-field solver
refuses to start; the one-hot encoder carries `GF(p)` because it was the only
entrant that could run, not because it won. The SMT writer stays, since the file
it produces is correct and a cvc5 with CoCoALib would decide it.
[`method.md`](method.md) records that, so nobody later reads one-hot as having
been measured the better.

## What is checked, and how, without a solver installed

The encodings are validated by turning the question around. A decomposition we
**already know** is the assignment the formula must accept, so Karatsuba's
three products are written down, the corresponding variables are set, the
definitional clauses are propagated, and nothing may be violated. Then one entry
of the tensor is changed and the same assignment must break something, which is
what stops a permissive encoding from passing.

Håstad's Lemma 2 is checked the same way and just as cheaply: from a satisfying
assignment the proof builds `4n + 2m` matrices, and all three of its claims are
asserted: at most `4n + 2m` of them, every one of rank at most one, and
together they span the tensor.

The one-hot encoder also accepts GF(2), where it must agree with the Boolean
one. Two encodings accepting the same assignment is the cheapest evidence
available that the tables are right.

## Where this stops

A solver's "no" is a lower bound **only if it finished**. Timeouts and memory
kills are reported as a third answer and never folded into "no", because that would
turn giving up into a proof. A solver's "yes" is checked: the model is turned
back into rank-one matrices and recombined, and the command fails if that does
not reproduce the tensor.

Symmetry breaking ships off by default, because an over-strong constraint would
turn a satisfiable instance into UNSAT and a wrong "no" is a wrong lower bound.
**Turn it on for any question expected to answer no.** Ruling out six products
for `⟨2,2,2⟩` goes from no answer in 120 seconds to 1.57 seconds with it, and it
was checked first against all six fixtures whose rank is known: every one is
still found. That is evidence rather than a proof, which is why the flag stays
explicit.

Nothing here decides rank over the rationals. Håstad's theorem is NP-hardness
there, not NP-completeness, because the certificate may need too many bits.
