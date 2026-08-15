#pragma once

#include "field.h"

/// Matrix sparsification: given the operator `U` of a fast multiplication
/// algorithm, find an invertible `V` minimising `nnz(U V)`.
///
/// Fewer nonzeros in the operator means fewer additions in the algorithm it
/// encodes, which is the cost the multiplication count does not capture. The
/// filenames say whose each method is:
///
/// - [`heuristic_sparsifier.h`](heuristic_sparsifier.h): the row-basis
///   construction, Mohamed's, guaranteeing nothing about the minimum.
/// - [`oracle_sparsifier.h`](oracle_sparsifier.h): the two oracles of the
///   article, exact for the question they ask.
namespace matrix_sparsification {

/// Everything here works in exact rationals. The entries of a real operator are
/// fractions like 4/9, which no double holds, and the quantity being minimised
/// is how many of them are zero: a value that rounds to zero is not nearly
/// right, it is a different answer.
using Field = linear_algebra::RationalField;
using Matrix = linear_algebra::RationalMatrix;
using Element = Field::Element;

}  // namespace matrix_sparsification
