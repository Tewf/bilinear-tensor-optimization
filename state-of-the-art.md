# Where the research front is, and where this repository sits on it

Three questions get called "fast matrix multiplication" and they have almost
nothing to do with each other. Keeping them apart is the first thing, because a
result in one says nothing about the others.

| | The question | Who is winning it |
|---|---|---|
| **Upper bounds** | find a decomposition with fewer products | search, and since 2022 machine learning |
| **Lower bounds** | prove no smaller one exists | exhaustive methods and SAT, and it is very hard |
| **The exponent** | how does the cost scale asymptotically | the laser method, a separate field entirely |

## Upper bounds: the front moved twice, and neither time by exhaustive search

**`[alphatensor2022]`** put reinforcement learning on it. AlphaZero treated
decomposition as a single-player game and produced 14 236 non-equivalent schemes
for `⟨4,4,4⟩` alone, the first time a learned system improved on human schemes.

**`[kauers2023]`** then matched much of that with no learning at all. The **flip
graph** starts from a decomposition that works and rewrites it: a *flip* swaps a
shared factor between two terms and keeps the sum, so rank is unchanged and the
walk can move sideways for ever, and a *reduction* fires when two terms come to
share two factors, dropping the rank by one. A random walk on that graph found
`⟨5,5,5⟩` in 95.

**`[moosbauer2025]`** added the tensor's own symmetries to the walk and reached
**`⟨5,5,5⟩` in 93 and `⟨6,6,6⟩` in 153**, and `[kauers2025]` generalised the
construction again.

**`[alphaevolve2025]`** then found **`⟨4,4,4⟩` in 48 multiplications over `ℂ`**,
the first improvement on Strassen applied twice, 49, in fifty-six years. That
one needed complex coefficients, which cost arithmetic, so the follow-up work is
about removing them: `[dumas2026]` gives a systematic method that either
converts a complex scheme to a rational one or proves no rational equivalent
exists, generalising Dumas, Pernet and Sedoglavic's earlier ad hoc results.

**The pattern is that every recent record came from walking or evolving a
decomposition that already worked, not from searching a space from nothing.**

## Lower bounds: still the hard direction, and where this repository lives

Nothing above proves anything is optimal. That is the other half, it is where
this repository is, and the front is much closer to us:

- `[bdez2012]` searching subspaces rather than subsets, which
  [`bilinear_rank/`](bilinear_rank/) implements.
- `[covanov2019]` adding the automorphism group, which the orbit work implements.
- `[heule2021]` encoding the question for a SAT solver, which
  [`satisfiability/`](satisfiability/) implements, and `[heule2024]` using SAT
  specifically to rule decompositions out under assumed symmetries.
- `[yang2025]` is the one thing here we do not have: exact decision over finite
  fields in `O*(|F|^(min{R, Σn_d} + (R−n₀)(Σ_{d≠0} n_d)))` and **polynomial
  space**, beating the exponent of naive enumeration outright. **It is
  implemented and public**, at `github.com/coolcomputery/tensor-cpd-search`,
  including a border-CPD search and a Z3 baseline of the same shape as this
  repository's SAT encoding. Its search makes the tensor concise at every node
  of the recursion and prunes with `rref` and `ranksum`, none of which is done
  here.

## The exponent, for completeness

`ω < 2.371339` (`[alman2025]`), improving Duan, Wu and Zhou and then Vassilevska
Williams, Xu, Xu and Zhou. This is the laser method on border rank and shares no
machinery with anything here. It is also famously not implementable: the schemes
behind it are galactic.

## So where are we

**On the lower-bound side we are close to the front and on it in one place.**
The SAT strand is the same technique as `[heule2021]`, and the measurement that
Kissat beats CryptoMiniSat five times on these instances while native XOR is
worth nothing is not in any paper I could find.

