# Decomposition of Bilinear Tensors

[![CI](https://github.com/Tewf/bilinear-tensor-optimization/actions/workflows/ci.yml/badge.svg)](https://github.com/Tewf/bilinear-tensor-optimization/actions/workflows/ci.yml)
[![Live pages](https://img.shields.io/badge/pages-tewf.github.io%2Fbilinear--tensor--optimization-1f6feb)](https://tewf.github.io/bilinear-tensor-optimization/)
[![Licence](https://img.shields.io/badge/licence-MIT-lightgrey)](LICENSE)

> [Lire en français](README.fr.md)

Research internship at the **LJK** (Laboratoire Jean Kuntzmann, Université
Grenoble Alpes), supervised by **Jean-Guillaume Dumas**, 21 May to 15 July 2024
— and what happened when it was picked up again and finished.

The rank of a bilinear map is the number of multiplications needed to compute
it. Strassen's seven-instead-of-eight for 2×2 matrices is where fast matrix
multiplication comes from, and finding such decompositions in general is open.
This is a search for them, in two directions.

[`original/`](original/) holds the internship exactly as delivered and does not
move. Everything else is a corrected, tested reimplementation in C++, measured
against it. Every number below is produced by code in this repository, and
[CI](https://github.com/Tewf/bilinear-tensor-optimization/actions/workflows/ci.yml)
reruns the whole table on every push.

## What came out of finishing it

**[Rank of bilinear maps](rank/)** — F3 3×6 polynomial multiplication now takes
**10 multiplications instead of the published 11**, because the internship's
final step on that map never terminated and the 11 was a figure from an
abandoned run. The search finishes in **9.9 seconds**. The three maps that did
complete are reproduced exactly, at **5.7×, 582× and 283×** the speed.

| Map | Naive | Result | Internship |
|---|---|---|---|
| F2 5×5 | 25 | **14** · 2.52 s | 14 · 14.42 s |
| F2 3×8 | 24 | **15** · 5.95 s | 15 · 3460.54 s |
| F2 4×7 | 28 | **16** · 17.80 s | 16 · 5044.06 s |
| F3 3×6 | 18 | **10** · 9.92 s | 11, *did not finish* |

**[Sparsifying the operators](sparsify/)** — the strand that reported no
measured result has one. Strassen's encoding operators go from **12 nonzeros to
10**, and the alternative-basis operator the original was tested against goes
from **21 to 10**, in milliseconds. Fewer nonzeros means fewer additions, which
is the cost the multiplication count does not capture.

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
original/     the internship as delivered, frozen — the thing being measured against
fixtures/     the four bilinear maps and three operators everything is run on
exact/        exact linear algebra over GF(p) and over Q, shared by both strands
rank/         strand 1 — fewest multiplications for a bilinear map
sparsify/     strand 2 — fewest nonzeros in an operator
site/         the published page's stylesheet and charts
```

| Folder | What it is | Start with |
|---|---|---|
| **[`original/`](original/)** | The 2024 internship, moved here by a rename and never edited since. Two PDFs with the derivations, plus the Julia and Python behind them. | [its README](original/README.md) — what was delivered, and the defect list the rewrite was built from |
| **[`fixtures/`](fixtures/)** | The input data, written out in full so the code is checked against bytes rather than against a generator. `.tensor` files are bilinear maps, `.matrix` files are operators. | [its README](fixtures/README.md) — the published results table, and what it actually says |
| **[`exact/`](exact/)** | The shared layer: matrix, rank, span, exact solve, rank-one decomposition. Templated on the field, so one implementation serves both strands. | [`linear_algebra.h`](exact/linear_algebra.h) — every operation, with why the original's version was wrong |
| **[`rank/`](rank/)** | Strand 1. The three-step greedy search. Builds `minimise-rank`. | [its README](rank/README.md), then [`cpp/search.h`](rank/cpp/search.h) |
| **[`sparsify/`](sparsify/)** | Strand 2. Row-basis heuristic and two exact oracles. Builds `sparsify-operator`. | [its README](sparsify/README.md), then [`cpp/sparsify.h`](sparsify/cpp/sparsify.h) |
| **[`site/`](site/)** | `style.css`, `chart.js` and `nav.js` for [the page](https://tewf.github.io/bilinear-tensor-optimization/), shared with tewf.github.io. No build step, no CDN. | [`index.html`](index.html) at the root |

Each strand folder holds a `README.md`, a `results.json` the site charts from,
and a `cpp/` with the code, its `tests/`, and a command-line entry point.

**Where to start, depending on what you want.** For the mathematics, the two
PDFs in [`original/`](original/). For what was wrong and what changed,
[`original/README.md`](original/README.md). For the results,
[`rank/README.md`](rank/README.md) and
[`sparsify/README.md`](sparsify/README.md). For the code, `exact/` first — the
two strands are thin on top of it.

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
a float** — both strands search over ranks and over counts of zeros, so an
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
