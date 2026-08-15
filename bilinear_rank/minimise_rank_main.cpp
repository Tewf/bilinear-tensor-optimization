/// Run the heuristic on a bilinear map and report what each step cost.
///
/// The original's entry point read the map from an interactive prompt and
/// evaluated the reply as source code. This takes a file and arguments, so a
/// run can be scripted, timed, and repeated.
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include "algorithm_recovery.h"
#include "dense_matrix_file.h"
#include "heuristic_search.h"
#include "tensor_file.h"

namespace {

double seconds_since(std::chrono::steady_clock::time_point started) {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
}

/// Checked after every step, not only in the tests: the result has to still
/// generate the map it came from, or the number means nothing.
bool verify(const linear_algebra::ModularField& field, const std::vector<linear_algebra::ModularMatrix>& current,
            const std::vector<linear_algebra::ModularMatrix>& original, const std::string& step) {
    if (linear_algebra::spans_all(field, current, original)) return true;
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
        std::cerr << "usage: minimise-rank <tensor-file> [--steps 1|2|3] [--json]"
                     " [--emit-operators <prefix>]\n";
        return 2;
    }

    std::string path = argv[1];
    int wanted_steps = 3;
    bool as_json = false;
    std::string operator_prefix;
    for (int argument = 2; argument < argc; ++argument) {
        const std::string option = argv[argument];
        if (option == "--json") {
            as_json = true;
        } else if (option == "--steps" && argument + 1 < argc) {
            wanted_steps = std::stoi(argv[++argument]);
        } else if (option == "--emit-operators" && argument + 1 < argc) {
            operator_prefix = argv[++argument];
        } else {
            std::cerr << "unrecognised option: " << option << "\n";
            return 2;
        }
    }

    const linear_algebra::Tensor tensor = linear_algebra::read_tensor_file(path);
    const linear_algebra::ModularField field(tensor.characteristic);
    const auto started = std::chrono::steady_clock::now();

    std::cout << (as_json ? "[\n" : path + "\n");
    report("naive", linear_algebra::multiplication_count(field, tensor.slices), tensor.slices.size(), 0.0,
           as_json);

    std::vector<linear_algebra::ModularMatrix> current = bilinear_rank::smallest_basis(field, tensor.slices);
    if (!verify(field, current, tensor.slices, "step 1")) return 1;
    if (as_json) std::cout << ",";
    report("step 1", linear_algebra::multiplication_count(field, current), current.size(),
           seconds_since(started), as_json);

    if (wanted_steps >= 2) {
        const std::vector<linear_algebra::ModularMatrix> own = bilinear_rank::rank_one_candidates(field, current);
        const std::vector<linear_algebra::ModularMatrix> shortlist =
            bilinear_rank::improving_candidates(field, current, own);
        current = bilinear_rank::minimise_rank(field, current, shortlist);
        if (!verify(field, current, tensor.slices, "step 2")) return 1;
        if (as_json) std::cout << ",";
        report("step 2", linear_algebra::multiplication_count(field, current), current.size(),
               seconds_since(started), as_json);
    }

    if (wanted_steps >= 3) {
        const std::vector<linear_algebra::ModularMatrix> everything =
            bilinear_rank::all_rank_one_maps(field, tensor.rows(), tensor.columns());
        std::cerr << "step 3 pool: " << everything.size() << " rank-one maps\n";
        const std::vector<linear_algebra::ModularMatrix> shortlist =
            bilinear_rank::improving_candidates(field, current, everything);
        std::cerr << "step 3 shortlist: " << shortlist.size() << "\n";
        current = bilinear_rank::minimise_rank(field, current, shortlist);
        if (!verify(field, current, tensor.slices, "step 3")) return 1;
        if (as_json) std::cout << ",";
        report("step 3", linear_algebra::multiplication_count(field, current), current.size(),
               seconds_since(started), as_json);
    }

    // The multiplication count is not the deliverable: the algorithm is.
    // Recovering it also emits the operators the sparsification then works on,
    // which is what joins the two halves of this repository into one pipeline.
    const std::vector<linear_algebra::ModularMatrix> products =
        bilinear_rank::rank_one_candidates(field, current);
    bilinear_rank::Algorithm algorithm;
    if (!bilinear_rank::recover_algorithm(field, tensor.slices, products, algorithm)) {
        std::cerr << "FAILED: the decomposition did not turn back into an algorithm\n";
        return 1;
    }
    if (!linear_algebra::spans_all(field, bilinear_rank::computed_map(field, algorithm),
                                   tensor.slices)) {
        std::cerr << "FAILED: the recovered algorithm computes a different map\n";
        return 1;
    }
    std::cerr << "algorithm: " << algorithm.product_count() << " products, L is "
              << algorithm.left.rows() << "x" << algorithm.left.columns() << ", R is "
              << algorithm.right.rows() << "x" << algorithm.right.columns() << ", P is "
              << algorithm.decode.rows() << "x" << algorithm.decode.columns() << "\n";

    if (!operator_prefix.empty()) {
        const std::string origin = "Encoding operator recovered from " + path +
                                   "\nby minimise-rank, " +
                                   std::to_string(algorithm.product_count()) + " products.";
        linear_algebra::write_matrix_file(operator_prefix + "_left.matrix",
                                          origin + " Left operand.", algorithm.left);
        linear_algebra::write_matrix_file(operator_prefix + "_right.matrix",
                                          origin + " Right operand.", algorithm.right);
        std::cerr << "wrote " << operator_prefix << "_left.matrix and _right.matrix\n";
    }

    if (as_json) std::cout << "]\n";
    return 0;
}
