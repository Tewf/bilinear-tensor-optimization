/// Run the heuristic on a bilinear map and report what each step cost.
///
/// The original's entry point read the map from an interactive prompt and
/// evaluated the reply as source code. This takes a file and arguments, so a
/// run can be scripted, timed, and repeated.
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include "search.h"
#include "tensor.h"

namespace {

double seconds_since(std::chrono::steady_clock::time_point started) {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
}

/// Checked after every step, not only in the tests: the result has to still
/// generate the map it came from, or the number means nothing.
bool verify(const exact::Field& field, const std::vector<exact::Matrix>& current,
            const std::vector<exact::Matrix>& original, const std::string& step) {
    if (exact::spans_all(field, current, original)) return true;
    std::cerr << "FAILED: after " << step << " the result no longer generates the map\n";
    return false;
}

void report(const std::string& step, std::size_t multiplications, std::size_t slices,
            double cumulative_seconds, bool as_json) {
    if (as_json) {
        std::cout << "  {\"step\": \"" << step << "\", \"multiplications\": " << multiplications
                  << ", \"slices\": " << slices << ", \"cumulative_seconds\": "
                  << cumulative_seconds << "}\n";
    } else {
        std::cout << "  " << step << ": " << multiplications << " multiplications, " << slices
                  << " slices, " << cumulative_seconds << " s cumulative\n";
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: minimise-rank <tensor-file> [--steps 1|2|3] [--json]\n";
        return 2;
    }

    std::string path = argv[1];
    int wanted_steps = 3;
    bool as_json = false;
    for (int argument = 2; argument < argc; ++argument) {
        const std::string option = argv[argument];
        if (option == "--json") {
            as_json = true;
        } else if (option == "--steps" && argument + 1 < argc) {
            wanted_steps = std::stoi(argv[++argument]);
        } else {
            std::cerr << "unrecognised option: " << option << "\n";
            return 2;
        }
    }

    const exact::Tensor tensor = exact::read_tensor_file(path);
    const exact::Field field(tensor.characteristic);
    const auto started = std::chrono::steady_clock::now();

    std::cout << (as_json ? "[\n" : path + "\n");
    report("naive", exact::multiplication_count(field, tensor.slices), tensor.slices.size(), 0.0,
           as_json);

    std::vector<exact::Matrix> current = rank_search::smallest_basis(field, tensor.slices);
    if (!verify(field, current, tensor.slices, "step 1")) return 1;
    if (as_json) std::cout << ",";
    report("step 1", exact::multiplication_count(field, current), current.size(),
           seconds_since(started), as_json);

    if (wanted_steps >= 2) {
        const std::vector<exact::Matrix> own = rank_search::rank_one_candidates(field, current);
        const std::vector<exact::Matrix> shortlist =
            rank_search::improving_candidates(field, current, own);
        current = rank_search::minimise_rank(field, current, shortlist);
        if (!verify(field, current, tensor.slices, "step 2")) return 1;
        if (as_json) std::cout << ",";
        report("step 2", exact::multiplication_count(field, current), current.size(),
               seconds_since(started), as_json);
    }

    if (wanted_steps >= 3) {
        const std::vector<exact::Matrix> everything =
            rank_search::all_rank_one_maps(field, tensor.rows(), tensor.columns());
        std::cerr << "step 3 pool: " << everything.size() << " rank-one maps\n";
        const std::vector<exact::Matrix> shortlist =
            rank_search::improving_candidates(field, current, everything);
        std::cerr << "step 3 shortlist: " << shortlist.size() << "\n";
        current = rank_search::minimise_rank(field, current, shortlist);
        if (!verify(field, current, tensor.slices, "step 3")) return 1;
        if (as_json) std::cout << ",";
        report("step 3", exact::multiplication_count(field, current), current.size(),
               seconds_since(started), as_json);
    }

    if (as_json) std::cout << "]\n";
    return 0;
}
