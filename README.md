# Decomposition of Bilinear Tensors

[![CI](https://github.com/Tewf/bilinear-tensor-optimization/actions/workflows/ci.yml/badge.svg)](https://github.com/Tewf/bilinear-tensor-optimization/actions/workflows/ci.yml)
[![Live pages](https://img.shields.io/badge/pages-tewf.github.io%2Fbilinear--tensor--optimization-1f6feb)](https://tewf.github.io/bilinear-tensor-optimization/)
[![Licence](https://img.shields.io/badge/licence-MIT-lightgrey)](LICENSE)

> [Lire en français](README.fr.md)

Research internship at the **LJK** (Laboratoire Jean Kuntzmann, Université
Grenoble Alpes), supervised by **Jean-Guillaume Dumas**, 21 May to 15 July 2024,
and what happened when it was picked up again and finished.

The rank of a bilinear map is the number of multiplications needed to compute
it. Strassen's seven-instead-of-eight for 2×2 matrices is where fast matrix
multiplication comes from, and finding such decompositions in general is open.
This is a search for them, in four directions: fewer multiplications, fewer
additions, the rank asked as a question a SAT solver can answer, and, where no
search can reach, a bound read off an algebraic curve.

[`original/`](original/) holds the internship exactly as delivered and does not
move. Everything else is a corrected, tested reimplementation in C++, measured
against it. Every number below is produced by code in this repository, and
[CI](https://github.com/Tewf/bilinear-tensor-optimization/actions/workflows/ci.yml)
reruns the whole table on every push.

## What came out of finishing it

**[Rank of bilinear maps](internship_heuristic/)**: F3 3×6 polynomial multiplication now takes
**10 multiplications instead of the published 11**, because the internship's
final step on that map never terminated and the 11 was a figure from an
abandoned run. The search finishes in **7.49 seconds**. The three maps that did
complete are reproduced exactly, at **10×, 1033× and 520×** the speed.

| Map | Naive | Result | Internship |
|---|---|---|---|
| F2 5×5 | 25 | **14** · 1.41 s | 14 · 14.42 s |
| F2 3×8 | 24 | **15** · 3.35 s | 15 · 3460.54 s |
| F2 4×7 | 28 | **16** · 9.69 s | 16 · 5044.06 s |
| F3 3×6 | 18 | **10** · 7.49 s | 11, *did not finish* |

**[Sparsifying the operators](matrix_sparsification/)**: the strand that reported no
measured result has one. Strassen's encoding operators go from **12 nonzeros to
10**, and the alternative-basis operator the original was tested against goes
from **21 to 10**, in milliseconds. Fewer nonzeros means fewer additions, which
is the cost the multiplication count does not capture.

**The exact search settles small maps outright** and, for the first time here,
bounds a large one from *below*. F2 5×5 has no 9-, 10- or 11-product algorithm,
each ruled out exhaustively, so its rank is at least 12, and the heuristic
exhibits 14: what this repository proves is **12 ≤ rank ≤ 14**. Ruling out 12 has
not been run, and is priced at about seven hours by extrapolation. Barbulescu,
Detrey, Estibals and Zimmermann found 27 decompositions at 13 in 2012, so the
rank is 13 on their authority, and the heuristic's 14 is therefore not optimal. On
maps it can finish it reproduces Karatsuba's 3, the write-up's own 5, and the classical
3 and 6 for GF(4) and GF(8) multiplication.

**And the two strands are one pipeline again.** The rank search recovers the
encoding operators ⟨L, R, P⟩ from its decomposition and writes them out; the
sparsification is what they are for:

```sh
minimise-rank fixtures/f2_5x5.tensor --emit-operators out   # 25 -> 14 multiplications
sparsify-operator out_L.sms                          # 31 -> 27 nonzeros
```

**[Asking the oracle for a fixed `k`](oracle_guided_search/) is a negative result,
and one positive one.** `find-at-rank` commits to one orbit representative at a time
and accepts on satisfiable without ever waiting for a refutation. It is **dominated by
the two heuristics on every fixture measured** and the asymmetry it was built on is
not there: matched, a pinned refutation and a pinned acceptance cost about the same
([`measurements.md`](oracle_guided_search/measurements.md)). What did come out of it
is `deflate-strictly --refuter tree`, which buys the pinned refutation from
`[covanov2019]`'s tree instead of a solver and is about **50x** cheaper than either
the pinned or the whole-instance solver refutation on `⟨2,2,2⟩`, because pinning
deletes a level of the tree and does nothing to a CNF
([`refutation-prices.md`](oracle_guided_search/refutation-prices.md)).

**And the enumerators here now deduplicate up to the group**, which nothing in this
repository did. `[bdez2012]` §4.5 and `[covanov2019]` Algorithm 7 both say duplicate
subspaces have to go and neither says how; `enumerate-subspaces --canonical` is
`[mckay1998]`'s canonical augmentation, which does it with no memory at all. It
returns `⟨2,2,2⟩`'s 36 solution subspaces as the **1 orbit** they are, and visits
**1982x fewer nodes** to do it. Wall clock only improves 1.6x, because finding a
canonical code by walking the whole group spends most of the saving on itself
([`deduplication-cost.md`](oracle_guided_search/deduplication-cost.md)).

## The finding hiding in Table 1

Turning the published table into fixtures turned up something the write-up does
not draw out. Its expensive third step improved the answer in **one of four
cases**. On 4×7 it spent 5020 of its 5044 seconds to return the rank the
previous step already had; on F3 3×6 it never terminated at all.

So the step that costs essentially everything buys essentially nothing, and
making it faster is optimising the part that does not pay. That is why the
interesting result here came from finishing a run rather than from accelerating
one.

## What is where

```
original/                the internship as delivered, frozen
COVERAGE.md              every one of its 89 functions, and where each one went
linear_algebra/          exact arithmetic over GF(p) and over Q, shared by everything
formats/                 tensor, dense matrix and SMS files
cli/                     the one thing the commands share, a clock
testing/                 the assertion helper every module's tests use
run_limits/              how much memory and how many cores one run may take
internship_heuristic/    strand 1: the internship's heuristic, corrected
exhaustive_search/       strand 1: deciding the fewest products outright
map_construction/        strand 1: building the maps both searches run on
orbit_reduction/         strand 1: quotienting all three searches by symmetry
flip_graph/              strand 1: moving a decomposition instead of building one
matrix_sparsification/   strand 2: fewest nonzeros in an operator
satisfiability/          strand 3: the same rank question put to a SAT or SMT solver
curve_bounds/            strand 4: bounds from interpolation on an algebraic curve
integer_programme/       the linear and integer programme layer the curve strand uses
references.md            every paper cited anywhere here, by the keys the code uses
state-of-the-art.md      where the research front is, and which parts of it are here
positioning.md           what this repository adds to it, and what it does not
fixtures/                the maps and operators everything is run on
tools/                   the coverage checker CI runs
site/                    the published page's stylesheet and charts
```

Eight command-line tools. Three ask how few multiplications a map needs and
disagree about what they can prove: **`minimise-rank`** (heuristic),
**`decide-rank`** (complete), **`walk-scheme`** (a walk that moves sideways).
**`decide-rank-by-sat`** puts that same question to somebody else's solver, and
**`list-solvers`** says which backends this machine has. **`curve-bounds`**
answers a different question, bounding the rank from a curve's points rather than
searching for a decomposition. Then
**`sparsify-operator`** for the other strand, and **`make-tensor`** to build a map
to run any of them on.

Every paper any of it implements is named once, in
**[`references.md`](references.md)**, and the code cites a key. Where the
research front actually is: **[`state-of-the-art.md`](state-of-the-art.md)**.
What this repository adds to it, and what it does not:
**[`positioning.md`](positioning.md)**.

| Folder | What it is | Start with |
|---|---|---|
| **[`original/`](original/)** | The 2024 internship, moved here by a rename and never edited since. Two PDFs with the derivations, plus the Julia and Python behind them. | [its README](original/README.md): what was delivered, and the defect list the rewrite was built from |
| **[`formats/`](formats/)** | Reading and writing: tensors, dense matrices, and SMS, the format LinBox and Givaro speak. | [its README](formats/README.md): the three formats and why an operator file is rational |
| **[`fixtures/`](fixtures/)** | The input data, written out in full so the code is checked against bytes rather than against a generator. `.tensor` files are bilinear maps, `.matrix` files are operators. | [its README](fixtures/README.md): the published results table, and what it actually says |
| **[`linear_algebra/`](linear_algebra/)** | The shared layer: matrix, rank, span, exact solve, rank-one decomposition. Templated on the field, so one implementation serves both strands. | [its README](linear_algebra/README.md), and [`costs.md`](linear_algebra/costs.md) for what each operation costs and where exact rationals stop being free |
| **[`internship_heuristic/`](internship_heuristic/)** | Strand 1, the heuristic. One file per step, named for what it guarantees: `smallest_basis` is exact for the basis it picks, `minimise_rank` guarantees nothing. `commands/` builds `minimise-rank`. | [its README](internship_heuristic/README.md) for results, [`method.md`](internship_heuristic/method.md) for the algorithms and their complexity |
| **[`exhaustive_search/`](exhaustive_search/)** | Strand 1, the complete decision: is there an algorithm with exactly `k` products? Exponential, so it settles small maps outright and bounds large ones from below. `commands/` builds `decide-rank`. | [`exhaustive_search.h`](exhaustive_search/exhaustive_search.h) for what it decides and what it costs |
| **[`map_construction/`](map_construction/)** | Strand 1, the inputs: building the bilinear maps every method is then run on. `commands/` builds `make-tensor`. | [`map_construction.h`](map_construction/map_construction.h) |
| **[`orbit_reduction/`](orbit_reduction/)** | Strand 1, the saving. A change of coordinates that fixes the target subspace maps solutions to solutions, so one member of each orbit suffices: 28× on a refutation. | [its README](orbit_reduction/README.md), then [`orbit_cube_boundary.md`](orbit_reduction/orbit_cube_boundary.md) for what the cubes promise a solver |
| **[`flip_graph/`](flip_graph/)** | Strand 1, sideways. A flip rewrites two terms of a working scheme into two others, so every vertex of the walk is valid and the method gives upper bounds only. `commands/` builds `walk-scheme`. | [its README](flip_graph/README.md) |
| **[`matrix_sparsification/`](matrix_sparsification/)** | Strand 2. `heuristic_sparsifier` is Mohamed's row-basis construction, `oracle_sparsifier` the article's two exact oracles. `commands/` builds `sparsify-operator`. | [its README](matrix_sparsification/README.md) for results, [`method.md`](matrix_sparsification/method.md) for the algorithms and their complexity |
| **[`satisfiability/`](satisfiability/)** | Strand 3. Håstad proved deciding tensor rank NP-complete over every finite field, which cuts both ways: `formula_to_tensor` turns 3SAT into a tensor, and the three encoders turn the rank question into one a solver answers. `commands/` builds `decide-rank-by-sat`. | [its README](satisfiability/README.md), [`method.md`](satisfiability/method.md) for the encodings, [`measurements.md`](satisfiability/measurements.md) for what they cost, [`choices.md`](satisfiability/choices.md) for the settings measurement decided, [`complexity.md`](satisfiability/complexity.md) for why "NP-hard" understates it, [`correctness.md`](satisfiability/correctness.md) for what each claim rests on |
| **[`curve_bounds/`](curve_bounds/)** | Strand 4, and the smallest, because most of the method deliberately is not here. For large `m` the best bounds on `GF(q^m)` multiplication come from interpolating on a curve rather than from any search. Two of the four steps are integer arithmetic and are here; two need algebraic geometry and are not. `commands/` builds `curve-bounds`. | [its README](curve_bounds/README.md), which is mostly about what a number out of it does *not* mean |
| **[`integer_programme/`](integer_programme/)** | Simplex, branch and bound, MPS output and a chain of external solvers. Not a strand of its own: it is what the curve strand's step 3 is handed to. `commands/` builds `list-solvers`. | [its README](integer_programme/README.md) |
| **[`famous_tensors.md`](famous_tensors.md)** | The tensors the literature argues about, put through both searches: Strassen's ⟨2,2,2⟩ decided exactly, the W state, cyclic convolution, and where each method gives up. | it, for what the two methods do on maps this repository was not written for |
| **[`COVERAGE.md`](COVERAGE.md)** | Every one of the original's 89 functions, and where each one went: ported, superseded, replaced, or still to come. CI fails if a row is missing. | it, if you want to know whether something survived |
| **[`site/`](site/)** | `style.css`, `chart.js` and `nav.js` for [the page](https://tewf.github.io/bilinear-tensor-optimization/), shared with tewf.github.io. No build step, no CDN. | [`index.html`](index.html) at the root |

Each method folder holds the code itself, its `tests/` and, where it has an entry
point, a `commands/`. The documents for strand 1, its `README.md`, its `method.md`
and the `results.json` the site charts from, sit in `internship_heuristic/`. There
is no page listing every module: a folder with something to say carries its own
`README.md`, and one without says its purpose at the top of its `CMakeLists.txt`.

**Where to start, depending on what you want.** For the mathematics, the two
PDFs in [`original/`](original/). For what was wrong and what changed,
[`original/README.md`](original/README.md). For the results,
[`internship_heuristic/README.md`](internship_heuristic/README.md) and
[`matrix_sparsification/README.md`](matrix_sparsification/README.md). For the code, `linear_algebra/` first; the
two strands are thin on top of it. For the algorithms stated precisely and their
time and space cost, the two `method.md` files.

## Building

Needs a C++20 compiler, CMake and **Givaro** (`sudo apt install libgivaro-dev`).

```sh
cmake -B build -G Ninja && cmake --build build
ctest --test-dir build            # the whole table, about 30 seconds
ctest --test-dir build -LE slow   # skip the three expensive searches
```

`ccache` is picked up automatically when it is installed and ignored when it is
not, so it shortens a rebuild without becoming a second dependency.

Givaro comes from the CASYS team at the LJK, which makes it the supervisor's own
library for exactly this. It supplies what is load-bearing: modular inverse and
exact rational arithmetic that cannot overflow or round. **Nothing here is ever
a float**: both strands search over ranks and over counts of zeros, so an
answer that is nearly right answers a different question.

## Where this stops

Every method here is a heuristic. None proves the decomposition it finds is
optimal, and none settles the bilinear rank problem, which is still open. The 10
for F3 3×6 is a better decomposition than the one on record, not a claim about
the true rank.

---

The original working repository is on the UGA GitLab (`hamlilm/AltBase`) and is
not publicly reachable; this is the public copy.

## Licence and credits

Code and writing are MIT; see [LICENSE](LICENSE). **[NOTICE](NOTICE) matters
here**: MIT covers my own work only. The algorithms come from published papers,
which are cited rather than redistributed, and Givaro is its authors'.
