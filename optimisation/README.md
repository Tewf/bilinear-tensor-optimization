# Constrained minimisation

A linear objective under linear constraints, with some variables required to be
whole, and whichever solver this machine has to answer it.

Several things here minimise something subject to constraints and all of them do
it by enumeration: the sparsification oracles walk column subsets, and the
interpolation bound of the curve strand is an integer programme written out by
hand. This is the model they can be handed to instead.

## The chain

    gurobi  →  cbc  →  glpk  →  lp_solve  →  built-in

The ranking is fixed; what is installed is not. A machine with nothing gets the
built-in and a slower answer, a machine that acquires a Gurobi licence tomorrow
uses it without a line changing, and `list-solvers` says which is which:

    $ ./build/optimisation/list-solvers

Backends are found on `PATH` at run time and never linked, which is how the
satisfiability strand already treats `kissat` and `cvc5`. The tree therefore
builds identically everywhere and the choice stays a run-time one.

## What is believed, and from whom

An outside solver reports in decimal, so what comes back from one is a **point,
not a result**. It is read onto exact rationals, whole variables are put back on
the integers their decimals were a rendering of, and the whole thing goes through
`satisfies` before anyone sees it. The objective is then recomputed from the
model rather than taken from what the solver printed.

**A `no` is only ever believed from the built-in.** A point can be checked and
is; a claim that no point exists cannot be, so `solve` treats that claim as one
more backend declining to answer and carries on down the chain. Proving a
programme infeasible is therefore as slow as the exact solver, deliberately: a
false `Infeasible` is the one answer here that nothing downstream would catch.

## The built-in

Two-phase simplex under Bland's rule over the relaxation, then branch and bound
on a variable the relaxation left fractional. Exact rationals throughout, so
there is no tolerance to tune and no degenerate pivot that is only degenerate to
fifteen digits. It is the slowest backend and the only one that never has to be
believed, which is why it is both the last resort and the arbiter.

`node_limit` bounds the tree. Reaching it returns `Exhausted` carrying the best
point found, which bounds the optimum without proving it; `Optimal` is a proof.

## What the file format cost

Everything reaches a solver as fixed-column MPS, and three of its traps are
silent. All three were found by running the solvers, not by reading about them.

- **An integer variable with no stated upper bound is binary** to CBC and to
  GLPK, and unbounded to lp_solve. The same file is then two different problems.
  Every variable states both bounds, `PL` and `MI` included.
- **The integrality markers belong in fields 3 and 5**, not 4 and 6, and free-form
  MPS is not an escape: CBC parses `BOUNDS` by character position whatever the
  rest of the file looks like.
- **MPS has no direction.** The objective row is always minimised, so a
  maximisation is written negated. Without that every maximisation comes back at
  its minimum, which is feasible, and therefore passes every check except
  comparison with the right answer.

Rows are scaled by the lowest common denominator of their own entries, so
coefficients and bounds arrive as integers and nothing is lost on the way in.

## Verified here

`test_optimisation` runs one battery through every installed backend and the
built-in, and they must agree. On this machine that is CBC 2.10.11, GLPK 5.0 and
lp_solve 5.5. **The Gurobi recipe follows its documented `ResultFile` output and
is unverified**, there being no licence here to test it against.
