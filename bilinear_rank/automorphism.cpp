#include "automorphism.h"

#include <string>
#include <unordered_set>

#include "matrix_ops.h"
#include "memory_budget.h"
#include "span_queries.h"

namespace bilinear_rank {

namespace {

/// The entries of both halves, read as bytes, which is what tells two
/// automorphisms apart when closing a group.
std::string fingerprint(const Automorphism& sigma) {
    std::string bytes;
    bytes.reserve((sigma.left.entry_count() + sigma.right.entry_count()) * sizeof(Element));
    for (const Matrix* half : {&sigma.left, &sigma.right}) {
        const char* start = reinterpret_cast<const char*>(half->data());
        bytes.append(start, half->entry_count() * sizeof(Element));
    }
    return bytes;
}

}  // namespace

Matrix act_on(const Field& field, const Automorphism& sigma, const Matrix& form) {
    const Matrix transposed = linear_algebra::transpose<Field>(sigma.left);
    return linear_algebra::multiply(field, linear_algebra::multiply(field, transposed, form),
                                    sigma.right);
}

Automorphism identity_automorphism(const Field& field, std::size_t rows, std::size_t columns) {
    Automorphism sigma{Matrix(rows, rows), Matrix(columns, columns)};
    for (std::size_t index = 0; index < rows; ++index) {
        field.assign(sigma.left(index, index), field.one);
    }
    for (std::size_t index = 0; index < columns; ++index) {
        field.assign(sigma.right(index, index), field.one);
    }
    return sigma;
}

Automorphism compose(const Field& field, const Automorphism& first, const Automorphism& second) {
    return {linear_algebra::multiply(field, first.left, second.left),
            linear_algebra::multiply(field, first.right, second.right)};
}

std::vector<Automorphism> group_closure(const Field& field,
                                        const std::vector<Automorphism>& generators) {
    if (generators.empty()) return {};

    std::vector<Automorphism> group{identity_automorphism(
        field, generators.front().left.rows(), generators.front().right.rows())};
    std::unordered_set<std::string> seen{fingerprint(group.front())};

    for (std::size_t frontier = 0; frontier < group.size(); ++frontier) {
        for (const Automorphism& generator : generators) {
            Automorphism reached = compose(field, group[frontier], generator);
            if (!seen.insert(fingerprint(reached)).second) continue;

            require_room("the group being closed", group.size() + 1,
                         bytes_per_matrix(reached.left.entry_count()) +
                             bytes_per_matrix(reached.right.entry_count()));
            group.push_back(std::move(reached));
        }
    }
    return group;
}

std::vector<Automorphism> stabiliser_of(const Field& field, const std::vector<Matrix>& slices,
                                        const std::vector<Automorphism>& group) {
    std::vector<Automorphism> stabiliser;
    for (const Automorphism& sigma : group) {
        std::vector<Matrix> moved;
        moved.reserve(slices.size());
        for (const Matrix& slice : slices) moved.push_back(act_on(field, sigma, slice));

        // Setwise: the image has to span what it came from, not to match slice
        // by slice. Both directions, since a map into a space of equal dimension
        // is onto it, and the dimensions are equal because the action is
        // invertible.
        if (linear_algebra::spans_all(field, moved, slices)) stabiliser.push_back(sigma);
    }
    return stabiliser;
}

}  // namespace bilinear_rank
