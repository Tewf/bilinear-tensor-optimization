#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace exact {

/// A dense row-major matrix of finite field elements.
///
/// Small by construction: the bilinear maps searched over here are a few dozen
/// entries, so there is nothing to gain from a sparse representation and a
/// great deal to lose in legibility.
class Matrix {
public:
    Matrix() = default;
    Matrix(std::size_t rows, std::size_t columns)
        : rows_(rows), columns_(columns), entries_(rows * columns, 0) {}

    std::size_t rows() const { return rows_; }
    std::size_t columns() const { return columns_; }
    std::size_t entry_count() const { return entries_.size(); }

    int64_t& operator()(std::size_t row, std::size_t column) {
        return entries_[row * columns_ + column];
    }
    int64_t operator()(std::size_t row, std::size_t column) const {
        return entries_[row * columns_ + column];
    }

    /// FFPACK works on raw storage, and overwrites what it is given.
    int64_t* data() { return entries_.data(); }
    const int64_t* data() const { return entries_.data(); }

    bool is_zero() const {
        for (int64_t entry : entries_) {
            if (entry != 0) return false;
        }
        return true;
    }

private:
    std::size_t rows_ = 0;
    std::size_t columns_ = 0;
    std::vector<int64_t> entries_;
};

}  // namespace exact
