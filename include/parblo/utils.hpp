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

/// \brief Get the number of bits required to store the value.
///
/// \param v The value to store in bits. This must be greater than zero.
/// \return The number of bits required to store the value.
static inline constexpr auto bit_size(size_t v) -> size_t {
    return static_cast<size_t>(ceil(log2(static_cast<double>(v))));
}
} // namespace parblo::internal