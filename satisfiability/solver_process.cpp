#include "solver_process.h"

#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>

#include <unistd.h>

namespace satisfiability {

namespace {

/// The first entry of `PATH` holding an executable called `name`.
std::string on_path(const std::string& name) {
    const char* path = std::getenv("PATH");
    if (path == nullptr) return {};

    std::istringstream entries(path);
    std::string entry;
    while (std::getline(entries, entry, ':')) {
        if (entry.empty()) continue;
        const std::filesystem::path candidate = std::filesystem::path(entry) / name;
        std::error_code ignored;
        if (std::filesystem::is_regular_file(candidate, ignored)) return candidate.string();
    }
    return {};
}

std::string run_and_capture(const std::string& command) {
    std::string captured;
    std::FILE* pipe = popen(command.c_str(), "r");
    if (pipe == nullptr) return captured;

    std::array<char, 4096> buffer{};
    while (std::fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        captured += buffer.data();
    }
    pclose(pipe);
    return captured;
}

}  // namespace

std::string find_solver() { return on_path("cryptominisat"); }

SolverRun solve(const linear_algebra::Cnf& formula, bool native_xor,
                std::size_t memory_megabytes, std::size_t timeout_seconds) {
    SolverRun run;
    const std::string solver = find_solver();
    if (solver.empty()) return run;
    run.solver_found = true;

    std::error_code ignored;
    const std::filesystem::path scratch =
        std::filesystem::temp_directory_path(ignored) /
        ("tensor-rank-" + std::to_string(::getpid()) + ".cnf");
    {
        std::ofstream out(scratch);
        linear_algebra::write_dimacs(out, formula, native_xor);
    }

    // ulimit is per-process and needs no privileges, unlike a cgroup, and this
    // is a child we already spawn through a shell.
    const std::string command = "sh -c 'ulimit -v " +
                                std::to_string(memory_megabytes * 1024) + "; exec timeout " +
                                std::to_string(timeout_seconds) + " \"" + solver + "\" \"" +
                                scratch.string() + "\" 2>/dev/null'";

    const auto started = std::chrono::steady_clock::now();
    const std::string output = run_and_capture(command);
    run.seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();

    std::istringstream lines(output);
    run.model = linear_algebra::read_dimacs_model(lines);
    run.answered = run.model.answered;
    run.satisfiable = run.model.satisfiable;

    std::filesystem::remove(scratch, ignored);
    return run;
}

}  // namespace satisfiability
