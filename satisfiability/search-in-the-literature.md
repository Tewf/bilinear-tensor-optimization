# What the field already settled about asking a solver in order

The order this module asks its questions in was chosen twice by argument and
twice wrongly, then settled by measurement in [`search.md`](search.md). This file
is the other half: the question has a literature, the literature had already
answered it, and reading it first would have saved both wrong answers.

## Naming the problem, which is the step that was skipped

Say it in one sentence: **find the least `k` at which a decision oracle answers
yes, by asking it a sequence of yes/no questions.** That shape is not ours and
neither is the difficulty. The field calls it **SAT-based optimisation**, or
**iterative MaxSAT solving** when the objective is a clause count, and its three
orders have standard names that predate this repository by a decade.

| Their name | Ours | Their gloss |
|---|---|---|
| linear search **UNSAT-SAT** | the ascending sweep | refines a lower bound; all calls but the last return unsatisfiable |
| linear search **SAT-UNSAT**, model-improving | descending | refines an upper bound |
| **binary search** | bisection | fewest calls |

Our names were invented here, which is why three searches for them returned
nothing: zero results meant the query was wrong, not that nothing existed.

## The finding that decides it

`[morgado2013]` assesses all of them. The verdict is one sentence, and it belongs
to **`[heras2011]`**, the paper about binary search specifically, which the survey
then restates with three of the same authors: "although binary search is optimal
in terms of the number of calls to a SAT oracle, it has seldom been used in
practical MaxSAT solvers; in particular, given that all clauses are relaxed,
cardinality constraints are fairly complex."

**Optimal in calls is not optimal in time.** The survey also prices the calls:
the two linear searches need, in the worst case, a number of oracle calls
exponential in the instance size, while binary search needs a linear number. So
the theoretical case for bisection is real, and it still loses in practice.

## Why that transfers here, and why it also weakens

It transfers as a **prediction that was borne out**: on GF(16) bisection makes
the fewest calls and finishes **last**, 113.614 s against 110.094 s for the
schedule that wins. Measured, on a problem class the survey never looked at.

It weakens for a reason worth stating, because it is the difference between
citing a paper and understanding it. The survey's exponential-versus-linear gap
is in the *size of the cost range*, which for MaxSAT is exponential in the number
of soft clauses. Here the range is `[flattening bound, n₁n₂]`, a dozen values at
most, so log versus linear is a handful of calls either way. **The asymptotic
argument for bisection has almost no room to act on instances this narrow**, and
what remains is dominated by which questions a schedule happens to ask. That is
why the whole choice is worth about 3% here, and the survey does not say that
because nobody had a range this small.

## The one thing the survey says is not implemented, and we ship it

The same survey records that there are **no known implementations of linear
search UNSAT-SAT for MaxSAT**, though it is used elsewhere, for minimal
unsatisfiable subsets. That schedule is this module's default. Not by
independence of mind: it is the right default here for reasons that do not hold
in MaxSAT, namely that the flattening bound is often already the rank, so
ascending asks one question and stops, and that it is the only schedule which
never reads the ceiling and so cannot be misled by a loose one.

## Positionnement, stated so it can be contradicted

**Not new**: the three schedules, the observation that bisection loses on time,
the hybrid instinct.

**New, as far as reading found**: the per-question price table in
[`search.md`](search.md). Every question here is a separate deterministic
process, so its cost is independent of the order it is reached in, and pricing
all of them prices every schedule exactly and at once, including unimplemented
ones. **A MaxSAT solver cannot do this**, because it is incremental: a call's
cost there depends on what the solver learned in the calls before it. The
no-linking rule that costs this module incrementality is what buys it exact
schedule pricing. That is a trade, not a gap.

**The baseline** for this strand is therefore `[morgado2013]`'s taxonomy and the
MaxSAT Evaluation record beside it, and the review is finished because that
baseline can now be named. On the other side of the module, a refutation is
measured against `[wang2026]`: [`../state-of-the-art.md`](../state-of-the-art.md).
