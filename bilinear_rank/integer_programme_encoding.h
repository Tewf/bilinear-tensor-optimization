#pragma once

#include <cstddef>
#include <vector>

#include "integer_programme.h"
#include "types.h"

/// The rank question as an integer programme, so a MILP solver can answer it
/// beside the SAT solvers and the tree search.
///
/// The equations are Brent's: a decomposition with `k` terms exists exactly when
///
///     sum_r left[r][i] · right[r][j] · output[r][s] = T[s](i, j)
///
/// for every `(i, j, s)`. They are cubic, so a linear programme cannot state
/// them directly; each triple product becomes its own variable pinned to the
/// conjunction of the three, which is the standard linearisation and the reason
/// this encoding is large. `[deza2023]` solves the same equations by constraint
/// programming and `[heule2021]` by SAT; this exists so those two can be
/// compared against a third instrument on identical instances, not because a
/// MILP is expected to win.
///
/// **GF(2) only.** Over a larger field the products are no longer conjunctions
/// and the linearisation needs a variable per field element per product, which
/// is a different encoding rather than a bigger one. The SAT strand does carry a
/// GF(p) encoding; see `prime_field_encoding.h`.
namespace bilinear_rank {

/// The programme, and where to find things in it. A solution is only useful if
/// it can be read back as an algorithm, which is what the accessors are for.
struct EncodedRank {
    optimisation::IntegerProgramme programme;
    std::size_t rank = 0;
    std::size_t rows = 0;
    std::size_t columns = 0;
    std::size_t slices = 0;

    std::size_t left_variable(std::size_t term, std::size_t row) const;
    std::size_t right_variable(std::size_t term, std::size_t column) const;
    std::size_t output_variable(std::size_t term, std::size_t slice) const;
};

/// Is there a decomposition of `slices` with exactly `rank` terms?
///
/// Feasibility, not optimisation: the objective is zero and the question is
/// whether the constraints admit a point at all. That matches how the SAT and
/// tree searches are asked, so the three answer the same question.
EncodedRank encode_rank_question(const Field& field, const std::vector<Matrix>& slices,
                                 std::size_t rank);

/// Pin the first term's two operand vectors.
///
/// Every decomposition can be moved by the map's automorphisms until its first
/// term is a chosen orbit representative, so asking the question once per
/// representative asks it completely while each question is smaller. That is the
/// same reduction `orbit_cubes.h` gives the SAT solver, spelled for a solver
/// that takes equalities rather than clauses.
void fix_first_term(EncodedRank& encoded, const std::vector<Element>& left,
                    const std::vector<Element>& right);

/// The rank-one products a solution stands for, ready for `recovers_map`.
///
/// Terms whose operands came back zero are dropped: they compute nothing, and a
/// solution using fewer terms than asked for is a better answer, not a broken
/// one.
std::vector<Matrix> products_of(const Field& field, const EncodedRank& encoded,
                                const std::vector<optimisation::Number>& values);

}  // namespace bilinear_rank
