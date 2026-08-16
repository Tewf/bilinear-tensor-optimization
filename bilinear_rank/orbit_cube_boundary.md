# The orbit cube boundary

Two branches of `bilinear-tensor-optimization` meet here, and this file is the
whole contract. `search-and-symmetry` supplies representatives; the
satisfiability module consumes them. Written 2026-08-16, after the earlier
attempt lost both halves to an OOM with the boundary agreed only in conversation.

Both sides break the symmetry of the same formula. Theirs does it with a term
*ordering*; this one does it with the map's full automorphism group. Each is
sound alone. **Their conjunction is not**, and that is the one thing below that
cannot be got wrong.

## What is supplied

`bilinear_rank/orbit_cubes.h`, on `search-and-symmetry`:

```cpp
std::vector<std::vector<int>> orbit_cubes(
    const Field& field, const std::vector<Matrix>& slices,
    std::size_t rows, std::size_t inner, std::size_t columns,
    const std::vector<int>& left_variables,
    const std::vector<int>& right_variables);
```

One cube per orbit, each a list of literals over the **consumer's** variable
numbers. Together they cover every possible first term up to the group, so
solving the formula once per cube decides the same question as solving it whole.

## The layout

Pass the whole variable arrays, not a slice of them. The offset arithmetic lives
on the supplying side so that no consumer repeats it:

    left_variables [term * rows  * inner   + coordinate]
    right_variables[term * inner * columns + coordinate]

which is the layout `satisfiability/binary_encoding.h` already builds. A positive
literal asserts the coordinate is one, a negative literal that it is zero: the
DIMACS convention, so the encoder needs to hand over no header to honour it.

`slices` is the tensor the consumer encoded, and the shape is **checked against
`⟨rows, inner, columns⟩` rather than taken on trust**. The representatives are
written for that map in that coordinate order, so naming the wrong shape would
pin the first term to a map the tensor does not contain, and refute a
decomposition that exists. That is why the shape is an argument and not a comment.

## The rule that makes the conjunction sound

**A cube pins the first term and orders nothing.** A representative is not
obliged to be lexicographically least, so the consumer's own term ordering
**must skip term zero**. Conjoin an unmodified ordering with a cube and the two
together exclude decompositions that exist, which reads as a refutation.

Second guard, from `orbit_plan.md`: orbit pruning requires the pool to be closed
under the action. `all_rank_one_maps` is closed; `rank_one_candidates` is not.
Refuse rather than assume.

## The hazard, stated plainly

A wrong or incomplete symmetry constraint turns a satisfiable formula
unsatisfiable **silently**. That is a false lower bound, and nothing downstream
catches it: a *yes* carries its own proof and gets multiplied out against the
map, but a *no* rests on the search having been complete.

An incomplete group only costs speed. Fewer verified elements means more orbits
means a bigger search, still exhaustive and still sound. Only an element that was
never verified, or a cube set that misses a first term, can corrupt an answer.

## Validation, which is the deliverable

No refutation built on cubes is believed until both of these pass.

| test | what it establishes |
|---|---|
| `ctest -R orbit_cubes` | representatives partition the pool, cubes are well formed, a wrong shape is refused |
| `ctest -R orbit_cubes_preserve_the_answer` | the same question whole and split into cubes agrees, at 7 products where `⟨2,2,2⟩` is satisfiable and at 6 where it is not |

The second is labelled `slow` with a 900 s timeout. **Both passed on 2026-08-16,
the second in 238.8 s**, nearly all of it the refutation at 6. It is the only test
that links both sides of this boundary, because the question needs both.

This is what closes the one unchecked row in `satisfiability/correctness.md`,
which lives on `solvers-and-certificates` with the rest of that module: *"a cube
split is complete ... not checked here"*. The row stands as written until the two
branches meet on `main`, because only there is the test in the same tree as the
claim it discharges.
