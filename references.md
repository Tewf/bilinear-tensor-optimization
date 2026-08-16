# References

Every algorithm in this repository comes from a published paper. This file is
where they are named; nothing else restates a citation, and code cites a key and
a numbered result, `[hastad1990, Lemma 2]`, so that the claim can be checked
against the source rather than against the code that implements it.

Papers are cited, never redistributed. [NOTICE](NOTICE) says what the licence
does and does not cover.

## What the problem is, and how hard

**`hastad1990`** — J. Håstad. *Tensor rank is NP-complete.* Journal of
Algorithms **11** (1990), no. 4, 644–654.
[doi:10.1016/0196-6774(90)90014-6](https://doi.org/10.1016/0196-6774(90)90014-6),
author's copy at <https://www.csc.kth.se/~johanh/tensorrank.pdf>.
Deciding tensor rank is NP-complete over every finite field and NP-hard over the
rationals. Lemma 2 is the reduction from 3SAT implemented in
[`satisfiability/formula_to_tensor.h`](satisfiability/formula_to_tensor.h): a
formula of `n` variables and `m` clauses becomes a tensor of rank `4n + 2m`
exactly when it is satisfiable.

## The exact search

**`bdez2012`** — R. Barbulescu, J. Detrey, N. Estibals, P. Zimmermann.
*Finding Optimal Formulae for Bilinear Maps.* WAIFI 2012, Bochum.
[doi:10.1007/978-3-642-31662-3_12](https://doi.org/10.1007/978-3-642-31662-3_12),
[hal-00640165v2](https://inria.hal.science/hal-00640165v2).
Algorithm 1 is the search over subspaces rather than subsets, which
[`bilinear_rank/exhaustive_search.h`](bilinear_rank/exhaustive_search.h)
implements. Its Tables 1–4 are the published ranks the fixtures are checked
against.

## Deciding rank with a solver

**`heule2021`** — M. J. H. Heule, M. Kauers, M. Seidl. *New ways to multiply
3 × 3-matrices.* Journal of Symbolic Computation **104** (2021), 899–916.
The SAT encoding of tensor decomposition over `Z/2Z`, and the method that
actually produced new schemes at that size.

**`heule2019`** — M. J. H. Heule, M. Kauers, M. Seidl. *Local search for fast
matrix multiplication.* SAT 2019, [arXiv:1903.11391](https://arxiv.org/abs/1903.11391).

**`ozdemir2023`** — A. Ozdemir, G. Kremer, C. Tinelli, C. Barrett.
*Satisfiability Modulo Finite Fields.* CAV 2023,
[eprint.iacr.org/2023/091](https://eprint.iacr.org/2023/091). The decision
procedure for prime fields implemented in cvc5, which is how `GF(p)` for `p > 2`
is decided here without hand-writing field arithmetic into clauses. The theory's
SMT-LIB surface is [arXiv:2407.21169](https://arxiv.org/abs/2407.21169).

## Sparsifying the operators

**`beniamini2020`** — G. Beniamini, N. Cheng, O. Holtz, E. Karstadt, O. Schwartz.
*Sparsifying the Operators of Fast Matrix Multiplication Algorithms.*
[arXiv:2008.03759](https://arxiv.org/abs/2008.03759), 2020.
Definition 3.2 is the Ω-valid set; Algorithms 3 and 4 are the two exact oracles
in [`matrix_sparsification/oracle_sparsifier.h`](matrix_sparsification/oracle_sparsifier.h);
Algorithm 2 is the driver they feed, from `gottlieb2010`.

**`beniamini2019`** — G. Beniamini, O. Schwartz. *Faster Matrix Multiplication
via Sparse Decomposition.* SPAA 2019, pp. 11–22. Described in
`original/Sparsifying_Matrices/` and not implemented here.

**`gottlieb2010`** — L.-A. Gottlieb, T. Neylon. *Matrix Sparsification and the
Sparse Null Space Problem.* APPROX/RANDOM 2010.

## The algorithms everything is measured against

**`strassen1969`** — V. Strassen. *Gaussian elimination is not optimal.*
Numerische Mathematik **13** (1969), no. 4, 354–356.
`fixtures/strassen_u.matrix` and `strassen_v.matrix`.

## Software

**`givaro`** — Exact arithmetic over GF(p) and over the rationals, from the CASYS
team at the Laboratoire Jean Kuntzmann.
[casys.gricad-pages.univ-grenoble-alpes.fr/givaro](https://casys.gricad-pages.univ-grenoble-alpes.fr/givaro/).
Used, not vendored. **GMP** underneath it. The only build dependency.

**`cryptominisat`** — M. Soos et al. Found on `PATH` at run time, never linked.
Chosen for native XOR clauses, which is exactly the shape of a GF(2) tensor
equation.

**`cvc5`** — The SMT solver implementing `ozdemir2023`'s finite-field theory.
Also found at run time. Its finite-field solver requires a CoCoALib build.

**`plinopt`** — J-G. Dumas, B. Grenet, C. Pernet, A. Sedoglavic. *PLinOpt: C++
routines for linear, bilinear & trilinear straight-line programs.*
[github.com/jgdumas/plinopt](https://github.com/jgdumas/plinopt), CeCILL-B. The
supervisor's own library for this problem area — `bin/sparsifier`,
`bin/factorizer`, `bin/orbiter` — and the reference implementation to check
against. Not a dependency: it needs LinBox, which this repository does not.
