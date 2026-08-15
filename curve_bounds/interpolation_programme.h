#pragma once

#include <cstddef>
#include <vector>

/// Step 3 of the Chudnovsky-Chudnovsky roadmap: choosing the divisor.
///
/// `[rambaud2014, Thm. 2]` says that if a curve carries a point `Q` of degree
/// `m` and an effective divisor `G = Σ uᵢPᵢ` admitting a suitable `D`, then
///
/// > `µ_sym_q(m) ≤ Σᵢ µ_sym_q(deg Pᵢ, uᵢ)`
///
/// The right-hand side depends only on the degrees and the multiplicities, so
/// choosing them well is an integer programme over
/// [the published bounds](symmetric_bound_table.h) and nothing else. That is
/// what this solves, exactly: the numbers are small and a dynamic programme
/// settles it, so there is no heuristic here and no solver dependency.
///
/// **What this does not do, and it is most of the method.** Steps 2 and 4 of
/// the roadmap are absent. Finding a curve with many points of low degree, and
/// checking that an admissible interpolation system `(G, D, Q)` exists at all
/// -- conditions `l(2D − G) = 0` and `i(D − Q) = 0` -- need Riemann-Roch spaces
/// and curve construction. That is Magma or Sage work, not a Givaro
/// repository. So a result here is **a lower envelope on what the method could
/// give if a curve with that supply of points exists**, and never a bound on
/// `µ_sym_q(m)` by itself. The supply is an input precisely because this cannot
/// compute it.
namespace curve_bounds {

/// How many distinct closed points of a given degree the curve is assumed to
/// carry. This is step 2's output, supplied rather than computed.
struct PointSupply {
    std::size_t degree = 0;
    std::size_t available = 0;
};

/// `count` points of degree `degree`, each taken with multiplicity
/// `multiplicity`.
struct Selection {
    std::size_t degree = 0;
    std::size_t multiplicity = 0;
    std::size_t count = 0;
};

struct Programme {
    bool solved = false;
    /// `Σ µ_sym_q(dᵢ, uᵢ)`, the right-hand side of Theorem 2.
    std::size_t bound = 0;
    /// `Σ uᵢ·dᵢ`, which is `deg G`.
    std::size_t degree_used = 0;
    std::vector<Selection> chosen;
};

/// Minimise Theorem 2's right-hand side over effective divisors of degree
/// **exactly** `divisor_degree`, drawing points from `supply`.
///
/// Exactly, not at most. The roadmap fixes a candidate `deg G` first and then
/// asks for the best divisor of that degree, and it has to: `µ_sym_q(d, u)` is
/// positive for every point, so minimising over smaller divisors too would
/// always answer "take one rational point, cost 1", which is a bound on
/// nothing. The degree is what the existence of an interpolation system will
/// later be checked against, so it is an input, not something to economise on.
///
/// Multiplicities are capped by what the bound table knows: a point whose
/// `µ_sym_q(d, u)` is unpublished cannot be costed, so it is not offered.
Programme minimise_interpolation_bound(const std::vector<PointSupply>& supply,
                                       std::size_t divisor_degree);

}  // namespace curve_bounds
