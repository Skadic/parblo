#pragma once
#include <cstddef>

namespace parblo::internal {

/// \brief Branch-less saturating subtraction.
///
/// Source: http://locklessinc.com/articles/sat_arithmetic/
///
/// \param x The minuend.
/// \param y The subtrahend.
/// \return x - y. However, if the result would underflow, returns zero.
static inline constexpr auto saturating_sub(size_t x, size_t y) -> size_t {
    size_t res = x - y;
    res &= -(res <= x);
    return res;
}

/// \brief Get the floored logarithm of x.
static inline constexpr auto floorlog2(size_t x) -> size_t { return x == 1 ? 0 : 1 + floorlog2(x >> 1); }

/// \brief Get the number of bits required to store the value.
///
/// Source:
/// https://stackoverflow.com/questions/23781506/compile-time-computing-of-number-of-bits-needed-to-encode-n-different-states
///
/// \param v The value to store in bits. This must be greater than zero.
/// \return The number of bits required to store the value.
static inline constexpr auto bit_size(size_t v) -> size_t { return v == 1 ? 0 : floorlog2(v - 1) + 1; }

} // namespace parblo::internal