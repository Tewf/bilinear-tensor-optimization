# The internship as delivered

Frozen. Six files, exactly as they were at the LJK in 2024, moved here by a
rename with no edit so that `git log --follow` still reaches their history. The
rest of the repository is measured against this folder, so it does not move.

| | |
|---|---|
| [`Bilinear_Rank_over_Finite_Field/`](Bilinear_Rank_over_Finite_Field/) | The heuristic for the bilinear rank problem: write-up, a Julia implementation and a fuller Python one. Table 1 of the PDF is the result the rest of this repository has to reproduce. |
| [`Sparsifying_Matrices/`](Sparsifying_Matrices/) | `argmin nnz(AU)` over invertible `U`: write-up, a Python implementation, a Julia notebook. The write-up reports no measured result. |

Both PDFs carry the derivations and the worked polynomial-multiplication
example, and both are worth reading before any of the code.

## What is wrong with it

Not a criticism of research code written to be run once by its author. It is
the list the reimplementation was built from, and each entry is a thing the new
code makes impossible rather than merely avoids.

**Arithmetic that is not exact.** `bilinear.jl:198` reduces modulo `p` only
after the elimination has finished and applies `%`, which overflows a 64-bit
integer and leaves negative residues. Invertibility is then decided from that
result. On the rational side, `sparsifying_…py` inverts in floating point and
takes rank from `numpy.linalg.matrix_rank`, an SVD tolerance. These are searches
over ranks and over counts of zeros, so an answer that is nearly right answers a
different question.

**A search objective computed on doubles.** `sparsifying_…py:107` counts zeros
with `== 0` on raw floats. Running it on `example0` in that file, it sees 86
zeros where 144 exist. The count it *reports* is fine, because `nnz` sends every
entry through `Fraction(...).limit_denominator()`, which snaps `1.85e-18` to
zero. The count was sound; the quantity being maximised was not.

**A branch that cannot be reached.** `sparsifying_…py:268-272` offers the choice
between algorithm 2.3 and algorithm 2.4, and both arms call `algorithm2_3`. The
top-down oracle could never be run.

**A function that falls off the end.** `algorithm4` returns `None` when it finds
nothing, into `v, i = algorithm4(u)`. It does not fire on the operators here, so
it was latent rather than observed.

**Nothing is importable and nothing is tested.** `bilinear.jl:425` calls
`main()` at file scope, so `include`ing the file opens an interactive prompt,
and `:380` evaluates the reply as source with `eval(Meta.parse(readline()))`.
Both entry points read their input from prompts, so no run can be scripted,
timed or repeated, and there are no tests anywhere.

**A name that shadows the language.** `bilinear.jl:330` defines `filter`, while
`:188` still calls Julia's own `filter`. They differ only in how many arguments
they take.

## What the table actually said

Reading Table 1 closely to turn it into fixtures turned up something the
write-up does not draw out, and it changed what the continuation went after.

Step 3 improved the answer in one of four cases. On `3x8` it found 16→15 and
paid about 3432 seconds for it. On `4x7` it ran 5020 seconds and returned the 16
that step 2 already had. On `F3 3x6` it never terminated, which makes the
published 18→11 a step 2 figure from an abandoned run.

That run now finishes, [in under ten seconds and at 10 rather than
11](../bilinear_rank/README.md).
