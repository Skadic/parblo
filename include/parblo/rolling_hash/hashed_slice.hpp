#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace parblo {

/// @brief A hashed slice of a text.
class HashedSlice {
    /// @brief A pointer to the start of the hashed slice of the string.
    const uint8_t *m_bytes;
    /// @brief The length of the hashed slice in bytes.
    size_t m_length;
    /// @brief The hash value of this slice.
    uint64_t m_hash;

  public:
    /// @brief Constructs a new hashed slice from a raw pointer.
    /// @param bytes A pointer to the start of the hashed slice.
    /// @param len The length of the hashed slice in bytes.
    /// @param hash The hash value of this slice.
    inline HashedSlice(const uint8_t *bytes, size_t len, uint64_t hash) : m_bytes{bytes}, m_length{len}, m_hash{hash} {}

    /// @brief Constructs a new hashed slice from a raw pointer.
    /// @param bytes A pointer to the start of the hashed slice.
    /// @param len The length of the hashed slice in bytes.
    /// @param hash The hash value of this slice.
    inline HashedSlice(const char *bytes, size_t len, uint64_t hash) :
        HashedSlice(reinterpret_cast<const uint8_t *>(bytes), len, hash) {}

    /// @brief Constructs a new hashed slice from a string and a range.
    /// @param s The string in which the hashed range is found.
    /// @param start The inclusive start index of the hashed slice inside the string.
    /// @param end The exclusive end index of the hashed slice inside the string.
    /// @param hash The hash value of this slice.
    inline HashedSlice(const std::string &s, size_t start, size_t end, uint64_t hash) :
        HashedSlice(s.c_str(), end - start, hash) {}

    /// @brief Constructs a new hashed slice from a string view.
    /// @param view The string view that represents the hashed slice.
    /// @param hash The hash value of this slice.
    inline HashedSlice(const std::string_view &view, uint64_t hash) : HashedSlice(view.cbegin(), view.length(), hash) {}

    /// @brief Constructs a new hashed slice from a pair of iterators.
    /// @tparam It The iterator type.
    /// @param start The inclusive start iterator of the hashed slice inside the string. This must be able to be cast to
    /// a `const uint_8` pointer.
    /// @param end The exclusive end iterator of the hashed slice inside the string.
    /// @param hash The hash value of this slice.
    template<std::input_iterator It>
    inline HashedSlice(It start, It end, uint64_t hash) :
        HashedSlice(reinterpret_cast<const char *>(start), std::distance(start, end), hash) {}

    /// \brief Constructs a hashed slice exclusively for searching in hash tables.
    ///
    /// Since the hash implementation only cares about the m_hash field, we can use this to search in hash tables with
    /// HashedSlice as keys.
    /// \param hash The hash value of the slice you want to search for.
    inline HashedSlice(uint64_t hash) : m_bytes{nullptr}, m_length{0}, m_hash{hash} {}

    /// @brief Indexes into the hashed slice and returns the byte at the given index.
    /// @param i The index to retrieve.
    /// @return The byte at index `i`.
    [[nodiscard]] inline auto operator[](const size_t i) const -> const uint8_t { return *(m_bytes + i); }

    /// @brief Returns the pointer to the underlying slice.
    /// @return The pointer to the slice.
    [[nodiscard]] inline auto bytes() const -> const uint8_t * { return m_bytes; }

    /// @brief Returns the length of the hashed slice.
    /// @return The length of the hashed slice.
    [[nodiscard]] inline auto length() const -> size_t { return m_length; }

    /// @brief Returns the precomputed hash value.
    /// @return The precomputed hash value.
    [[nodiscard]] inline auto hash() const -> uint64_t { return m_hash; }

    /// @brief Compares two hashed slices based on their hash.
    /// @param other Another hashed slice.
    /// @return `true`, iff their hashes are equal.
    inline auto operator==(const HashedSlice &other) const -> bool { return m_hash == other.m_hash; }
};

} // namespace parblo

namespace std {
template<>
struct hash<parblo::HashedSlice> {
    inline std::size_t operator()(const parblo::HashedSlice &slice) const { return slice.hash(); }
};
} // namespace std
