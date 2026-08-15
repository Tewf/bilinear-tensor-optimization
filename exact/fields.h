#pragma once

#include <givaro/modular.h>
#include <givaro/qfield.h>

#include <cstdint>

#include "matrix.h"

/// Exact arithmetic, from Givaro. Both strands of this repository are searches
/// over ranks and over counts of nonzero entries, so an answer that is nearly
/// right is not a slightly worse answer, it is an answer to a different
/// question. Nothing here is ever a float.
namespace exact {

/// The finite-field side: the rank of a bilinear map over GF(p). The primes in
/// play are tiny, so a machine integer representation is ample.
using ModularField = Givaro::Modular<int64_t>;

/// The rational side: sparsifying the operators of a fast multiplication
/// algorithm, whose entries are fractions like 4/9. The original carried these
/// as doubles and then counted how many were zero.
using RationalField = Givaro::QField<Givaro::Rational>;

template <class Field>
using MatrixOver = Matrix<typename Field::Element>;

using ModularMatrix = MatrixOver<ModularField>;
using RationalMatrix = MatrixOver<RationalField>;

}  // namespace exact
