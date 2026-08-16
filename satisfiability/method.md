# The method, exactly

`p` is the characteristic, `r` the number of products asked for, and the tensor
is `n₁ × n₂ × n₃`: rows, columns, and slices.

## The problem, as a formula

> Are there vectors `a⁽ˡ⁾ ∈ F^{n₁}`, `b⁽ˡ⁾ ∈ F^{n₂}`, `c⁽ˡ⁾ ∈ F^{n₃}`, for
> `l < r`, with `t[i][j][k] = Σ_l a⁽ˡ⁾[i]·b⁽ˡ⁾[j]·c⁽ˡ⁾[k]` for every `i, j, k`?

`r(n₁+n₂+n₃)` unknowns and `n₁n₂n₃` equations. Everything below is that,
written for a different solver.

## GF(2), as CNF

```
q[l][i][j]    ↔ a[l][i] ∧ b[l][j]        3 clauses
p[l][i][j][k] ↔ q[l][i][j] ∧ c[l][k]     3 clauses
XOR_l p[l][i][j][k] = t[i][j][k]         1 parity constraint
```

| | |
|---|---|
| Variables | `r(n₁+n₂+n₃)` + `r·n₁n₂` + `r·n₁n₂n₃` |
| Clauses | `3r·n₁n₂` + `3r·n₁n₂n₃`, plus `n₁n₂n₃` parities of width `r` |

The two stages are the only optimisation and they matter: `q` does not depend on
`k`, so sharing it saves `r·n₁n₂(n₃−1)` conjunctions against encoding each
triple product directly.

A solver without native XOR expands each parity into `r−1` fresh variables and
`4(r−1)` clauses. That is `--plain-cnf`, and it is why the parities are kept
apart from the clauses until the file is written.

Measured, with `--emit-cnf`:

| Tensor | `r` | Variables | Clauses | Expanded |
|---|---|---|---|---|
| F₂ 2×3 | 5 | 195 | 474 | |
| ⟨2,2,2⟩ | 7 | 644 | 1 744 | 1 028 vars, 3 280 clauses |
| ⟨3,3,3⟩ | 23 | 19 251 | 56 619 | |

⟨3,3,3⟩ at Laderman's 23 is a file of about a megabyte. The exhaustive search
cannot go near that tensor; this is a question a solver can be asked.

## GF(p), one-hot

Each unknown becomes `p` variables with exactly one true; each product and sum
becomes the field's table as implications `(¬x[e₁] ∨ ¬y[e₂] ∨ z[e₁∘e₂])`.

| | |
|---|---|
| Variables | `p·(r(n₁+n₂+n₃) + r·n₁n₂ + 2r·n₁n₂n₃)` |
| Clauses | about `2p²·r·n₁n₂n₃`, plus `1 + p(p−1)/2` per one-hot group |

So about `2p²` clauses per term per entry, nine times the GF(2) cost at `p = 3`.
At-most-one is written pairwise because at these primes that is three clauses
and a ladder encoding would be more machinery than it saves.

The sum is a chain: `s₀ = 0`, `s_{l+1} = s_l + q·c`, and one unit clause fixing
`s_r` to the entry. A wrong entry therefore forces two members of one one-hot
group true at once, which the at-most-one clause catches. That is what makes
the test able to detect a bad encoding at all.

## GF(p), SMT

```
(assert (= (ff.add (ff.mul (ff.mul a_l_i b_l_j) c_l_k) …) (as ff<t> F)))
```

`n₁n₂n₃` assertions over `r(n₁+n₂+n₃)` constants, and nothing else. The cost is
not in the file; it is in the Gröbner-basis procedure behind `QF_FF`
(`[ozdemir2023]`).

## Which GF(p) backend survives

**The comparison never happened.** Both solvers were installed on 2026-08-16 and
Ubuntu's `cvc5` 1.1.2 turns out to be built without CoCoALib:

```
(error "cvc5 can't solve field problems since it was not configured with --cocoa")
```

