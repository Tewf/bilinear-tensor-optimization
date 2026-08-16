# References

Every algorithm in this repository comes from a published paper. This file is
where they are named; nothing else restates a citation, and code cites a key and
a numbered result, `[hastad1990, Lemma 2]`, so that the claim can be checked
against the source rather than against the code that implements it.

Papers are cited, never redistributed. [NOTICE](NOTICE) says what the licence
does and does not cover.

This file is longer than the eighty lines the conventions ask of a markdown
document, deliberately. It is a bibliography, which is a data table: one entry
per paper, no prose to factor out, and splitting it across files would make a
citation harder to find rather than easier, which is the opposite of what the
length rule is for.

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

**`schaefer2018`**: M. Schaefer, D. Štefankovič. *The Complexity of Tensor
Rank.* Theory of Computing Systems **62** (2018), 1161-1174.
[preprint](https://www.cs.rochester.edu/~stefanko/Publications-new/J36.pdf).
Tensor rank over a field `F` is polynomial-time equivalent to the existential
theory of `F`, which gives NP-complete over finite fields, `∃ℝ`-complete over
the reals and `∃ℚ`-complete over the rationals, where decidability is open. The
reason [`satisfiability/complexity.md`](satisfiability/complexity.md) exists.

**`hillar2013`**: C. J. Hillar, L.-H. Lim. *Most Tensor Problems are NP-hard.*
Journal of the ACM **60** (2013), no. 6, article 45,
[arXiv:0911.1393](https://arxiv.org/abs/0911.1393). Extends Håstad's hardness to
`ℝ` and `ℂ`, and the source of the shorthand that flattens the per-field
picture.

## The exact search

**`bdez2012`**: R. Barbulescu, J. Detrey, N. Estibals, P. Zimmermann.
*Finding Optimal Formulae for Bilinear Maps.* WAIFI 2012, Bochum.
[doi:10.1007/978-3-642-31662-3_12](https://doi.org/10.1007/978-3-642-31662-3_12),
[hal-00640165v2](https://inria.hal.science/hal-00640165v2).
Algorithm 1 is the search over subspaces rather than subsets, which
[`exhaustive_search/exhaustive_search.h`](exhaustive_search/exhaustive_search.h)
implements. Its Tables 1-4 are the published ranks the fixtures are checked
against.

**`covanov2019`**: S. Covanov. *Improved Method for Finding Optimal Formulae
for Bilinear Maps in a Finite Field.*
[arXiv:1705.07728v3](https://arxiv.org/abs/1705.07728), 2018.
Definition 7 and Definition 13 are the automorphism action and the setwise
stabiliser; Algorithm 3 is `BDEZStab`; Definitions 20 and 22 and Algorithm 4 are
the covering-sets method; Propositions 28 and 29 are the stems for the short
product and the matrix product.

**`covanov2018`**: S. Covanov. *Algorithmes de Multiplication: Complexité
Bilinéaire et Méthodes Asymptotiquement Rapides.* Thèse, Université de Lorraine,
2018. NNT 2018LORR0057, [tel-01825744](https://theses.hal.science/tel-01825744v1).
The long form of `covanov2019`.

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

## Searching for decompositions, which is the other direction

**`alphatensor2022`**: A. Fawzi et al. *Discovering faster matrix multiplication
algorithms with reinforcement learning.* Nature **610** (2022), 47-53.
AlphaZero applied to decomposition as a single-player game; 14 236 non-equivalent
schemes for `⟨4,4,4⟩`.

**`kauers2023`**: M. Kauers, J. Moosbauer. *Flip Graphs for Matrix
Multiplication.* ISSAC 2023, [arXiv:2212.01175](https://arxiv.org/abs/2212.01175).
Rewriting a working decomposition rather than searching for one. `⟨5,5,5⟩` in 95
with no machine learning.

**`moosbauer2025`**: J. Moosbauer, M. Poole. *Flip Graphs with Symmetry and New
Matrix Multiplication Schemes.* ISSAC 2025,
[arXiv:2502.04514](https://arxiv.org/abs/2502.04514). `⟨5,5,5⟩` in 93 and
`⟨6,6,6⟩` in 153, by taking the tensor's symmetries into the walk.

**`chen2025`**: S. Chen, M. Kauers. *Flip Graphs for Polynomial Multiplication.*
[arXiv:2502.06264](https://arxiv.org/abs/2502.06264), 2025. The flip graph
applied to **polynomial multiplication**, which is this repository's own
subject: every `.tensor` fixture here is a polynomial product. Their walk finds the schemes and **a SAT solver proves them optimal** (their
Theorem 7), which is exactly the division of labour between the two strands
here. **Their `(n,m)` are degrees, not term counts**: Toom-Cook gives rank
`n+m+1`, so their `(n,m)` is this repository's `(n+1)x(m+1)`. Their proven
list, translated, is 2x2, 2x3, 2x4, 2x5, 2x6, 3x3, 3x4, 3x5 and 4x4 over `Z2`.

**`kauers2025`**: M. Kauers, I. Wood. *Exploring the Meta Flip Graph for Matrix
Multiplication.* [arXiv:2510.19787](https://arxiv.org/abs/2510.19787), 2025.

**`ikenmeyer2025`**: C. Ikenmeyer, J. Moosbauer. *Strassen's Algorithm via Orbit
Flip Graphs.* [arXiv:2503.05467](https://arxiv.org/abs/2503.05467), 2025.
Strassen's 7 reproved from an order-6 group action, with no calculation and no
pattern matching.

**`arai2024`**: Y. Arai, Y. Ichikawa, K. Hukushima. *Adaptive Flip Graph
Algorithm for Matrix Multiplication.* Proc. ISSAC'24, 292-298,
[arXiv:2312.16960](https://arxiv.org/abs/2312.16960).
Transitions that do not strictly reduce the count, and a constrained search range.

**`perminov2026`**: A. I. Perminov. *Fast Matrix Multiplication in Small Formats:
Discovering New Schemes with an Open-Source Flip Graph Framework.*
[arXiv:2603.02398](https://arxiv.org/abs/2603.02398), code at
[github.com/dronperminov/FastMatrixMultiplication](https://github.com/dronperminov/FastMatrixMultiplication),
MIT. Bit-level encoding, OpenMP, 680 formats from `(2,2,2)` to `(16,16,16)`, and
a GPU variant. **The baseline for any flip graph number produced here.**

**`sedoglavic2024`**: A. Sedoglavic. *Yet Another Catalogue of Fast Matrix
Multiplication Algorithms.* [fmm.univ-lille.fr](https://fmm.univ-lille.fr/).
The field's running record of best known upper bounds.

**`deza2023`**: A. Deza, C. Liu, E. B. Khalil, P. Vaezipoor. *Fast Matrix
Multiplication Without Tears: A Constraint Programming Approach.* Proc. CP 2023,
LIPIcs vol. 280, [arXiv:2306.01097](https://arxiv.org/abs/2306.01097).
The Brent equations solved by constraint programming; `integer_programme_encoding.h`,
which is not on `main` yet, states the same equations for a MILP solver, so that
a third instrument answers
the question the SAT strand and the tree search answer. The 2x2 and 3x3 cases are
MIPLIB 2017 benchmarks, so the formulation is standard and nothing here is new.

**`alphaevolve2025`**: Google DeepMind. *AlphaEvolve: A Coding Agent for
Scientific and Algorithmic Discovery.* 2025. `⟨4,4,4⟩` in 48 multiplications over
`ℂ`, the first improvement on 49 in fifty-six years.

**`dumas2026`**: *Complex to Rational Fast Matrix Multiplication.*
[arXiv:2602.13171](https://arxiv.org/abs/2602.13171), 2026. Converts a complex
scheme to a rational one or proves none exists, generalising Dumas, Pernet and
Sedoglavic (2025).

**`yang2025`**: J. Yang. *Faster search for tensor decomposition over finite
fields.* [arXiv:2502.12390](https://arxiv.org/abs/2502.12390), 2025. Exact
decision in `O*(|F|^(min{R, Σn_d} + (R−n₀)(Σ_{d≠0} n_d)))` and polynomial space.
The nearest thing to this repository's own problem, and not implemented here.

**`heule2024`**: *Ruling Out Low-rank Matrix Multiplication Tensor
Decompositions with Symmetries via SAT.*
[arXiv:2402.01011](https://arxiv.org/abs/2402.01011), 2024.

**`alman2025`**: J. Alman, R. Duan, V. Vassilevska Williams, Y. Xu, Z. Xu,
R. Zhou. *More Asymmetry Yields Faster Matrix Multiplication.* SODA 2025,
[arXiv:2404.16349](https://arxiv.org/abs/2404.16349). `ω < 2.371339`. The laser
method, which shares no machinery with anything here.

**`morgado2013`**: A. Morgado, F. Heras, M. Liffiton, J. Planes, J. Marques-Silva.
*Iterative and core-guided MaxSAT solving: a survey and assessment.*
Constraints **18** (2013), 478-534. Names the search this module does: finding an
optimum by a sequence of decision queries, as linear UNSAT-SAT, linear SAT-UNSAT
and binary search. Its finding that binary search "is optimal in terms of the
number of calls to a SAT oracle" yet "has seldom been used in practical MaxSAT
solvers" is reproduced in [`satisfiability/search.md`](satisfiability/search.md).

## Sparsifying the operators

**`beniamini2020`**: G. Beniamini, N. Cheng, O. Holtz, E. Karstadt, O. Schwartz.
*Sparsifying the Operators of Fast Matrix Multiplication Algorithms.*
[arXiv:2008.03759](https://arxiv.org/abs/2008.03759), 2020.
Definition 3.2 is the Ω-valid set; Algorithms 3 and 4 are the two exact oracles
in [`matrix_sparsification/oracle_sparsifier.h`](matrix_sparsification/oracle_sparsifier.h);
Algorithm 2 is the driver they feed, from `gottlieb2010`; Algorithm 6 is the
greedy in `greedy_sparsifier.h`; Claim 2.11 is the additive complexity in
`algorithm_cost.h`.

**`beniamini2019`**: G. Beniamini, O. Schwartz. *Faster Matrix Multiplication
Via Sparse Decomposition.* SPAA 2019, pp. 11-22.
Definition 2.8 is the trilinear identity `algorithm_check.h` verifies; Claim 3.9
and Corollary 3.10 are the arithmetic complexity and its leading coefficient;
Definition 3.5 and Algorithm 2 are the decomposed recursive-bilinear algorithm.

**`karstadt2017`**: E. Karstadt, O. Schwartz. *Matrix Multiplication, A Little
Faster.* SPAA 2017. The alternative-basis technique both papers above build on,
and the source of Strassen's leading coefficient dropping from 7 to 5.

**`gottlieb2010`**: L.-A. Gottlieb, T. Neylon. *Matrix Sparsification and the
Sparse Null Space Problem.* APPROX/RANDOM 2010. The greedy driver that the
sparsest-independent-vector oracles are oracles for.

**`dumas2024cex`**: J-G. Dumas. *Cex_Poldet*, Maple worksheet, 27 May 2024,
unpublished; supplied with the internship material. The determinant-polynomial
feasibility test in `matrix_sparsification/pattern_feasibility.h`, and the
counterexample fixture `fixtures/dumas_counterexample_l.matrix`.

## Finite field extensions and curves

**`rambaud2014`**: M. Rambaud. *Finding Optimal Chudnovsky-Chudnovsky
Multiplication Algorithms.* WAIFI 2014.
Its four-step roadmap in §1; Theorem 2 is the bound an interpolation system
gives; Algorithm 3 is `bdez2012`'s search restricted to symmetric forms; Tables
1 and 2 are the published bounds on `µ_sym`.

**`ballet2021`**: S. Ballet et al. *On the Tensor Rank of Multiplication in
Finite Extensions of Finite Fields and Related Issues in Algebraic Geometry.*
Russian Mathematical Surveys **76** (2021), no. 1, 29-89. The survey the bound
tables are taken from.

**`rousseau2021`**: É. Rousseau. *Arithmétique Efficace des Extensions de Corps
Finis.* Thèse, Institut Polytechnique de Paris, 2021. NNT 2021IPPAT013,
[tel-03299466](https://theses.hal.science/tel-03299466). Context, not implemented
here.

**`akleylek2014`**: S. Akleylek, F. Özbudak, C. Özel. *On the Arithmetic
Operations Over Finite Fields of Characteristic Three with Low Complexity.*
Journal of Computational and Applied Mathematics **259** (2014), 546-554.
Context, not implemented here.

## The algorithms everything is measured against

**`strassen1969`**: V. Strassen. *Gaussian elimination is not optimal.*
Numerische Mathematik **13** (1969), no. 4, 354-356.
`fixtures/strassen_u.matrix` and `strassen_v.matrix`.

## Software

**`givaro`**: Exact Arithmetic Over GF(p) and Over the Rationals, from the CASYS
team at the Laboratoire Jean Kuntzmann.
[casys.gricad-pages.univ-grenoble-alpes.fr/givaro](https://casys.gricad-pages.univ-grenoble-alpes.fr/givaro/).
Used, not vendored. **GMP** underneath it. The only build dependency.

**`cryptominisat`**: M. Soos et al. Found on `PATH` at run time, never linked.
Chosen for native XOR clauses, which is exactly the shape of a GF(2) tensor
equation.

**`cvc5`**: The SMT solver implementing `ozdemir2023`'s finite-field theory.
Also found at run time. Its finite-field solver requires a CoCoALib build.

**`plinopt`**: J-G. Dumas, B. Grenet, C. Pernet, A. Sedoglavic. *PLinOpt: C++
Routines for Linear, Bilinear & Trilinear Straight-line Programs.*
[github.com/jgdumas/plinopt](https://github.com/jgdumas/plinopt), CeCILL-B.
The supervisor's own library for this problem area, and the reference
implementation to check against: `bin/sparsifier`, `bin/factorizer`,
`bin/orbiter`. It reaches sparsity by a different route from `beniamini2020`;
sparse QLUP elimination and bounded coefficient search rather than the Ω-valid
oracles; so the two are worth comparing rather than one replacing the other.
Not a dependency here: it needs LinBox, which this repository does not.

**`cbc`**, **`glpsol`**, **`lp_solve`**, **`gurobi_cl`**: The integer programming
backends of `optimisation/`, which lands with the branch that owns it, ranked and
found on `PATH`
at run time, never linked. CBC (COIN-OR, EPL), GLPK (GNU, GPL) and lp_solve
(LGPL) are in the Ubuntu archive and are the three verified on this machine;
Gurobi is proprietary, free to academics, and its recipe here is unverified for
want of a licence. None is a dependency: absent all four, the built-in exact
simplex and branch and bound answers.
