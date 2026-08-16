#pragma once

#include <cstddef>
#include <vector>

#include "dimacs_file.h"
#include "tensor_file.h"
#include "types.h"

/// `rank(T) ≤ r` over GF(2), as a formula in conjunctive normal form.
///
/// This is the easy half of `[hastad1990]` made useful. Deciding rank is in NP
/// because a decomposition is a certificate, so the question can be handed to a
/// solver instead of enumerated, and over GF(2) the translation costs almost
/// nothing, because a Boolean *is* a field element.
///
/// `T = Σ_{l<r} a⁽ˡ⁾ ⊗ b⁽ˡ⁾ ⊗ c⁽ˡ⁾` becomes:
///
/// ```
/// q[l][i][j]    ↔ a[l][i] ∧ b[l][j]        3 clauses each
/// p[l][i][j][k] ↔ q[l][i][j] ∧ c[l][k]     3 clauses each
/// XOR_l p[l][i][j][k] = t[i][j][k]         one parity per tensor entry
/// ```
///
/// The two stages matter: `q` does not depend on `k`, so sharing it saves
/// `r·n₁·n₂·(n₃−1)` conjunctions over encoding each triple product directly.
///
/// GF(2) only. `[heule2021]` is the same idea, and the method that actually
/// found new ways to multiply 3×3 matrices; keys are
/// [references.md](../references.md).
namespace satisfiability {

/// The formula, and which variable stands for what, so that a solver's answer
/// can be turned back into matrices rather than admired as a truth assignment.
struct BinaryEncoding {
    linear_algebra::Cnf formula;

    std::size_t products = 0;
    std::size_t rows = 0;
    std::size_t columns = 0;
    std::size_t slices = 0;

    /// `left[l * rows + i]`, and likewise for the other two.
    std::vector<int> left;
    std::vector<int> right;
    std::vector<int> output;
};

/// Encode "is there a decomposition of `tensor` into `products` rank-one terms".
///
/// `break_symmetry` adds a lexicographic ordering between consecutive terms,
/// which is sound because permuting the terms of a decomposition gives another
/// decomposition. It is off by default: an over-strong symmetry constraint turns
/// a satisfiable instance into UNSAT, and a wrong "no" is a wrong lower bound,
/// which is the one kind of error this repository must not make quietly.
///
/// Throws unless the tensor is over GF(2), and unless the encoding fits the
/// budget below: the product variables alone number `r·n₁·n₂·n₃`, and a
/// refusal naming that number is a result where a multi-gigabyte file is not.
BinaryEncoding encode_rank_at_most(const linear_algebra::Tensor& tensor, std::size_t products,
                                   bool break_symmetry = false);

/// The ceiling on product variables in one encoding. Ten million is a file of a
/// few hundred megabytes, which is already past what is worth waiting for.
std::size_t variable_budget();
void set_variable_budget(std::size_t variables);

}  // namespace satisfiability
