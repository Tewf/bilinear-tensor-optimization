#pragma once

#include <cstddef>
#include <string>

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

    /// Where to write a DRAT refutation, when the answer is no and the solver
    /// can produce one. Empty means none, and a no is then believed on the
    /// solver's word alone.
    std::string proof_path;

    std::size_t memory_megabytes = 2048;
    std::size_t timeout_seconds = 300;
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

/// Write the question to a file and stop, for a solver of your own.
///
/// Returns what was written, as "N variables, M clauses" or the SMT-LIB
/// equivalent, because the sizes are the interesting part of not solving it.
std::string write_question(const linear_algebra::Tensor& tensor, std::size_t products,
                           const Approach& approach, const std::string& path);

}  // namespace satisfiability