**The sharpest positioning available is `[chen2025]`**, and it is uncomfortable
in the useful way. Chen and Kauers apply the flip graph to *polynomial
multiplication*, which is what every `.tensor` fixture here is, and prove the
schemes optimal **with a SAT solver** (Theorem 7). That is precisely the
division of labour between these two strands, published February 2025. This
repository did not invent the pairing; it rebuilt it, on the same problem
class, without knowing.

**How much of our ground they already cover**, once their degrees are
translated into term counts (`n+m+1` is Toom-Cook, so their `(n,m)` is our
`(n+1)x(m+1)`): their proven-optimal list is 2x2, 2x3, 2x4, 2x5, 2x6, 3x3, 3x4,
3x5 and 4x4 over `Z2`. So **`f2_2x2` and `f2_2x3` are theirs already**, and
`f2_3x8`, `f2_4x7` and `f2_5x5` are not, which is where `f2_5x5 = 13` sits.

They also state the asymmetry this repository is built around, in their own
words: "Flip graphs are useful for finding low-rank tensor representations, but
it is not clear how to use the technique for checking whether an optimum has
been reached."

And their open question is the symmetry both strands attacked today: "constant
factors can be freely moved between the components of a rank-one tensor ... it
is unclear what is the best way of doing this. This may be an explanation why an
automated search in the flip graph works best for `K = Z2`." Two independent
answers were measured here on 2026-08-16 and both are negative: quotienting by
that freedom makes the flip graph run over `GF(3)` without making it
competitive, and breaking the same symmetry in the SAT encoding is sound and
does not rescue `f3_3x6`.

**On the upper-bound side we were a decade behind until today**, when the flip
graph landed on the orbit branch and recovered Strassen by walking. That is
`[kauers2023]`, the 2023 method, and reaching `[moosbauer2025]` means adding
symmetry to the walk, which is exactly the group the orbit work already computes.

**What is missing from this repository as a whole**: `[yang2025]`'s algorithm,
symmetry-aware flip graphs, and any evolutionary or learned search. The last is
not a weekend's work and needs hardware this laptop does not have.

**What is missing from the solver strand specifically is a shorter and
different list**, and it is worth separating, because a feature another design
needs is not automatically a gap in this one:

- **Proof logging.** Kissat takes a proof file as its second argument. An UNSAT
  is currently believed because the solver said so, and
  [`satisfiability/complexity.md`](satisfiability/complexity.md) argues at
  length that a "yes" carries its own certificate while a "no" carries nothing.
  A DRAT proof closes exactly that asymmetry and nothing else here does.
- **Incremental solving.** A sweep re-encodes and re-solves from scratch at
  every `k`, so nothing learned at `k` is reused at `k+1`. The clauses differ
  only in the number of terms.
- **The instances that do not answer**: `f3_3x6` at ten and `f2_5x5` at twelve.

**Conciseness reduction is not on that list, and an earlier version of this
file wrongly implied it was.** It is an internal step of a *recursive* search,
which re-compresses the residual tensor at every node; a single monolithic
encoding has no nodes to do it at. Applied once at the top it would help only a
tensor that is not concise, and **every fixture in this repository is concise**,
measured: the flattening ranks equal the shape on all twelve. It would buy
nothing here. Flip graphs are likewise not a gap in this strand, since they
produce upper bounds only and this strand exists for the other direction.

**And a correction about how this file was written.** The first version said
`[yang2025]` was "not implemented here", which was true of this repository and
read as though no implementation existed. One does, it is public, and I had
read the paper's abstract before writing that sentence without looking for its
code. The search that would have found it is `gh search repos "tensor CPD"`,
where it is the first hit; the searches I actually ran were for "tensor rank
SAT" and "matrix multiplication SAT solver", which return nothing at all. The
lesson is in `localAI/memory/check-before-you-assert.md`: search the problem's
own vocabulary, not the vocabulary of the method you already chose.

Full citations, with what each contributes: [`references.md`](references.md).
