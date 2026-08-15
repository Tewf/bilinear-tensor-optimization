# Coverage: every function of the original, and where it went

The repository claims the internship was carried over whole. This is the table
that makes the claim checkable, and
[`tools/check_coverage.py`](tools/check_coverage.py) runs in CI to assert that
every `def` and `function` under [`original/`](original/) appears below.

**Status** is one of:

| | |
|---|---|
| **ported** | It is in the C++ now, under the name given |
| **to port** | Not yet. **No row carries this today** |
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
| `first_nonzero_row` | 675 | ported | the pivot search in `SpanBasis::try_add`, and `first_nonzero_row` in `algorithm_recovery.cpp` |
| `solve_linear_combination` | 520 | ported | `linear_algebra::solve_in_row_space` |
| `solve_linear_combinations` | 620 | superseded | same solver, matrices flattened |
| `matrix_decomposer` | 546 | ported | `linear_algebra::rank_one_decomposition` |
| `rank1_decomposer` | 562 | superseded | wrapper around `matrix_decomposer` |

### The heuristic search, now in `span_enumeration.*`, `smallest_basis.*` and `minimise_rank.*`

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

### The exact search: `bilinear_rank/exhaustive_search.*`

| Function | Line | Status | Where |
|---|---|---|---|
| `has_one_rank_basis` | 227 | ported | `rank_one_maps_within` |
| `expand_subspace` | 245 | ported, fixed | same name. The original returned the extended subspace from its base case and the rank-one basis from its recursive one; only the second is usable |
| `iteration_search` | 270 | ported | `fewest_products_by_sweep` |
| `binary_Search` | 314 | ported, fixed | `fewest_products_by_bisection`. The original passed 3 arguments to a 5-parameter function |
| `binary_Search_auto` | 280 | superseded | it differed only by calling `auto_expand_subspace`, which is defined nowhere in the file, so it could never have run |
| `separater` | 347 | ported | `lowest_rank_partition` |
| `bottom_up` | 367 | ported | `build_bottom_up` |
| `bottom_up_auto` | 357 | superseded | it differed only by using `binary_Search_auto`, which could never run |

### ⟨L, R, P⟩ recovery: `bilinear_rank/algorithm_recovery.*`

This is what connects the two strands: `L` and `R` are exactly the operators
[`matrix_sparsification/`](matrix_sparsification/) then makes sparse.

| Function | Line | Status | Where |
|---|---|---|---|
| `reverse_custom_tensor_product` | 681 | ported, fixed | `recover_operands`. The original read the right operand from row 0 while computing the first nonzero row and using it for everything else, so a slice with a zero first row gave a zero operand row |
| `solve_tensor_equation` | 647 | ported | `recover_decoder` |
| `custom_tensor_product` | 654 | ported | `encoded_products` |
| `matrix_tensor_multiplication` | 694 | ported | the combination step inside `computed_map` |
| `find_scalar_multiple` | 663 | ported | `scalar_multiple`, which also doubles as the rank-one check |

### Map construction: `bilinear_rank/map_construction.*`

| Function | Line | Status | Where |
|---|---|---|---|
| `create_Tensor_model_of_polynomial_multiplication` | 190 | ported | `polynomial_multiplication_tensor`, and the `make-tensor` tool that writes it out |
| `create_of_zeroes` | 184 | superseded | `Matrix` is zero-initialised on construction |
| `tensor_modular` | 215 | ported | `reduce_tensor_modulo` |
| `substract` | 199 | ported | the subtraction step inside `reduce_tensor_modulo` |
| `is_irreducible` | 751 | ported | `is_irreducible`, over Givaro's `Poly1FactorDom`, the original's only real use of sympy |
| `read_poly` | 772 | ported | `make-tensor --field`, which refuses a reducible modulus instead of re-prompting |

### Automorphism reduction of `G`: `bilinear_rank/candidate_pool.*`

The PDF's conclusion names this as future work and the code has a partial start.

| Function | Line | Status | Where |
|---|---|---|---|
| `auto_verify` | 171 | ported | `row_space_contains`: its rank comparison is exactly row-space containment |
| `auto_sort` | 177 | ported | `has_equivalent`, with `one_per_row_space` built on it |

### Entry points

| Function | Line | Status | Where |
|---|---|---|---|
| `main` | 787 | ported | `bilinear_rank/commands/minimise_rank_main.cpp`, minus the parts that are steps 3-5 |
| `read_matrix_of_matrices` | 708 | replaced | `formats/tensor_file.*`; the original parsed a Python literal from a prompt |
| `read_single_matrix` | 730 | replaced | `formats/dense_matrix_file.*` |

---

## `original/Sparsifying_Matrices/sparsifying_fast_matrix_multiplication_operators.py`

18 functions. All the algorithms are ported; only the file formats are not.

| Function | Line | Status | Where |
|---|---|---|---|
| `nnz` | 75 | ported | `linear_algebra::nonzero_count`, exact instead of `limit_denominator` on doubles |
| `Cnk` | 63 | ported | `matrix_sparsification::combinations` |
| `matrix_u` | 66 | ported | `linear_algebra::select_columns` |
| `inverse` | 69 | replaced | `linear_algebra::invert`, exact rather than `numpy.linalg.inv` |
| `gaussian_elimination` | 17 | ported | `linear_algebra::solve_in_row_space` |
| `supp` | 72 | ported | the support scan inside `find_validator` |
| `omega_validator3` | 135 | ported | `matrix_sparsification::find_validator` |
| `omega_validator4` | 145 | superseded | the same λ by elimination instead of inversion; one implementation now |
| `algorithm3` | 99 | ported | the inner loop of `sparsify_bottom_up` |
| `algorithm4` | 112 | ported | the inner loop of `sparsify_top_down`; it fell off the end returning `None` |
| `algorithm2_3` | 78 | ported | `matrix_sparsification::sparsify_bottom_up` |
| `algorithm2_4` | 89 | ported | `matrix_sparsification::sparsify_top_down`; it was unreachable from the prompt |
| `algorithm5` | 124 | ported | `matrix_sparsification::row_basis_sparsifier` |
| `print_matrix` | 244 | ported | `formats::to_string` |
| `input_matrix` | 233 | replaced | `formats/dense_matrix_file.*` |
| `input_sparse_matrix` | 203 | ported | `formats/sms_file.*`; `sparsify-operator` reads a `.sms` path directly |
| `print_sms` | 247 | ported | `write_sms`, choosing the M or R tag itself |
| `main` | 252 | ported | `matrix_sparsification/commands/sparsify_main.cpp` |

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
