# The exact layer

Everything both strands stand on: a dense matrix, a basis kept in reduced row
echelon form, and the operations that go with them. Templated on the field, so
one implementation serves the rank search over `GF(p)` and the sparsification
over `Q`.

| File | Role |
|---|---|
| [`matrix.h`](matrix.h) | Dense row-major matrix, templated on the element type |
| [`fields.h`](fields.h) | The two fields: `ModularField` = `GF(p)`, `RationalField` = `Q` |
| [`span_basis.h`](span_basis.h) | A basis in reduced row echelon form, built one vector at a time |
| [`linear_algebra.h`](linear_algebra.h) | Rank, span, exact solve, inverse, rank-one decomposition |
| [`tensor.h`](tensor.h) · [`rational_matrix_io.h`](rational_matrix_io.h) | Reading the two fixture formats |

## Notation

`p` the characteristic · `r × c` a matrix's shape · `d` the dimension of the
span in play, always `d ≤ min(r, c)` · `k` slices of shape `n × m` · `w = n·m`,
the width a slice occupies when flattened.

## Cost of each operation

Counted in **field operations**, not bit operations — see the caveat below.

| Operation | Time | Space |
|---|---|---|
| `SpanBasis::reduce` | Θ(d·w) | — |
| `SpanBasis::contains` | Θ(d·w) | Θ(w) |
| `SpanBasis::try_add` | Θ(d·w) | Θ(w) added to Θ(d·w) held |
| `rank(A)` | O(r·d·c) | Θ(d·c) |
| `nonzero_count(A)` | Θ(r·c) | Θ(1) |
| `multiplication_count` | O(k·n·d·m) | Θ(d·m) |
| `spans_all(S, T)` | O((\|S\|+\|T\|)·d·w) | Θ(d·w) |
| `solve_in_row_space` | Θ(e·u²) for `u` unknowns, `e` equations | Θ(e·u) |
| `invert(A)`, A square `c × c` | Θ(c⁴) | Θ(c²) |
| `rank_one_decomposition(A)` | Θ(r·c·d²) | Θ(d·r·c) |
| `multiply(A, B)` | Θ(a·b·c) | Θ(a·c) |
| `transpose(A)` | Θ(r·c) | Θ(r·c) |

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