so its finite-field decision procedure cannot run on this machine at all. The
one-hot CNF encoder therefore carries `GF(p)` **by default rather than on
merit**, and nobody should read it as having won a contest. It was the only
entrant that could start.

`field_theory_encoding.h` stays as a writer, because the file it produces is
correct and a cvc5 with CoCoALib would decide it. Getting one means a source
build with `--cocoa` or an upstream release binary, which is a change to the
machine's setup and Mohamed's call rather than an agent's.

## What it costs against the exhaustive search

Measured 2026-08-16, one core, against `[bdez2012]`-style exhaustive runs on the
same fixtures. Every rank below agrees with the exhaustive search in both
directions, which is the point of having two methods.

| Question | Exhaustive | SAT | |
|---|---|---|---|
| `⟨2,2,2⟩` find 7 | 7 436 nodes | **0.48 s** | both find Strassen |
| `⟨2,2,2⟩` rule out 6 | 25 399 nodes, 0.41 s | **1.57 s** | |
| `⟨2,2,3⟩` rule out 8 | 446 923 nodes, 53.1 s | **167.9 s** | 3.2x slower |
| GF(16) find 9 | not reachable | **36.7 s** | |
| GF(8) rule out 5 | | **4.1 s** | |
| Karatsuba, GF(4), W state | | **under 0.02 s** | |
| GF(16) rule out 8 | 105 600 301 nodes, 2328 s | **no answer in 413 s** | stopped, see below |
| F₃ 3×6 find 10 | rank 10 known | **no answer in 300 s** | one-hot, no symmetry breaking |

**The honest summary is that this is not a faster method, it is a differently
shaped one.** Finding a decomposition is what a solver is good at, and it finds
Strassen from nothing in under half a second. Proving there is none is where it
loses: on `⟨2,2,3⟩` at eight products the exhaustive search is three times
quicker, and the gap grows with the tensor. A solver has to refute every
assignment; the exhaustive search prunes whole subspaces and, with the orbit
quotient, whole orbits of them at once.

## Symmetry breaking is not optional after all

It ships off by default because an over-strong constraint would turn a
satisfiable instance into UNSAT, which is a wrong lower bound. The measurement
says it should be on for any question expected to answer no:

| `⟨2,2,2⟩` rule out 6 | without | **no answer in 120 s** |
|---|---|---|
| | with | **1.57 s** |

At least seventy-six times, and the difference between an answer and none. It
was checked for soundness first, on all six fixtures whose rank is known: every
one is still FOUND with the ordering on, so it rules out no decomposition that
exists. That is evidence, not a proof, and the flag stays explicit for that
reason.

**It is not implemented for the one-hot GF(p) encoding, and that shows.** `F₃
3×6` at its known rank of ten is 10 122 variables and 41 324 clauses, and the
solver had not answered in five minutes. The GF(2) measurement above says what
is probably missing rather than proving it: the same instance class gains at
least seventy-six times from an ordering constraint, and the prime-field encoder
has none. That is the first thing to try, not a conclusion that GF(p) is out of
reach.

## Two runs stopped by heat rather than by a cap

GF(16) at eight products was given 2 600 s and stopped at **413 s** with the
package at 91 °C, three sessions having been active on this laptop that evening.
It reported "no answer", which is the third verdict and not a no; peak RSS was
5.5 MB, so memory was never the constraint and heat was. The comparison against
the exhaustive search's 2 328 s is therefore **unresolved**, and it is recorded
that way rather than rounded into the story the other rows tell.

## Håstad's reduction

`(2 + n + 2m) × 3n × (3n + m)`, built in `Θ(n·m)` entries, with target rank
`4n + 2m`. The witness from a satisfying assignment costs one rank-one
decomposition per clause and is otherwise arithmetic.

It is a reduction, not a way to solve SAT. A formula of ten variables and
twenty clauses becomes a `52 × 30 × 50` tensor asked for rank 80, which is
enormously harder than the formula it came from. That is what a hardness proof
looks like from the inside, and it is the reason the arrow that gets used in
practice points the other way.
