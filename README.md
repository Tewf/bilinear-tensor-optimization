# Decomposition of Bilinear Tensors

Research internship at the **LJK** (Laboratoire Jean Kuntzmann, Université
Grenoble Alpes), supervised by **Jean-Guillaume Dumas**.

The rank of a bilinear map is the number of multiplications needed to compute it.
Strassen's seven-instead-of-eight for 2×2 matrices is where fast matrix
multiplication comes from, and finding such decompositions in general is open.
This is a search for them, in two directions.

| Folder | What is in it |
|---|---|
| [`Bilinear_Rank_over_Finite_Field/`](Bilinear_Rank_over_Finite_Field/) | A heuristic that takes a bilinear map over a finite field and returns another spanning the same space with fewer rank-1 components. Write-up, plus Python and Julia implementations. |
| [`Sparsifying_Matrices/`](Sparsifying_Matrices/) | Sparsifying the operators fast multiplication relies on — `argmin nnz(AU)` over invertible `U`. Write-up, Python implementation, Julia notebook. |

Start with the PDF in either folder: they carry the derivations, the worked
polynomial-multiplication example, and the constraints each method needs.

## Where this stops

Both are heuristics. Neither proves the decomposition it finds is optimal, and
neither settles the bilinear rank problem — that is still open.

---

The original working repository is on the UGA GitLab (`hamlilm/AltBase`) and is
not publicly reachable; this is the public copy.
