#include "external_solver.h"

#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "mps_format.h"
#include "whole_numbers.h"

namespace optimisation {

namespace {

std::filesystem::path scratch_path(const char* suffix) {
    return std::filesystem::temp_directory_path() /
           ("optimisation-" + std::to_string(::getpid()) + suffix);
}

std::vector<std::string> lines_of(const std::filesystem::path& path) {
    std::vector<std::string> lines;
    std::ifstream file(path);
    for (std::string line; std::getline(file, line);) lines.push_back(line);
    return lines;
}

std::vector<std::string> words_of(const std::string& line) {
    std::istringstream stream(line);
    std::vector<std::string> words;
    for (std::string word; stream >> word;) words.push_back(word);
    return words;
}

/// An exact rational from whatever decimal a solver printed, exponent included:
/// lp_solve writes `1e+30` for an absent bound and CBC pads to eight places.
Number number_from(const std::string& text) {
    std::size_t position = 0;
    bool negative = false;
    if (position < text.size() && (text[position] == '+' || text[position] == '-')) {
        negative = text[position] == '-';
        ++position;
    }
    const auto digit_here = [&] {
        return position < text.size() && std::isdigit(static_cast<unsigned char>(text[position]));
    };

    Givaro::Integer digits = 0;
    long exponent = 0;
    for (; digit_here(); ++position) digits = digits * 10 + (text[position] - '0');
    if (position < text.size() && text[position] == '.') {
        for (++position; digit_here(); ++position) {
            digits = digits * 10 + (text[position] - '0');
            --exponent;
        }
    }
    if (position < text.size() && (text[position] == 'e' || text[position] == 'E')) {
        exponent += std::strtol(text.c_str() + position + 1, nullptr, 10);
    }

    Givaro::Integer power = 1;
    for (long step = 0; step < (exponent < 0 ? -exponent : exponent); ++step) power *= 10;
    const Number value = exponent < 0 ? Number(digits) / Number(power) : Number(digits) * Number(power);
    return negative ? Number(0) - value : value;
}

/// Values by the names the writer gave the columns. lp_solve, CBC and Gurobi all
/// put `name value` somewhere on the line, whatever else is on it, so the rule is
/// "a word that names a column, then the word after it". The first occurrence
/// wins, which is what keeps lp_solve's later sensitivity table from overwriting
/// the answer with its own numbers.
bool read_by_name(const std::vector<std::string>& lines, std::vector<Number>& values) {
    std::map<std::string, std::size_t> column_of;
    for (std::size_t column = 0; column < values.size(); ++column) {
        column_of[column_name(column)] = column;
    }

    std::vector<bool> seen(values.size(), false);
    for (const std::string& line : lines) {
        const std::vector<std::string> words = words_of(line);
        for (std::size_t position = 0; position + 1 < words.size(); ++position) {
            const auto found = column_of.find(words[position]);
            if (found == column_of.end() || seen[found->second]) continue;
            values[found->second] = number_from(words[position + 1]);
            seen[found->second] = true;
        }
    }
    return std::find(seen.begin(), seen.end(), false) == seen.end();
}

/// GLPK's plain format names nothing: `j <column> <value>` for a mixed integer
/// answer, and `j <column> <status> <primal> <dual>` when the programme turned
/// out to have no integer variables at all.
bool read_by_index(const std::vector<std::string>& lines, std::vector<Number>& values) {
    std::vector<bool> seen(values.size(), false);
    for (const std::string& line : lines) {
        const std::vector<std::string> words = words_of(line);
        if (words.size() < 3 || words[0] != "j") continue;
        const std::size_t column = std::strtoul(words[1].c_str(), nullptr, 10) - 1;
        if (column >= values.size()) continue;
        values[column] = number_from(words.size() >= 4 ? words[3] : words[2]);
        seen[column] = true;
    }
    return std::find(seen.begin(), seen.end(), false) == seen.end();
}

struct Recipe {
    std::string command;
    bool answer_in_log = false;  // lp_solve prints its answer rather than filing it
    bool by_index = false;       // only GLPK
    const char* infeasible = "";
};

std::string quoted(const std::filesystem::path& path) { return "'" + path.string() + "'"; }

Recipe recipe_for(Backend backend, const std::filesystem::path& model,
                  const std::filesystem::path& answer, const std::filesystem::path& log) {
    // `timeout` bounds the solver from outside, so the bound survives this
    // process being killed: a shell-launched solver has no handle to kill it
    // with, and an orphaned one runs until the machine is rebooted.
    const std::string cap = "timeout " + std::to_string(solver_time_limit()) + " ";
    const std::string tail = " </dev/null >" + quoted(log) + " 2>&1";
    switch (backend) {
        case Backend::Gurobi:
            return {cap + "gurobi_cl ResultFile=" + quoted(answer) + " " + quoted(model) + tail, false,
                    false, "infeasible"};
        case Backend::Cbc:
            return {cap + "cbc " + quoted(model) + " -solve -solution " + quoted(answer) + tail, false,
                    false, "infeasible"};
        case Backend::Glpk:
            return {cap + "glpsol --mps " + quoted(model) + " -w " + quoted(answer) + tail, false, true,
                    "no primal feasible solution"};
        case Backend::LpSolve:
            return {cap + "lp_solve -mps " + quoted(model) + " -S3" + tail, true, false,
                    "problem is infeasible"};
        case Backend::BuiltIn:
            break;
    }
    return {};
}

bool log_says(const std::filesystem::path& log, const char* phrase) {
    if (*phrase == '\0') return false;
    std::string text;
    for (const std::string& line : lines_of(log)) text += line + "\n";
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char letter) { return std::tolower(letter); });
    return text.find(phrase) != std::string::npos;
}

}  // namespace

Solution run_backend(Backend backend, const IntegerProgramme& programme) {
    Solution solution;
    solution.solved_by = name_of(backend);
    if (backend == Backend::BuiltIn || !is_available(backend)) return solution;

    const std::filesystem::path model = scratch_path(".mps");
    const std::filesystem::path answer = scratch_path(".sol");
    const std::filesystem::path log = scratch_path(".log");
    std::filesystem::remove(answer);
    {
        std::ofstream file(model);
        file << mps_of(programme);
    }

    const Recipe recipe = recipe_for(backend, model, answer, log);
    // A solver that refuses the model exits non-zero and leaves nothing to read,
    // which the parse below finds out for itself. Only a shell that could not
    // start at all is worth short-circuiting on.
    const bool started = std::system(recipe.command.c_str()) != -1;

    std::vector<Number> values(programme.variables.size(), Number(0));
    const std::vector<std::string> output = lines_of(recipe.answer_in_log ? log : answer);
    const bool complete =
        started && (recipe.by_index ? read_by_index(output, values) : read_by_name(output, values));

    if (complete) {
        // A whole variable's decimal is a rendering of the integer it was, so it
        // is put back on that integer before the model is asked to accept it.
        for (std::size_t index = 0; index < values.size(); ++index) {
            if (programme.variables[index].integral) {
                values[index] = Number(nearest_whole(values[index]));
            }
        }
        if (satisfies(programme, values)) {
            solution.status = Status::Optimal;
            solution.values = values;
            solution.objective = objective_at(programme, values);
        }
    }
    if (solution.status != Status::Optimal && log_says(log, recipe.infeasible)) {
        solution.status = Status::Infeasible;
    }

    std::filesystem::remove(model);
    std::filesystem::remove(answer);
    std::filesystem::remove(log);
    return solution;
}

}  // namespace optimisation
