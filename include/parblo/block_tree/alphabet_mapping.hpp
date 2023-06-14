#pragma once

#include <array>
#include <iterator>

#include <pasta/bit_vector/bit_vector.hpp>

namespace parblo {

/// \brief This class maps an extended ascii alphabet to a contiguous sequence of numbers starting at zero.
///
/// For example, given an alphabet `a, c, d, f, g`, we create a mapping for the alphabet `\0, a, c, d, f, g` which
/// normally would correspond to extended ascii codes `0, 97, 99, 100, 102, 103`. Note, that the null byte is always
/// added to the alphabet even if it does not exist. This alphabet will be mapped to `0, 1, 2, 3, 4, 5`.
class AlphabetMapping {

    /// \brief Maps a character code to its original extended ascii representation.
    std::array<uint8_t, 256> m_to_ascii;
    /// \brief Maps a character's extended ascii code to its code.
    std::array<uint8_t, 256> m_to_code;
    /// \brief Number of characters in the alphabet
    size_t m_alphabet_size;

  public:
    /// \brief Creates a new alphabet mapping from a text or alphabet.
    ///
    /// \tparam Range A range which contains values that can be cast to `uint8_t`. This can be e.g. an std::string.
    /// \param input The range from which to construct the mapping.
    template<std::ranges::input_range Range>
    explicit AlphabetMapping(Range input) : m_to_ascii{}, m_to_code{}, m_alphabet_size{0} {
        pasta::BitVector exists(256, false);
        exists[0] = true;
        for (auto c : input) {
            exists[static_cast<uint8_t>(c)] = true;
        }
        for (uint16_t c = 0; c < 256; ++c) {
            if (exists[c]) {
                m_to_code[c]                = m_alphabet_size;
                m_to_ascii[m_alphabet_size] = c;
                m_alphabet_size++;
            }
        }
    }

    /// \brief Maps a character code to its original extended ascii representation.
    ///
    /// \param code The code for a character.
    /// \return The character's extended ascii code.
    [[nodiscard]] inline auto to_ascii(uint8_t code) const -> uint8_t { return m_to_ascii[code]; }

    /// \brief Maps a character's extended ascii code to its code.
    ///
    /// \param ext_ascii The character's extended ascii code.
    /// \return The character code.
    [[nodiscard]] inline auto to_code(uint8_t ext_ascii) const -> uint8_t { return m_to_code[ext_ascii]; }

    /// \brief Returns the number of characters in the alphabet.
    /// \return The number of characters in the alphabet.
    [[nodiscard]] inline auto size() const -> size_t { return m_alphabet_size; }
};

} // namespace parblo