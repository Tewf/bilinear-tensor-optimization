# Sparsifying the operators

A fast multiplication algorithm has two costs. The multiplications, which
[the other strand](../bilinear_rank/README.md) counts, and the additions, which are set
by how many nonzero entries its operators carry. Given the operator `U`, the
problem is to find an invertible `V` minimising `nnz(U V)`.

```sh
./build/matrix_sparsification/sparsify-operator fixtures/strassen_u.matrix --show
```

## Results

This strand reported no measured result: the internship write-up says only that
the program took too long for a simple problem. It has one now. Full numbers in
[`results.json`](results.json).

| Operator | As given | Row basis | Oracle, bottom-up | Oracle, top-down |
|---|---|---|---|---|
| Strassen `U` (7×4) | 12 | **10** | **10** | **10** |
| Strassen `V` (7×4) | 12 | **10** | **10** | **10** |
| Alternative basis (7×4) | 21 | **10** | **10** | **10** |

All three methods reach 10, in milliseconds. Ten is what the construction
predicts rather than a surprise: inverting a square block of rows makes four
rows singletons, and the remaining three come out with two nonzeros each. It is
not proved minimal.

The third operator is `example0` from the original — a Strassen-like algorithm
already written in an alternative basis, so its entries are ninths. It is the
case where floating point has something to go wrong with, because no double
holds 4/9.

## The methods

**Row-basis heuristic.** Take a square block of rows of `U` that is invertible
and use its inverse. It guarantees that as many rows of the result as `U` has
columns are singletons, and costs one pass per subset of rows.

**Exact oracle, bottom-up.** Over every column subset one smaller than the row
count, find a vector in the row space that is zero on all of them, and keep
whichever has the most zeros overall. Replace a row with it, and repeat.

**Exact oracle, top-down.** The same, but walking subsets from the largest
downwards and taking the first one found — a vector forced to zero on more
columns cannot be beaten by one forced on fewer, so there is nothing to gain by
looking further.

All three written out precisely, with their time and space cost:
**[`method.md`](method.md)**.

## What was corrected

**Algorithm 2.4 was unreachable.** `sparsifying_…py:268-272` offers the choice
between the two oracles and both arms call `algorithm2_3`. The top-down method
could never be run from the command line.

**Its search falls off the end**, returning `None` into `v, i = algorithm4(u)`.
It does not fire on these operators, so it was latent rather than observed.

**The search objective was computed on doubles.** `algorithm3` counts zeros with
`== 0` on raw floats; running the original on the alternative-basis operator,
that sees 86 zeros where 144 exist. Worth being exact about where this bites:
the count it *reports* was fine, because `nnz` sends every entry through
`Fraction(...).limit_denominator()`, which snaps `1.85e-18` to zero. The count
was sound and the quantity being maximised was not.

Everything here is exact rationals, so none of that is possible: a zero is a
zero because it is one.

## Where this stops

Both oracles are exact for the sparsest-independent-vector subproblem, but they
assemble the answer greedily, one row at a time, and that assembly is not proved
optimal. The compressed-sensing approach of the second article
(`original/Sparsifying_Matrices/`) is not implemented here at all.
