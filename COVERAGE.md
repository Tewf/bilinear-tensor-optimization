# Coverage: every function of the original, and where it went

The repository claims the internship was carried over whole. This is the table
that makes the claim checkable, and
[`tools/check_coverage.py`](tools/check_coverage.py) runs in CI to assert that
every `def` and `function` under [`original/`](original/) appears below.

**Status** is one of:

| | |
|---|---|
| **ported** | It is in the C++ now, under the name given |
| **to port** | Not yet, with the step of the plan that will |
| **superseded** | Another function does the same job; the row says which |
| **replaced** | The job is done a different and better way; the row says how |

`bilinear.jl` is a subset of `bilinear_maps.py` under different names, so it is
listed last and each row points at its Python twin rather than repeating the
mapping.

---

## `original/Bilinear_Rank_over_Finite_Field/bilinear_maps.py`

47 functions. This is the fullest version of the rank strand and the reference
for it.

### Linear algebra, now in `linear_algebra/`

| Function | Line | Status | Where |
|---|---|---|---|
| `mod_inv` | 18 | replaced | Givaro `ModularField::inv`; the original's was hand-rolled |
| `row_reduce` | 32 | replaced | `SpanBasis`, kept in reduced echelon form incrementally |
| `Row_Reduce` | 466 | superseded | duplicate of `row_reduce` |
| `row_reduce_modified` | 566 | superseded | duplicate of `row_reduce` |
| `matrix_rank` | 79 | ported | `linear_algebra::rank` |
| `is_linearly_independent` | 85 | ported | `SpanBasis::contains`, `raises_rank` |
| `Span_it` | 121 | ported | `linear_algebra::spans_all` |
| `first_nonzero_row` | 675 | ported | the pivot search inside `SpanBasis::try_add` |
| `solve_linear_combination` | 520 | ported | `linear_algebra::solve_in_row_space` |
| `solve_linear_combinations` | 620 | superseded | same solver, matrices flattened |
| `matrix_decomposer` | 546 | ported | `linear_algebra::rank_one_decomposition` |
| `rank1_decomposer` | 562 | superseded | wrapper around `matrix_decomposer` |

### The heuristic search, now in `bilinear_rank/heuristic_search.*`

