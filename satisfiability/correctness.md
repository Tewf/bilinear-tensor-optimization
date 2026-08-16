# What each claim rests on

This folder answers a question whose two verdicts are not equally trustworthy, so
it is worth setting out, claim by claim, what each one depends on and which are
checked rather than argued. The short version is in the last column: **checked**
means a test fails if it stops being true.

| Claim | Rests on | |
|---|---|---|
| A **yes** is right | the decomposition is multiplied out and compared against the tensor | checked |
| A **no** is right | the solver, *unless* a refutation is written and verified | checked when `--proof` |
| An **unknown** is not a no | a third verdict carried everywhere, never folded | checked |
| The flattening bound is sound | a rank-one tensor has rank-one flattenings, and rank is subadditive | checked |
| Walking up finds the rank | monotonicity: a `k`-decomposition gives a `k+1` one | checked |
| The rank is **exact** | a satisfiable `k`, a definite refusal below it, and a start at or below the flattening bound | checked |
| Symmetry breaking removes no solution | the terms are a set; `(λa)⊗(μb)⊗(νc)` is the same term when `λμν = 1` | checked |
| A cube split is complete | the representatives cover every first term up to the group | **not checked here** |

## The asymmetry that shapes everything else

A **yes** carries its own proof. The model is turned back into rank-one matrices
and recombined, and the answer is refused if that does not reproduce the tensor.
This is the FNP verifier of [`complexity.md`](complexity.md) written out, and it
means a yes cannot be wrong unless the verifier is.

A **no** carries nothing. It is a claim about every assignment the solver did not
visit. Until `drat-trim` was wired in, it rested entirely on kissat being
correct; with `--proof` the refutation is checked by a program sharing no code
with the solver that produced it, and **a refutation that fails to verify throws
rather than printing a warning**, because it would mean the encoding or the
solver is wrong and a false lower bound is the error nothing downstream catches.

An **unknown** is neither. A solver killed by its timeout or its memory cap has
proved nothing, and folding that into a no would turn giving up into a lower
bound. It moves no bound, in the search or in the cube combination.

## Monotonicity, and why it is asserted rather than assumed

Walking up and stopping at the first satisfiable `k` gives the rank only if the
question is monotone: satisfiable at `k` implies satisfiable at `k+1`. It holds
because the encodings permit an all-zero term, so a `k`-decomposition embeds in
`k+1` by padding. That is a one-line argument and it is still asserted in the
tests, because the day someone forbids the zero term to tighten the encoding,
the search silently starts returning wrong answers.

## Exactness is a property of the run, not of the tool

A first satisfiable `k` is the rank only if everything below was **refused**, and
that needs two things the tool tracks: the sweep began at or below the flattening
bound, and no question came back unknown. Start it by hand above the bound and it
reports "at most", not "exactly". The distinction is the difference between a
determination and a bound, and it is the reason the sweep keeps that bookkeeping
rather than reporting whatever it found first.

## Where a false lower bound could still come from

Two constraints, each sound alone, whose conjunction is not: a cube fixes the
first term to an orbit representative, and the ordering constraint demands the
first term be lexicographically least. Nothing makes a representative least. The
ordering therefore starts at term 1 whenever a cube is supplied, and the test
that would catch a regression uses an exhaustive cube set over the first term's
left vector, where the union must agree with solving whole.

The remaining exposure is the one row above marked not checked. The cube
machinery is validated against a partition made by hand; whether some *other*
group's representatives really do cover every first term is a property of that
group, not of this code, and it has to be established where the group is. If a
cube set misses a first term, a yes becomes a no, silently.
