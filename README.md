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

### 🔹 `Bilinear Rank Over Finite Field`

This part of the project focuses on reducing the rank of bilinear maps over finite fields.  
Given a bilinear map, the code returns another bilinear map that spans the same space but has a lower rank.

### 🔹 `Sparsifying Matrices`

This component provides code for sparsifying matrices.  
Given a matrix, it produces a sparser version while preserving key structural properties.

## 📄 Mathematical Background

Each subproject includes a detailed PDF document explaining the mathematical theory behind the implementations.  
We recommend downloading and reading the PDFs instead of relying on the README, as LaTeX offers a clearer format for expressing mathematical notation and concepts.

## 🚀 Continued independently

The work on these problems continued on my own after the internship, rewritten in
C++ and taken considerably further. It now lives in its own repository:
**[tensor-rank-toolkit](https://github.com/Tewf/tensor-rank-toolkit)**.

This repository stays as the internship was delivered.
