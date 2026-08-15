#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "linear_algebra.h"
#include "matrix.h"

namespace exact {

/// A basis of a set of matrices regarded as vectors, built one candidate at a
/// time and kept in reduced row echelon form.
///
/// The search asks "is this one new?" far more often than it asks anything
/// else, and the original answered it by computing two ranks from scratch every
/// time. Reducing the candidate against what is already held answers the same
/// question in a single pass.
class SpanBasis {
public:
    SpanBasis(const Field& field, std::size_t width) : field_(&field), width_(width) {}

    std::size_t dimension() const { return rows_.size(); }

    /// Reduce a flat vector against the rows held. What remains is zero exactly
    /// when the vector was already in the span.
    void reduce(std::vector<int64_t>& entries) const {
        for (std::size_t index = 0; index < rows_.size(); ++index) {
            const int64_t leading = entries[pivot_columns_[index]];
            if (field_->isZero(leading)) continue;
            int64_t factor;
            field_->neg(factor, leading);
            for (std::size_t column = 0; column < width_; ++column) {
                field_->axpyin(entries[column], factor, rows_[index][column]);
            }
        }
    }

    bool contains(const Matrix& candidate) const {
        std::vector<int64_t> entries = flatten(candidate);
        reduce(entries);
        for (int64_t entry : entries) {
            if (!field_->isZero(entry)) return false;
        }
        return true;
    }

    /// Add `candidate` if it is outside the span, and say whether it was.
    bool try_add(const Matrix& candidate) {
        std::vector<int64_t> entries = flatten(candidate);
        reduce(entries);

        std::size_t pivot = width_;
        for (std::size_t column = 0; column < width_; ++column) {
            if (!field_->isZero(entries[column])) {
                pivot = column;
                break;
            }
        }
        if (pivot == width_) return false;

        int64_t scale;
        field_->inv(scale, entries[pivot]);
        for (int64_t& entry : entries) field_->mulin(entry, scale);

        // Clear the new pivot out of the rows already held, so the whole set
        // stays reduced and `reduce` above does not depend on row order.
        for (std::vector<int64_t>& row : rows_) {
            const int64_t leading = row[pivot];
            if (field_->isZero(leading)) continue;
            int64_t factor;
            field_->neg(factor, leading);
            for (std::size_t column = 0; column < width_; ++column) {
                field_->axpyin(row[column], factor, entries[column]);
            }
        }

        rows_.push_back(std::move(entries));
        pivot_columns_.push_back(pivot);
        return true;
    }

private:
    static std::vector<int64_t> flatten(const Matrix& matrix) {
        return std::vector<int64_t>(matrix.data(), matrix.data() + matrix.entry_count());
    }

    const Field* field_;
    std::size_t width_;
    std::vector<std::vector<int64_t>> rows_;
    std::vector<std::size_t> pivot_columns_;
};

}  // namespace exact