| Function | Line | Status | Where |
|---|---|---|---|
| `Number_of_multiplications` | 128 | ported | `linear_algebra::multiplication_count` |
| `multiplier` | 377 | ported | `bilinear_rank::combine` |
| `generate_arrays` | 166 | ported | `bilinear_rank::coefficient_vector` |
| `span_of` | 384 | ported | the candidate enumeration inside `smallest_basis` |
| `smallest_base` | 401 | ported | `bilinear_rank::smallest_basis` |
| `rank_minimizer` | 419 | ported | `bilinear_rank::minimise_rank` |
| `filter` | 441 | ported | `bilinear_rank::improving_candidates` (it shadowed Python's `filter`) |
| `rank1_bases` | 781 | ported | `bilinear_rank::rank_one_candidates` |
| `generate_G` | 135 | ported | `bilinear_rank::all_rank_one_maps` |
| `my_product` | 139 | replaced | outer products of normalised vectors, instead of growing tuples and testing each partial result |
| `independ` | 152 | replaced | normalisation gives one representative per scalar class, so nothing needs de-duplicating |

### The exact search — **step 4**, `bilinear_rank/exhaustive_search.*`

| Function | Line | Status | Where |
|---|---|---|---|
| `has_one_rank_basis` | 227 | to port | counts the rank-one maps of `G` inside `span(W)` |
| `expand_subspace` | 245 | to port | the complete decision: can `W` extend to `k` rank-one maps? |
| `iteration_search` | 270 | to port | sweeps `k` upward; minimises **over decompositions containing `base`**, not globally |
| `binary_Search` | 314 | to port, fixed | binary search on `k`; passes 3 args to a 5-parameter function |
| `binary_Search_auto` | 280 | to port, fixed | same, and calls `auto_expand_subspace`, which is never defined anywhere |
| `separater` | 347 | to port | splits a map's slices into the lowest-rank ones and the rest |
| `bottom_up` | 367 | to port | builds from the empty set upward by rank level — the from-scratch variant |
| `bottom_up_auto` | 357 | to port | same, through the `_auto` search |

### ⟨L, R, P⟩ recovery — **step 3**, `bilinear_rank/algorithm_recovery.*`

This is what connects the two strands: `L` and `R` are exactly the operators
[`matrix_sparsification/`](matrix_sparsification/) then makes sparse.

| Function | Line | Status | Where |
|---|---|---|---|
| `reverse_custom_tensor_product` | 681 | to port | a decomposition → the encoding operators `L`, `R` |
| `solve_tensor_equation` | 647 | to port | a decomposition → the decoding operator `P` |
| `custom_tensor_product` | 654 | to port | `L`, `R` → the tensor they encode |
| `matrix_tensor_multiplication` | 694 | to port | a matrix times a tensor, for rebuilding from `P` |
| `find_scalar_multiple` | 663 | to port | the scalar `k` with `A·k = B`, used by the recovery |

### Map construction — **step 5**, `bilinear_rank/map_construction.*`

| Function | Line | Status | Where |
|---|---|---|---|
| `create_Tensor_model_of_polynomial_multiplication` | 190 | to port | currently only implicit, in how `fixtures/*.tensor` were generated |
| `create_of_zeroes` | 184 | superseded | `Matrix` is zero-initialised on construction |
| `tensor_modular` | 215 | to port | reduces the tensor modulo an irreducible polynomial, giving GF(pⁿ) multiplication |
| `substract` | 199 | to port | the subtraction step inside `tensor_modular` |
| `is_irreducible` | 751 | replaced | Givaro `Poly1FactorDom::is_irreducible`; the original's only real use of sympy |
| `read_poly` | 772 | to port | reads a modulus and rejects it unless irreducible |

### Automorphism reduction of `G` — **step 6**, `bilinear_rank/candidate_pool.*`

The PDF's conclusion names this as future work and the code has a partial start.

| Function | Line | Status | Where |
|---|---|---|---|
| `auto_verify` | 171 | to port | is there an automorphism `z` with `z·x = y`? |
| `auto_sort` | 177 | to port | keeps one representative per automorphism class |

### Entry points

| Function | Line | Status | Where |
|---|---|---|---|
| `main` | 787 | ported | `bilinear_rank/minimise_rank_main.cpp`, minus the parts that are steps 3–5 |
| `read_matrix_of_matrices` | 708 | replaced | `formats/tensor_file.*`; the original parsed a Python literal from a prompt |
| `read_single_matrix` | 730 | replaced | `formats/dense_matrix_file.*` |

---

## `original/Sparsifying_Matrices/sparsifying_fast_matrix_multiplication_operators.py`

18 functions. All the algorithms are ported; only the file formats are not.

| Function | Line | Status | Where |
|---|---|---|---|
| `nnz` | 75 | ported | `linear_algebra::nonzero_count`, exact instead of `limit_denominator` on doubles |
| `Cnk` | 63 | ported | `matrix_sparsification::subsets` |
| `matrix_u` | 66 | ported | `linear_algebra::select_columns` |
| `inverse` | 69 | replaced | `linear_algebra::invert`, exact rather than `numpy.linalg.inv` |
| `gaussian_elimination` | 17 | ported | `linear_algebra::solve_in_row_space` |
| `supp` | 72 | ported | the support scan inside `find_validator` |
| `omega_validator3` | 135 | ported | `matrix_sparsification::find_validator` |
| `omega_validator4` | 145 | superseded | the same λ by elimination instead of inversion; one implementation now |
| `algorithm3` | 99 | ported | the inner loop of `sparsify_exhaustive` |
| `algorithm4` | 112 | ported | the inner loop of `sparsify_top_down`; it fell off the end returning `None` |
| `algorithm2_3` | 78 | ported | `matrix_sparsification::sparsify_exhaustive` |
| `algorithm2_4` | 89 | ported | `matrix_sparsification::sparsify_top_down`; it was unreachable from the prompt |
| `algorithm5` | 124 | ported | `matrix_sparsification::row_basis_sparsifier` |
| `print_matrix` | 244 | ported | `formats::to_string` |
| `input_matrix` | 233 | replaced | `formats/dense_matrix_file.*` |
| `input_sparse_matrix` | 203 | to port | **step 7** — SMS, the LinBox and Givaro sparse format |
| `print_sms` | 247 | to port | **step 7** — writing SMS |
| `main` | 252 | ported | `matrix_sparsification/sparsify_main.cpp` |

---

## `original/Bilinear_Rank_over_Finite_Field/bilinear.jl`

24 functions, a subset of the Python under different names. Each row points at
its twin above.

| Function | Line | Twin in `bilinear_maps.py` |
|---|---|---|
| `matrix_rank` | 5 | `matrix_rank` |
| `mod_inv` | 11 | `mod_inv` |
| `row_reduce` | 15 | `row_reduce` |
| `is_linearly_independent` | 68 | `is_linearly_independent` |
| `Span_it` | 91 | `Span_it` |
| `Number_of_multiplications` | 100 | `Number_of_multiplications` |
| `generate_arrays` | 108 | `generate_arrays` |
| `my_product` | 116 | `my_product` |
| `independ` | 133 | `independ` |
| `generate_G` | 150 | `generate_G` |
| `create_of_zeroes` | 154 | `create_of_zeroes` |
| `create_Tensor_model_of_polynomial_multiplication` | 159 | same name |
| `tensor_modular` | 173 | `tensor_modular` |
| `subtract` | 190 | `substract` (spelled with the extra s in Python) |
| `gaussian_elimination_over_p` | 198 | `solve_linear_combination` |
| `matrix_rank1_decomposer` | 245 | `matrix_decomposer` |
| `rank1_base` | 276 | `rank1_bases` |
| `multiplier` | 285 | `multiplier` |
| `smallest_base` | 303 | `smallest_base` |
| `rank_minimizer` | 314 | `rank_minimizer` |
| `filter` | 330 | `filter` (it shadowed Julia's `Base.filter`, still called at `:188`) |
| `Has_one_rank_basis` | 346 | `has_one_rank_basis` |
| `expand_subspace` | 356 | `expand_subspace` |
| `main` | 375 | `main`, without the ⟨L,R,P⟩ recovery or the extension-field path |
