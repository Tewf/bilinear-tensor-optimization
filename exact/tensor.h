#pragma once

#include <cstddef>
#include <istream>
#include <string>
#include <vector>

#include "fields.h"

namespace exact {

/// A bilinear map, as the list of matrices of the forms producing each output
/// coordinate. Slice i of the polynomial multiplication tensor is the form
/// producing coefficient c_i.
struct Tensor {
    int64_t characteristic = 0;
    std::vector<ModularMatrix> slices;

    std::size_t rows() const { return slices.empty() ? 0 : slices.front().rows(); }
    std::size_t columns() const { return slices.empty() ? 0 : slices.front().columns(); }
};

/// Read the fixture format: `field p`, `shape slices rows columns`, then the
/// slices as dense rows. Blank lines and `#` comments are ignored.
///
/// Throws std::runtime_error on anything it does not understand, rather than
/// returning a half-built tensor for a caller to misread.
Tensor read_tensor(std::istream& input);

Tensor read_tensor_file(const std::string& path);

}  // namespace exact
