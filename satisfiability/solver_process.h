#pragma once

#include <cstddef>
#include <string>

#include "dimacs_file.h"

/// Running a solver, which is a program on the machine and not a library here.
///
/// Nothing links against a solver. The build depends on Givaro and nothing
/// else, and a machine without a solver builds and tests fine — it just cannot
/// answer this particular question, and says so. That is deliberate: the
/// encoding is the contribution, the solver is a tool, and pinning a tool into
/// the build makes the repository harder to run for no gain.
///
/// `cryptominisat` is the one looked for, because a GF(2) tensor equation is a
/// parity constraint and it takes those natively rather than through four
/// clauses and a fresh variable per pair.
namespace satisfiability {

/// Where `cryptominisat` is, or empty if it is not on `PATH`.
std::string find_solver();

struct SolverRun {
    bool solver_found = false;
    /// False when the solver was found but gave no verdict: killed by the
    /// timeout or the memory cap. **Never read this as unsatisfiable** — that
    /// would turn giving up into a proof of a lower bound.
    bool answered = false;
    bool satisfiable = false;
    linear_algebra::Model model;
    double seconds = 0;
};

/// Write the formula out and solve it, under a memory cap and a wall clock.
///
/// Both limits are arguments rather than constants because the only machine
/// this has run on shares its memory with other long searches, and a solver
/// that takes the box down has answered nothing.
SolverRun solve(const linear_algebra::Cnf& formula, bool native_xor,
                std::size_t memory_megabytes = 2048, std::size_t timeout_seconds = 300);

}  // namespace satisfiability
