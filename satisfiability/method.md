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

**The one-hot CNF encoder, and this time on merit.** Ubuntu's `cvc5` 1.1.2 is
built without CoCoALib and cannot run its finite-field solver at all, but the
upstream 1.3.4 GPL build can, so the comparison the two backends were built for
actually happened. Ground truth from the exhaustive search: `GF(9)` and F₃
2×2-term both rank 3, F₃ 2×3-term rank 4.

| Question | one-hot CNF | cvc5 finite fields |
|---|---|---|
| `GF(9)` find 3 | **0.010 s** | 5.44 s |
| `GF(9)` rule out 2 | **0.008 s** | 0.085 s |
| F₃ 2×2 find 3 | **0.014 s** | 3.00 s |
| F₃ 2×2 rule out 2 | **0.011 s** | 0.022 s |
| F₃ 2×3 find 4 | **0.051 s** | no answer in 150 s |
| F₃ 2×3 rule out 3 | **0.099 s** | 2.22 s |

**Every verdict they both produced agrees**, and agrees with the exhaustive
search. That is what the second backend was for, and it did its job: the
hand-written multiplication table, addition chain and one-hot constraints are
corroborated by an encoding that shares none of them.

So `cvc5` stays, demoted to exactly that role. It is not dead code and it is not
a rival; it is the independent check on arithmetic that would otherwise be
mine alone, and it is reachable with `--backend smt`. Neither backend settles
F₃ 3×6 at its known rank of ten within five minutes.

## What it costs against the exhaustive search

Measured 2026-08-16 on one core, against the exhaustive searches on the same
fixtures. Every rank agrees in both directions, which is the point of having two
methods.

| Question | Exhaustive | SAT | |
|---|---|---|---|
| `⟨2,2,2⟩` find 7 | 7 436 nodes | **0.48 s** | both find Strassen |
| `⟨2,2,2⟩` rule out 6 | 25 399 nodes, 0.41 s | **0.31 s** | |
| `⟨2,2,3⟩` rule out 8 | 446 923 nodes, 53.1 s | **34.2 s** | 1.6x |
| GF(16) find 9 | not reachable | **36.7 s** | |
| GF(16) rule out 8 | 105 600 301 nodes, 2328 s | **108.7 s** | **21x** |
| GF(8) rule out 5 | | **4.1 s** | |
| Karatsuba, GF(4), W state | | under 0.02 s | |

**The advantage grows with the instance**, which is the interesting part: level
on `⟨2,2,2⟩`, 1.6 times on `⟨2,2,3⟩`, twenty-one times on GF(16). The exhaustive
search prunes subspaces and, with the orbit quotient, whole orbits at once; the
solver learns clauses, and the harder the instance the more there is to learn.

**This table replaces one that said the opposite.** The first measurements had
this method losing badly, and both reasons were defaults of mine rather than
properties of the method. They are worth stating because they are the whole
lesson:

## Two defaults that were wrong

**Symmetry breaking off.** Ruling out six products for `⟨2,2,2⟩` went from *no
answer in 120 s* to 1.57 s with the ordering on, at least seventy-six times and
the difference between an answer and none. It ships off because an over-strong
constraint would turn a satisfiable instance into UNSAT, which is a wrong lower
bound; it was checked first against all six fixtures of known rank, and every
one is still found. Use it for any question expected to answer no. It is not
implemented for the one-hot GF(p) encoding, and F₃ 3×6 not answering is
probably that.

**CryptoMiniSat preferred.** A GF(2) tensor equation is a parity constraint, so
a solver taking it as one line rather than four clauses ought to win. On these
instances native XOR is worth **nothing measurable**: 1.559 s against 1.563 s on
the same question. Kissat, which cannot read an XOR clause at all, is worth
**five times**: 0.31 s on that question, and 34.2 s against 167.9 s on the next
one up. The reasoning was sound and the measurement disagreed, so Kissat is now
tried first.

## Håstad's reduction

`(2 + n + 2m) × 3n × (3n + m)`, built in `Θ(n·m)` entries, with target rank
`4n + 2m`. The witness from a satisfying assignment costs one rank-one
decomposition per clause and is otherwise arithmetic.

It is a reduction, not a way to solve SAT. A formula of ten variables and
twenty clauses becomes a `52 × 30 × 50` tensor asked for rank 80, which is
enormously harder than the formula it came from. That is what a hardness proof
looks like from the inside, and it is the reason the arrow that gets used in
practice points the other way.
