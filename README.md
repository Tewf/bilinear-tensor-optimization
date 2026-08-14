# Decomposition of Bilinear Tensors — LJK research internship

**How few multiplications does it take to multiply?**

The rank of a bilinear map is the number of multiplications needed to compute
it. Schoolbook multiplication of two 2×2 matrices takes 8; Strassen showed in
1969 that 7 suffice, and that single saved multiplication is what makes fast
matrix multiplication possible at all. Finding such decompositions — writing a
tensor as a sum of as few rank-1 terms as possible — is the *bilinear rank
problem*, and it is open in general.

This internship attacked it from two angles: a heuristic for lowering the rank
of a bilinear map over a finite field, and a method for sparsifying the
operators that fast multiplication algorithms rely on.

| | |
|---|---|
| **Intern** | Mohamed Ali HAMLIL |
| **Supervisor** | Jean-Guillaume Dumas |
| **Laboratory** | LJK — Laboratoire Jean Kuntzmann, Université Grenoble Alpes |
| **Placement** | Excellence Internship, 40 days |

## What I did

- Formalised the problem in tensor terms: represent a bilinear map as its
  associated tensor, where **each row is the set of multiplications computing one
  output coefficient**, then look for a decomposition `T = φ · T′` whose factor
  `T′` has lower rank. Worked through polynomial multiplication as the running
  example.
- Built a **heuristic for reducing bilinear rank** over finite fields — given a
  bilinear map, it returns another spanning the same space with fewer rank-1
  components. Implemented twice, in Python and in Julia.
- Studied rank minimisation under **structural constraints (automorphisms)**.
- Built a **matrix sparsification** method — `argmin nnz(AU)` over invertible `U`
  — and applied it to the operators of fast matrix-multiplication algorithms,
  since sparser operators mean fewer additions around the multiplications.
- Reviewed the tensor-decomposition literature, and wrote both results up.

## Contents

### [`Bilinear_Rank_over_Finite_Field/`](Bilinear_Rank_over_Finite_Field/)

Reducing the rank of bilinear maps over finite fields: given a bilinear map, the code returns another that spans the same space with lower rank.

| File | |
|---|---|
| [`bilinear_rank_problem_heuristic.pdf`](Bilinear_Rank_over_Finite_Field/bilinear_rank_problem_heuristic.pdf) | The write-up — the heuristic and why it works |
| [`bilinear_maps.py`](Bilinear_Rank_over_Finite_Field/bilinear_maps.py) | Python implementation |
| [`bilinear.jl`](Bilinear_Rank_over_Finite_Field/bilinear.jl) | Julia implementation |

### [`Sparsifying_Matrices/`](Sparsifying_Matrices/)

Given a matrix, produce a sparser one while preserving the structure that matters — applied to the operators of fast matrix-multiplication algorithms.

| File | |
|---|---|
| [`matrix_sparsification_summary.pdf`](Sparsifying_Matrices/matrix_sparsification_summary.pdf) | The write-up — the sparsification problem and the approach |
| [`sparsifying_fast_matrix_multiplication_operators.py`](Sparsifying_Matrices/sparsifying_fast_matrix_multiplication_operators.py) | Python implementation |
| [`matrix_sparsification_julia.ipynb`](Sparsifying_Matrices/matrix_sparsification_julia.ipynb) | Julia notebook — Gaussian elimination, the ω-validators and algorithms 2–4 |

## Where this stops

Both heuristics search for better decompositions; neither proves that the
decomposition it finds is optimal, and neither settles the bilinear rank problem
in general — it is open. The two write-ups above are mine and carry the
derivations, the worked polynomial-multiplication example, and the constraints
under which each method holds.

The original working repository is on the UGA GitLab (`hamlilm/AltBase`) and is
not publicly reachable; this is the public copy.
