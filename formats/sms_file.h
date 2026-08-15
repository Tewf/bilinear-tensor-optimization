#pragma once

#include <istream>
#include <ostream>
#include <string>

#include "field.h"

namespace linear_algebra {

/// SMS: the sparse format LinBox and Givaro read and write, and the one the
/// original spoke.
///
/// A header line `rows columns type`, then one `row column value` triple per
/// nonzero entry with **1-based** indices, terminated by `0 0 0`. The type is
/// `M` for integers and `R` for rationals; entries are read as fractions
/// either way, since an integer is one.
///
/// Worth having beyond fidelity to the original: it is how an operator gets
/// into and out of the rest of the exact-linear-algebra ecosystem.
RationalMatrix read_sms(std::istream& input);

RationalMatrix read_sms_file(const std::string& path);

/// `type` is `M` when every entry is an integer, `R` otherwise, chosen here
/// rather than asked of the caller.
void write_sms(std::ostream& output, const RationalMatrix& matrix);

}  // namespace linear_algebra
