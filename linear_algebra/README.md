# The exact layer

Everything both strands stand on: a dense matrix, a basis kept in reduced row
echelon form, and the operations that go with them. Templated on the field, so
one implementation serves the rank search over `GF(p)` and the sparsification
over `Q`.

| File | Role |
|---|---|
| [`matrix.h`](matrix.h) | Dense row-major matrix, templated on the element type |
| [`field.h`](field.h) | The two fields: `ModularField` = `GF(p)`, `RationalField` = `Q` |
| [`span_basis.h`](span_basis.h) | A basis in reduced row echelon form, built one vector at a time, and the span of a set of slices |
| [`measures.h`](measures.h) | What a thing costs: rank, multiplications, nonzeros |
| [`span_queries.h`](span_queries.h) | What a span contains: `spans_all`, `raises_rank`, `same_row_space` |
| [`solver.h`](solver.h) | Exact solve in a row space, and the inverse built on it |
| [`matrix_ops.h`](matrix_ops.h) | Transpose, product, row and column selection |
| [`decomposition.h`](decomposition.h) | A matrix as a sum of rank-one matrices |
| [`tensor_flattening.h`](tensor_flattening.h) | The three flattenings of a tensor, the rank lower bound `max_d rank(T⁽ᵈ⁾)`, and conciseness |
| [`tensor_contraction.h`](tensor_contraction.h) | Collapsing one axis of a tensor by a vector, which the bounds below are built from |
| [`tensor_rank_sum.h`](tensor_rank_sum.h) | Two rank-sum lower bounds out of one table of contraction ranks: over one affine line, and over every vector |
| [`rank_lower_bound.h`](rank_lower_bound.h) | The largest bound the three methods give, which is what callers wire in |
| [`linear_algebra.h`](linear_algebra.h) | An umbrella including all of the above, and no code of its own |

One file per role, because the umbrella used to be the layer: twelve functions
over five roles in one header, which is what the folder was reorganised to stop
happening elsewhere. Include the part you need; the umbrella is for callers who
want the layer as a whole.

Reading and writing files is [`../formats/`](../formats/), which depends on this
and not the other way round.

> **On the name.** This directory was called `exact/`, for exact arithmetic.
> That collided once the searches were filed by whether they are *exact methods*
> or heuristics, which is a different sense of the word. Exactness of the
> arithmetic is a property of everything here and needs no folder of its own.

## Notation

`p` the characteristic · `r × c` a matrix's shape · `d` the dimension of the
span in play, always `d ≤ min(r, c)` · `k` slices of shape `n × m` · `w = n·m`,
the width a slice occupies when flattened.

## Cost of each operation

Counted in **field operations**, not bit operations; see the caveat below.

| Operation | Time | Space |
|---|---|---|
| `SpanBasis::reduce` | Θ(d·w) |  |
| `SpanBasis::contains` | Θ(d·w) | Θ(w) |
| `SpanBasis::try_add` | Θ(d·w) | Θ(w) added to Θ(d·w) held |
| `rank(A)` | O(r·d·c) | Θ(d·c) |
| `nonzero_count(A)` | Θ(r·c) | Θ(1) |
| `multiplication_count` | O(k·n·d·m) | Θ(d·m) |
| `flattening_lower_bound` | O(n·m·k·(n+m+k)) | Θ(n·m·k) |
| `contraction(v, T, d)` | Θ(n·m·k) | Θ(n·m·k / n_d) |
| `total_rank_sum_lower_bound_on_axis` | Θ(\|F\|^n_d) given the table | Θ(1) |
| `line_rank_sum_lower_bound_on_axis` | O(\|F\|^(2·n_d)·n_d) given the table | Θ(\|F\|^n_d·n_d) |
| `contraction_ranks` | O(\|F\|^n_d·n·m·k) | Θ(\|F\|^n_d) |
| `spans_all(S, T)` | O((\|S\|+\|T\|)·d·w) | Θ(d·w) |
| `solve_in_row_space` | Θ(e·u²) for `u` unknowns, `e` equations | Θ(e·u) |
| `invert(A)`, A square `c × c` | Θ(c⁴) | Θ(c²) |
| `rank_one_decomposition(A)` | Θ(r·c·d²) | Θ(d·r·c) |
| `multiply(A, B)` | Θ(a·b·c) | Θ(a·c) |
| `transpose(A)` | Θ(r·c) | Θ(r·c) |

**The rank sums are the rows above that are not polynomial**, and the only
exponential thing in this layer. They are exponential in the *axis length*, not in
the rank, and the two differ by a whole exponent: the total bound reads the table
once, the line bound enumerates pairs. Measured whole, fastest of three: `gf16`
3 ms, `f2_5x5` 17 ms, `f2_3x8` 40 ms, `f3_3x6` 688 ms. Each refuses an axis past
its own budget rather than trying, and refusing only weakens the bound.

**`SpanBasis` is why the searches finish.** Asking "is this vector new?" is the
question they ask most often, and answering it by computing two ranks from
scratch costs Θ(d²·w) each time. Reducing the candidate against a basis already
in echelon form costs Θ(d·w). On F3 3×6 that change alone took step 3 from 29.8
seconds to 11.3.

**`invert` is a factor of `c` off the textbook**, at Θ(c⁴) where a single
Gauss-Jordan on `[A | I]` is Θ(c³): it runs `c` independent solves, one per row
of the inverse. That is deliberate. It reuses the solver that the rank strand
already exercises rather than introducing a second elimination that could
disagree with the first, and the matrices it inverts here are 4×4. If it were
ever asked to invert something large, this is the line to change.

## The caveat that matters: not all field operations cost the same

Over **`GF(p)`** they genuinely are constant time. The primes here are tiny, an
element is one machine integer, and Givaro's modular arithmetic is a multiply
and a reduce.

Over **`Q`** they are not. A rational carries a numerator and a denominator of
arbitrary size, every operation ends in a gcd to keep the fraction reduced, and
elimination makes those integers grow. A field operation on `L`-bit rationals
costs roughly `Θ(M(L) + gcd)` rather than `Θ(1)`, and `L` itself grows through a
Gauss-Jordan pass.

So the table above bounds the *number of operations*, which is the right
quantity for comparing the algorithms, and it understates wall-clock on the
rational side. It has not mattered yet: the operators being sparsified are 7×4
with entries in ninths, and the whole sparsification strand runs in
milliseconds. It would matter for anything larger.
