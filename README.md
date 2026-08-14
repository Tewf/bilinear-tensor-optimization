# Excellence Internship – Decomposition of Bilinear Tensors

This repository contains the work completed during an **Excellence Internship** at Université Grenoble Alpes, carried out within the **Jean Kuntzmann Laboratory (LJK)**.
🔗 Original project repository on GitLab:  
[https://gricad-gitlab.univ-grenoble-alpes.fr/hamlilm/AltBase](https://gricad-gitlab.univ-grenoble-alpes.fr/hamlilm/AltBase)  

📅 **Internship duration**: May 21 – July 15, 2024 (40 days)  
👤 **Intern**: Mohamed Ali HAMLIL  
🧪 **Supervisor**: Jean-Guillaume Dumas  
🏛️ **Laboratory**: LJK – Université Grenoble Alpes

## 🎯 Project Topic

**Decomposition of Bilinear Tensors**  
The main objective was to study and implement techniques for generating alternative bases using sparse decompositions for matrix multiplication, as well as minimizing the rank of bilinear tensors through specific automorphisms, with applications to polynomial multiplication.

## 🔍 Main Tasks

- Literature review on tensor decomposition techniques.
- Implementation of algorithms in Python and Julia for alternative basis generation.
- Study and minimization of bilinear tensor rank under structural constraints (automorphisms).
- Application of developed methods to matrix and polynomial multiplication.

## 🧠 Skills Developed

- Tensor rank and decomposition
- Linear, bilinear, and polynomial algebra
- Efficient algorithms & fast arithmetic
- Complexity theory
- Development in Python and Julia
- Scientific writing and presentation

## 📁 Project Structure

### 🔹 [`Bilinear_Rank_over_Finite_Field/`](Bilinear_Rank_over_Finite_Field/)

Reducing the rank of bilinear maps over finite fields: given a bilinear map, the code returns another that spans the same space with lower rank.

| File | |
|---|---|
| [`bilinear_rank_problem_heuristic.pdf`](Bilinear_Rank_over_Finite_Field/bilinear_rank_problem_heuristic.pdf) | The write-up — the heuristic and why it works |
| [`bilinear_maps.py`](Bilinear_Rank_over_Finite_Field/bilinear_maps.py) | Python implementation |
| [`bilinear.jl`](Bilinear_Rank_over_Finite_Field/bilinear.jl) | Julia implementation |

### 🔹 [`Sparsifying_Matrices/`](Sparsifying_Matrices/)

Given a matrix, produce a sparser one while preserving the structure that matters — applied to the operators of fast matrix-multiplication algorithms.

| File | |
|---|---|
| [`matrix_sparsification_summary.pdf`](Sparsifying_Matrices/matrix_sparsification_summary.pdf) | The write-up — the sparsification problem and the approach |
| [`sparsifying_fast_matrix_multiplication_operators.py`](Sparsifying_Matrices/sparsifying_fast_matrix_multiplication_operators.py) | Python implementation |
| [`matrix_sparsification_julia.ipynb`](Sparsifying_Matrices/matrix_sparsification_julia.ipynb) | Julia notebook — Gaussian elimination, the ω-validators and algorithms 2–4 |

## 📄 Mathematical Background

Both write-ups above are mine and carry the theory behind the code. Read them
rather than this README for the mathematics — LaTeX expresses the notation far
better than Markdown can.
