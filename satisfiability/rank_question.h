#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "tensor_file.h"
#include "types.h"

/// Asking one question about a tensor's rank, and answering it with a solver.
///
/// The question is always the same shape: is there a decomposition of this
/// tensor into `products` rank-one terms? What changes is the field it is asked
/// over and therefore which of the three encodings states it, and that choice
/// is made here rather than at the command line, so that anything wanting an
/// answer gets the same dispatch.
///
/// Kept apart from [the command](commands/decide_rank_by_sat_main.cpp), which
/// parses arguments and sweeps `k` over a range. This decides one `k`.
namespace satisfiability {

/// How the question should be put.
struct Approach {
    /// Hand `GF(p)` to cvc5's theory of finite fields instead of encoding it.
    bool use_field_theory = false;
    /// Write parities as clauses rather than `x` lines, for a solver without
    /// native XOR. Most solvers here have none, so this is usually what happens
    /// whether it is asked for or not.
    bool plain_cnf = false;
    /// Quotient by the symmetries a decomposition has: the order of its terms
    /// over any field, and the scaling of its operand vectors over GF(p).
    /// Sound, and worth a great deal on a question expected to answer no.
    bool break_symmetry = false;
    /// Pin a solver instead of taking the best fit.
    std::string solver;

    /// One assignment of the first term's operand variables per orbit, as
    /// literals. Solving once per cube and taking the union is equivalent to
    /// solving the whole formula, provided the cubes really do cover every
    /// first term up to the group; a cube set that misses one turns a yes into
    /// a no. GF(2) only, and empty means one undivided instance.
    std::vector<std::vector<int>> cubes;

    /// Set internally while solving one cube. Callers set `cubes`.
    std::vector<int> cube_literals;

    /// Where to write a DRAT refutation, when the answer is no and the solver
    /// can produce one. Empty means none, and a no is then believed on the
    /// solver's word alone.
    std::string proof_path;

    std::size_t memory_megabytes = 2048;
    std::size_t timeout_seconds = 300;

    /// A smaller budget for the questions a search asks while navigating, as
    /// opposed to the one it cannot avoid. Zero uses `timeout_seconds` for
    /// everything.
    ///
    /// The point is not to answer faster but to spend the big budget once. Cost
    /// is concentrated just below the rank, so a probe that exhausts a small
    /// budget is itself evidence of being there, without that evidence being
    /// treated as an answer: an unknown stays unknown and moves no bound.
    std::size_t probe_seconds = 0;
};

/// What came back.
///
/// `Verdict::Unknown` is a third answer and never folded into `No`: a solver
/// killed by its timeout or its memory cap has not proved a lower bound, and
/// treating silence as refusal is how a search that gave up becomes a claim.
enum class Verdict { Yes, No, Unknown };

struct Answer {
    Verdict verdict = Verdict::Unknown;
    std::string solver_name;
    double seconds = 0;
    /// Size of the refutation written, when one was asked for and produced.
    std::size_t proof_bytes = 0;
    /// The decomposition, when there is one, checked against the tensor before
    /// it is handed back.
    std::vector<Matrix> decomposition;
};

/// Decide whether `tensor` has a decomposition into `products` rank-one terms.
///
/// Throws when no solver is on `PATH`, when the approach and the field do not
/// go together, or when a solver answers yes with a model that does not rebuild
/// the tensor, which is a bug in the encoding rather than a result.
Answer decide_rank(const linear_algebra::Tensor& tensor, std::size_t products,
                   const Approach& approach);

/// What a search established, and whether it is a determination.
struct RankBounds {
    /// Every `k` strictly below this was refused, so the rank is at least here.
    std::size_t lower = 0;
    /// A decomposition into this many was found, so the rank is at most here.
    std::size_t upper = 0;
    /// `lower == upper`, reached without any question going unanswered.
    bool exact = false;
    std::size_t questions_asked = 0;
    double seconds = 0;
    std::vector<Matrix> decomposition;
};

/// Find the rank between two bounds, asking as few expensive questions as
/// possible.
///
/// **The strategy follows from a measured asymmetry rather than from taste.**
/// A satisfiable question is cheap and gets cheaper the further above the rank
/// it is asked: `⟨2,2,2⟩` at sixteen products is answered in 5 ms where seven
/// takes 400. An unsatisfiable one is dear and gets dearer the closer it is
/// asked from below, since the solver must refute everything. Cost is therefore
/// concentrated at the rank and worst just under it.
///
/// Exactly one expensive question is unavoidable: proving the rank is `r` means
/// refusing `r−1`, and nothing gets that for free. Everything else is
/// navigation, and the aim is to spend it on the cheap side.
///
/// So: gallop down from the ceiling in doubling steps, which keeps the probes
/// satisfiable and cheap until one overshoots, then bisect the bracket that
/// overshoot created. Linear from the bottom, which is what this did before,
/// pays an unsatisfiable question at every rank below the answer, and those are
/// the dear ones.
///
/// **This rests on the rank question being monotone**: a decomposition into `k`
/// terms gives one into `k+1` by adding a zero term. The encodings here permit
/// a zero term, so it holds, and it is asserted in the tests rather than
/// assumed, because bisection is unsound without it.
RankBounds find_rank(const linear_algebra::Tensor& tensor, const Approach& approach,
                     std::size_t floor, std::size_t ceiling);

/// Write the question to a file and stop, for a solver of your own.
///
/// Returns what was written, as "N variables, M clauses" or the SMT-LIB
/// equivalent, because the sizes are the interesting part of not solving it.
std::string write_question(const linear_algebra::Tensor& tensor, std::size_t products,
                           const Approach& approach, const std::string& path);

}  // namespace satisfiability
