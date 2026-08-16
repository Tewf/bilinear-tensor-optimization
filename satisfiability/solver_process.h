#pragma once

#include <cstddef>
#include <string>

#include "dimacs_file.h"
#include "smtlib_file.h"

/// Running a solver, which is a program on the machine and not a library here.
///
/// Nothing links against a solver. The build depends on Givaro and nothing
/// else, and a machine without a solver builds and tests fine. It just cannot
/// answer this particular question, and says so. That is deliberate: the
/// encoding is the contribution, the solver is a tool, and pinning a tool into
/// the build makes the repository harder to run for no gain.
///
/// Three are looked for, each for a reason:
///
/// | | Why it is here |
/// |---|---|
/// | `cryptominisat` | native XOR clauses, which is exactly the shape of a GF(2) tensor equation |
/// | `kissat` | the strongest solver available on unsatisfiable instances, which is where proving a lower bound lives |
/// | `cvc5` | the theory of finite fields, so GF(p) needs no encoding at all |
///
/// **Kissat is tried first, and that is a measurement rather than a guess.** A
/// GF(2) tensor equation is a parity constraint, so CryptoMiniSat taking it as
/// one line instead of four clauses ought to win; on these instances it is
/// worth nothing at all, 1.559 s against 1.563 s on the same question. Kissat's
/// raw strength on unsatisfiable instances is worth five times, and the
/// expensive questions here are exactly the ones that answer no. Numbers in
/// [`method.md`](method.md).
namespace satisfiability {

/// A solver found on `PATH`, and whether it understands XOR clauses directly.
struct SatSolver {
    bool found = false;
    std::string name;
    std::string path;
    bool native_xor = false;
};

/// Look for a SAT solver. `prefer_xor` asks for one that takes parity
/// constraints natively, which is only worth it on GF(2); `named` pins a
/// choice instead of taking the preference.
SatSolver find_sat_solver(bool prefer_xor, const std::string& named = "");

/// Where `cvc5` is, or empty. Its finite-field solver needs a CoCoALib build,
/// which distribution packages have been known to omit, so a solver that is
/// present may still refuse the query. That refusal comes back as no verdict.
std::string find_smt_solver();

struct SolverRun {
    bool solver_found = false;
    std::string solver_name;
    /// False when the solver was found but gave no verdict: killed by the
    /// timeout or the memory cap, or refusing the theory. **Never read this as
    /// unsatisfiable**, because that would turn giving up into a proof of a
    /// lower bound.
    bool answered = false;
    bool satisfiable = false;
    linear_algebra::Model model;
    linear_algebra::SmtModel field_model;
    double seconds = 0;
};

/// Write the formula out and solve it, under a memory cap and a wall clock.
///
/// Both limits are arguments rather than constants because the only machine
/// this has run on shares its memory with other long searches, and a solver
/// that takes the box down has answered nothing.
SolverRun solve(const linear_algebra::Cnf& formula, const SatSolver& solver,
                std::size_t memory_megabytes = 2048, std::size_t timeout_seconds = 300);

/// The same for an SMT problem in the theory of finite fields.
SolverRun solve_in_field(const linear_algebra::SmtProblem& problem,
                         std::size_t memory_megabytes = 2048,
                         std::size_t timeout_seconds = 300);

}  // namespace satisfiability
