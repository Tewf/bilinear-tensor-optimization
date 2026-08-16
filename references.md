# References

Every algorithm in this repository comes from a published paper. This file is
where they are named; nothing else restates a citation, and code cites a key and
a numbered result, `[hastad1990, Lemma 2]`, so that the claim can be checked
against the source rather than against the code that implements it.

Papers are cited, never redistributed. [NOTICE](NOTICE) says what the licence
does and does not cover.

## What the problem is, and how hard

**`hastad1990`**: J. Håstad. *Tensor rank is NP-complete.* Journal of
Algorithms **11** (1990), no. 4, 644-654.
[doi:10.1016/0196-6774(90)90014-6](https://doi.org/10.1016/0196-6774(90)90014-6),
author's copy at <https://www.csc.kth.se/~johanh/tensorrank.pdf>.
Deciding tensor rank is NP-complete over every finite field and NP-hard over the
rationals. Lemma 2 is the reduction from 3SAT implemented in
[`satisfiability/formula_to_tensor.h`](satisfiability/formula_to_tensor.h): a
formula of `n` variables and `m` clauses becomes a tensor of rank `4n + 2m`
exactly when it is satisfiable.

## The exact search

**`bdez2012`**: R. Barbulescu, J. Detrey, N. Estibals, P. Zimmermann.
*Finding Optimal Formulae for Bilinear Maps.* WAIFI 2012, Bochum.
[doi:10.1007/978-3-642-31662-3_12](https://doi.org/10.1007/978-3-642-31662-3_12),
[hal-00640165v2](https://inria.hal.science/hal-00640165v2).
Algorithm 1 is the search over subspaces rather than subsets, which
[`bilinear_rank/exhaustive_search.h`](bilinear_rank/exhaustive_search.h)
implements. Its Tables 1-4 are the published ranks the fixtures are checked
against.

## Deciding rank with a solver

**`heule2021`**: M. J. H. Heule, M. Kauers, M. Seidl. *New ways to multiply
3 × 3-matrices.* Journal of Symbolic Computation **104** (2021), 899-916.
The SAT encoding of tensor decomposition over `Z/2Z`, and the method that
actually produced new schemes at that size.

**`heule2019`**: M. J. H. Heule, M. Kauers, M. Seidl. *Local search for fast
matrix multiplication.* SAT 2019, [arXiv:1903.11391](https://arxiv.org/abs/1903.11391).

**`ozdemir2023`**: A. Ozdemir, G. Kremer, C. Tinelli, C. Barrett.
*Satisfiability Modulo Finite Fields.* CAV 2023,
[eprint.iacr.org/2023/091](https://eprint.iacr.org/2023/091). The decision
procedure for prime fields implemented in cvc5, which is how `GF(p)` for `p > 2`
is decided here without hand-writing field arithmetic into clauses. The theory's
SMT-LIB surface is [arXiv:2407.21169](https://arxiv.org/abs/2407.21169).

## Sparsifying the operators

**`beniamini2020`**: G. Beniamini, N. Cheng, O. Holtz, E. Karstadt, O. Schwartz.
*Sparsifying the Operators of Fast Matrix Multiplication Algorithms.*
[arXiv:2008.03759](https://arxiv.org/abs/2008.03759), 2020.
Definition 3.2 is the Ω-valid set; Algorithms 3 and 4 are the two exact oracles
in [`matrix_sparsification/oracle_sparsifier.h`](matrix_sparsification/oracle_sparsifier.h);
Algorithm 2 is the driver they feed, from `gottlieb2010`.

**`beniamini2019`**: G. Beniamini, O. Schwartz. *Faster Matrix Multiplication
via Sparse Decomposition.* SPAA 2019, pp. 11-22. Described in
`original/Sparsifying_Matrices/` and not implemented here.

**`gottlieb2010`**: L.-A. Gottlieb, T. Neylon. *Matrix Sparsification and the
Sparse Null Space Problem.* APPROX/RANDOM 2010.

## Walking a decomposition

Where this repository stands against all of these:
[`positioning.md`](positioning.md).

**`kauers2023`**: M. Kauers, J. Moosbauer. *Flip graphs for matrix
multiplication.* Proc. ISSAC'23, 381-388. arXiv:2212.01175. The method
[`flip_graph.h`](bilinear_rank/flip_graph.h) implements: random walks on a graph
whose vertices are decompositions, where a flip preserves the rank and a
reduction lowers it.

**`chen2025`**: S. Chen, M. Kauers. *Flip graphs for polynomial multiplication.*
arXiv:2502.06264. The same walk on this repository's own subject, over `Z2`, with
optimality proved by SAT for every degree pair up to `(3,3)`. Their closing
question, polynomial multiplication over `Z3`, `Z5` and `Z7`, and the obstacle
they name for it, are the one opening this repository has.

**`moosbauer2025`**: J. Moosbauer, M. Poole. *Flip graphs with symmetry and new
matrix multiplication schemes.* arXiv:2502.04514. The walk restricted to schemes
admitting a group action: `5x5` in 93 multiplications, `6x6` in 153.

**`ikenmeyer2025`**: C. Ikenmeyer, J. Moosbauer. *Strassen's algorithm via orbit
flip graphs.* arXiv:2503.05467. Strassen's 7 reproved from an order-6 group
action, with no calculation and no pattern matching.

**`arai2024`**: Y. Arai, Y. Ichikawa, K. Hukushima. *Adaptive flip graph
algorithm for matrix multiplication.* Proc. ISSAC'24, 292-298. arXiv:2312.16960.
Transitions that do not strictly reduce the count, and a constrained search range.

**`kauers2025meta`**: M. Kauers, I. Wood. *Exploring the meta flip graph for
matrix multiplication.* arXiv:2510.19787.

**`perminov2026`**: A. I. Perminov. *Fast matrix multiplication in small formats:
discovering new schemes with an open-source flip graph framework.*
arXiv:2603.02398, code at
[github.com/dronperminov/FastMatrixMultiplication](https://github.com/dronperminov/FastMatrixMultiplication),
MIT. Bit-level encoding, OpenMP, 680 formats from `(2,2,2)` to `(16,16,16)`, and
a GPU variant. **The baseline for any flip graph number produced here.**

**`sedoglavic2024`**: A. Sedoglavic. *Yet another catalogue of fast matrix
multiplication algorithms.* [fmm.univ-lille.fr](https://fmm.univ-lille.fr/). The
field's running record of best known upper bounds.

**`deza2023`**: A. Deza, C. Liu, E. B. Khalil, P. Vaezipoor. *Fast matrix
multiplication without tears: a constraint programming approach.* Proc. CP 2023,
LIPIcs vol. 280. arXiv:2306.01097. The Brent equations solved by constraint
programming; [`integer_programme_encoding.h`](bilinear_rank/integer_programme_encoding.h)
states the same equations for a MILP solver, so that a third instrument answers
the question the SAT strand and the tree search answer. The 2x2 and 3x3 cases are
MIPLIB 2017 benchmarks, so the formulation is standard and nothing here is new.

## The algorithms everything is measured against

**`strassen1969`**: V. Strassen. *Gaussian elimination is not optimal.*
Numerische Mathematik **13** (1969), no. 4, 354-356.
`fixtures/strassen_u.matrix` and `strassen_v.matrix`.

## Software

**`givaro`**: exact arithmetic over GF(p) and over the rationals, from the CASYS
team at the Laboratoire Jean Kuntzmann.
[casys.gricad-pages.univ-grenoble-alpes.fr/givaro](https://casys.gricad-pages.univ-grenoble-alpes.fr/givaro/).
Used, not vendored. **GMP** underneath it. The only build dependency.

**`cryptominisat`**: M. Soos et al. Found on `PATH` at run time, never linked.
Chosen for native XOR clauses, which is exactly the shape of a GF(2) tensor
equation.

**`cvc5`**: the SMT solver implementing `ozdemir2023`'s finite-field theory.
Also found at run time. Its finite-field solver requires a CoCoALib build.

**`plinopt`**: J-G. Dumas, B. Grenet, C. Pernet, A. Sedoglavic. *PLinOpt: C++
routines for linear, bilinear & trilinear straight-line programs.*
[github.com/jgdumas/plinopt](https://github.com/jgdumas/plinopt), CeCILL-B. The
supervisor's own library for this problem area (`bin/sparsifier`,
`bin/factorizer`, `bin/orbiter`), and the reference implementation to check
against. Not a dependency: it needs LinBox, which this repository does not.

**`cbc`**, **`glpsol`**, **`lp_solve`**, **`gurobi_cl`**: the integer programming
backends of [`optimisation/`](optimisation/README.md), ranked and found on `PATH`
at run time, never linked. CBC (COIN-OR, EPL), GLPK (GNU, GPL) and lp_solve
(LGPL) are in the Ubuntu archive and are the three verified on this machine;
Gurobi is proprietary, free to academics, and its recipe here is unverified for
want of a licence. None is a dependency: absent all four, the built-in exact
simplex and branch and bound answers.
