#pragma once

#include <cstdint>
#include <cstddef>
#include <string>

#include <parblo/rolling_hash/hashed_slice.hpp>

namespace parblo {
/// @brief A Rabin Karp rolling hasher
class RabinKarp {

    /// @brief The base of the Rabin-Karp polynomial.
    static constexpr uint64_t BASE = 257;
    /// @brief A large prime number such that the hashes are members of the finite field of integers modulo
    /// RabinKarp#PRIME.
    static constexpr uint64_t PRIME = 8589935681;

    /// @brief A pointer into the string we are hashing windows of
    const uint8_t *m_source;
    /// @brief The number of characters from the positions of RabinKarp#m_source
    size_t m_string_len;
    /// @brief The current offset into the string. We are hashing s[offset..offset + window_size]
    size_t m_offset;
    /// @brief The size of the hashed window in bytes
    size_t m_window_size;
    /// @brief When we need to remove a char from the hash we would actually need to multiply it by BASE^k and
    /// then subtract it. However since our hash is in the finite field GF(PRIME), multiplying with the remainder is
    /// equivalent.
    uint64_t m_remainder;
    /// @brief The current hash value
    uint64_t m_hash;

  public:
    /// @brief Create a new Rabin-Karp hasher from a pointer, with a given string length and window size.
    /// @param source A pointer into the string to be hashed.
    /// @param string_len The number of characters from the given pointer until the end of the string.
    /// @param window_size The number of characters each rolling hash will be hashing.
    RabinKarp(const uint8_t *source, const size_t string_len, const size_t window_size) :
        m_source{source},
        m_string_len{string_len},
        m_offset{0},
        m_window_size{window_size} {
        // Create the initial hash value
        m_hash = 0;
        uint64_t c;
        for (size_t i = 0; i < window_size; i++) {
            c = static_cast<uint64_t>(source[i]);
            m_hash *= BASE;
            m_hash += c;
            m_hash %= PRIME;
        }

        // Create the remainder of BASE^(window_size) modulo PRIME
        m_remainder = 1;
        for (size_t i = 0; i < window_size - 1; i++) {
            m_remainder = (m_remainder * BASE) % PRIME;
        }
    }

    /// @brief Create a new Rabin-Karp hasher from a pointer, with a given string length and window size.
    /// @param source A pointer into the string to be hashed.
    /// @param string_len The number of characters from the given pointer until the end of the string.
    /// @param window_size The number of characters each rolling hash will be hashing.
    inline RabinKarp(const char *source, const size_t string_len, const size_t window_size) :
        RabinKarp(reinterpret_cast<const uint8_t *>(source), string_len, window_size) {}

    /// @brief Create a new Rabin-Karp hasher from a string, a start index and window size which will hash until the end
    /// of the string.
    /// @param source A string from which window will be hashed.
    /// @param start The start index from which the hasher will start hashing end of the string.
    /// @param window_size The number of characters each rolling hash will be hashing.
    inline RabinKarp(const std::string &source, const size_t start, const size_t window_size) :
        RabinKarp(source.c_str() + start, source.length() - start, window_size) {}

    /// \brief Copy constructor
    inline RabinKarp(const RabinKarp &other) = default;

    /// \brief Move constructor
    inline RabinKarp(RabinKarp &&other) = default;

    /// \brief Copy assignment operator
    inline auto operator=(const RabinKarp &other) -> RabinKarp & = default;

    /// \brief Move assignment operator
    inline auto operator=(RabinKarp &&other) -> RabinKarp & = default;

    /// @brief Returns the hash value for the current window.
    [[nodiscard]] inline auto hash() const -> uint64_t { return this->m_hash; }

    /// @brief Advances the hashed by one byte and recalculates the hash value.
    /// @return The hash value after advancing.
    auto advance() -> uint64_t {
        uint8_t outchar = m_source[m_offset];
        uint8_t inchar  = m_source[m_offset + m_window_size];

        m_hash += PRIME;
        m_hash -= (m_remainder * outchar) % PRIME;
        m_hash *= BASE;
        m_hash += inchar;
        m_hash %= PRIME;

        m_offset++;

        return m_hash;
    }

    /// @brief Advances the hashed by the given number of bytes and returns the hash value after.
    /// @param n The number of bytes to advance.
    /// @return The hash value after advancing.
    inline auto advance_n(size_t n) -> uint64_t {
        for (size_t i = 0; i < n; i++) {
            advance();
        }
        return hash();
    }

    /// @brief Returns the current state of the hasher in form of a HashedSlice object.
    ///
    /// The contents of the object will be the pointer to the start of the currently hashed part of the string, the
    /// length being the window size and the hash value being the current hash value of the Rabin-Karp hasher.
    ///
    /// @return A HashedSlice containing the current state of the hasher.
    [[nodiscard]] inline auto hashed_slice() const -> HashedSlice {
        return {m_source + m_offset, m_window_size, m_hash};
    }

    /// @brief Comparison operator comparing solely on the current hash and position in the string.
    /// @param other Another Rabin-Karp hasher.
    /// @return `true`, if the hash values and the current position are the same.
    inline auto operator==(const RabinKarp &other) const -> bool {
        return m_hash == other.m_hash && (m_source + m_offset) == (other.m_source + other.m_offset);
    }
};
} // namespace parblo
