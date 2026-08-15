#pragma once

#include <istream>
#include <string>

#include "fields.h"

namespace exact {

/// Read a matrix of rationals: `shape rows columns`, then dense rows whose
/// entries are integers or fractions written `4/9`. Blank lines and `#`
/// comments are ignored.
///
/// Entries stay fractions all the way through. The operators of a fast
/// multiplication algorithm have entries like 4/9, which no double holds, and
/// the quantity being minimised is how many of them are zero.
RationalMatrix read_rational_matrix(std::istream& input);

RationalMatrix read_rational_matrix_file(const std::string& path);

/// Print a matrix with fractions written the way the fixtures write them.
std::string to_string(const RationalMatrix& matrix);

}  // namespace exact
