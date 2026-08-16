# Tensor rank and satisfiability are the same problem

Håstad proved it in 1990: deciding the rank of a three-dimensional tensor is
**NP-complete over every finite field**, and NP-hard over the rationals
(`[hastad1990]`; keys are [`../references.md`](../references.md)). That has two
directions, and this folder runs both.

| | |
|---|---|
| **Rank is at least as hard as SAT** | [`formula_to_tensor.h`](formula_to_tensor.h) turns a 3SAT formula into a tensor of rank `4n + 2m` exactly when it is satisfiable |
| **Rank is no harder than SAT** | the encoders turn `rank(T) ≤ r` into a formula, and a solver answers it |

The second is why this exists. [The exhaustive
search](../bilinear_rank/exhaustive_search.h) is complete and pays for it. Its
own `method.md` costs F₂ 5×5 at twelve products at seven hours. A solver
answers questions of that shape for a living.

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

**GF(p) has two backends on purpose, and is meant to end with one.** The one-hot
encoding has three hand-written pieces (multiplication table, addition chain,
one-hot constraints), and a mistake in any of them gives a confident wrong rank
rather than an error. The SMT one has none of them but needs a second solver,
whose Ubuntu build may lack the CoCoALib support its finite-field solver
requires. They are run against each other on `f3_3x6`, whose rank 10 is already
known, and [`method.md`](method.md) records which one survives and why.

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

Symmetry breaking is implemented and off by default. Permuting the terms of a
decomposition gives another decomposition, so ordering them is sound. But an
over-strong constraint turns a satisfiable instance into UNSAT, and a wrong "no"
is a wrong lower bound.

Nothing here decides rank over the rationals. Håstad's theorem is NP-hardness
there, not NP-completeness, because the certificate may need too many bits.
