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
This is a search for them, in three directions: fewer multiplications, fewer
additions, and the rank asked as a question a SAT solver can answer.

[`original/`](original/) holds the internship exactly as delivered and does not
move. Everything else is a corrected, tested reimplementation in C++, measured
against it. Every number below is produced by code in this repository, and
[CI](https://github.com/Tewf/bilinear-tensor-optimization/actions/workflows/ci.yml)
reruns the whole table on every push.

## What came out of finishing it

**[Rank of bilinear maps](bilinear_rank/)**: F3 3×6 polynomial multiplication now takes
**10 multiplications instead of the published 11**, because the internship's
final step on that map never terminated and the 11 was a figure from an
abandoned run. The search finishes in **9.9 seconds**. The three maps that did
complete are reproduced exactly, at **5.7×, 582× and 283×** the speed.

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
each ruled out exhaustively, so its rank lies in **12 ≤ rank ≤ 14**. On maps it
can finish it reproduces Karatsuba's 3, the write-up's own 5, and the classical
3 and 6 for GF(4) and GF(8) multiplication.

**And the two strands are one pipeline again.** The rank search recovers the
encoding operators ⟨L, R, P⟩ from its decomposition and writes them out; the
sparsification is what they are for:

```sh
minimise-rank fixtures/f2_5x5.tensor --emit-operators out   # 25 -> 14 multiplications
sparsify-operator out_left.matrix                           # 31 -> 27 nonzeros
```

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
bilinear_rank/           strand 1: fewest multiplications for a bilinear map
matrix_sparsification/   strand 2: fewest nonzeros in an operator
satisfiability/          strand 3: the rank as a question for a SAT solver
fixtures/                the maps and operators everything is run on
tools/                   the coverage checker CI runs
site/                    the published page's stylesheet and charts
```

Five command-line tools: **`minimise-rank`** (heuristic), **`decide-rank`**
(exact), **`decide-rank-by-sat`** (exact, via a solver), **`make-tensor`**
(build a map), **`sparsify-operator`**.

Every paper any of it implements is named once, in
**[`references.md`](references.md)**, and the code cites a key.

| Folder | What it is | Start with |
|---|---|---|
| **[`original/`](original/)** | The 2024 internship, moved here by a rename and never edited since. Two PDFs with the derivations, plus the Julia and Python behind them. | [its README](original/README.md): what was delivered, and the defect list the rewrite was built from |
| **[`formats/`](formats/)** | Reading and writing: tensors, dense matrices, and SMS, the format LinBox and Givaro speak. | [its README](formats/README.md): the three formats and why an operator file is rational |
| **[`fixtures/`](fixtures/)** | The input data, written out in full so the code is checked against bytes rather than against a generator. `.tensor` files are bilinear maps, `.matrix` files are operators. | [its README](fixtures/README.md): the published results table, and what it actually says |
| **[`linear_algebra/`](linear_algebra/)** | The shared layer: matrix, rank, span, exact solve, rank-one decomposition. Templated on the field, so one implementation serves both strands. | [its README](linear_algebra/README.md): what each operation costs, and where exact rationals stop being free |
| **[`bilinear_rank/`](bilinear_rank/)** | Strand 1. One file per method, named for what it guarantees: `smallest_basis` is exact for the basis it picks, `minimise_rank` guarantees nothing, `exhaustive_search` decides. `commands/` builds `minimise-rank`, `decide-rank`, `make-tensor`. | [its README](bilinear_rank/README.md) for results, [`method.md`](bilinear_rank/method.md) for the algorithms and their complexity |
| **[`matrix_sparsification/`](matrix_sparsification/)** | Strand 2. `heuristic_sparsifier` is Mohamed's row-basis construction, `oracle_sparsifier` the article's two exact oracles. `commands/` builds `sparsify-operator`. | [its README](matrix_sparsification/README.md) for results, [`method.md`](matrix_sparsification/method.md) for the algorithms and their complexity |
| **[`satisfiability/`](satisfiability/)** | Strand 3. Håstad proved deciding tensor rank NP-complete over every finite field, which cuts both ways: `formula_to_tensor` turns 3SAT into a tensor, and the three encoders turn the rank question into one a solver answers. `commands/` builds `decide-rank-by-sat`. | [its README](satisfiability/README.md), [`method.md`](satisfiability/method.md) for the encodings, [`measurements.md`](satisfiability/measurements.md) for what they cost, [`complexity.md`](satisfiability/complexity.md) for why "NP-hard" understates it |
| **[`bilinear_rank/famous_tensors.md`](bilinear_rank/famous_tensors.md)** | The tensors the literature argues about, put through both searches: Strassen's ⟨2,2,2⟩ decided exactly, the W state, cyclic convolution, and where each method gives up. | it, for what the two methods do on maps this repository was not written for |
| **[`COVERAGE.md`](COVERAGE.md)** | Every one of the original's 89 functions, and where each one went: ported, superseded, replaced, or still to come. CI fails if a row is missing. | it, if you want to know whether something survived |
| **[`site/`](site/)** | `style.css`, `chart.js` and `nav.js` for [the page](https://tewf.github.io/bilinear-tensor-optimization/), shared with tewf.github.io. No build step, no CDN. | [`index.html`](index.html) at the root |

Each strand folder holds a `README.md`, a `method.md` stating its algorithms
precisely, a `results.json` the site charts from, the code itself, its `tests/`
and a `commands/` entry point.

**Where to start, depending on what you want.** For the mathematics, the two
PDFs in [`original/`](original/). For what was wrong and what changed,
[`original/README.md`](original/README.md). For the results,
[`bilinear_rank/README.md`](bilinear_rank/README.md) and
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
