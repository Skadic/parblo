
.. _program_listing_file_parblo_block_tree_alphabet_mapping.hpp:

Program Listing for File alphabet_mapping.hpp
=============================================

|exhale_lsh| :ref:`Return to documentation for file <file_parblo_block_tree_alphabet_mapping.hpp>` (``parblo/block_tree/alphabet_mapping.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   #pragma once
   
   #include <array>
   #include <iterator>
   
   #include <pasta/bit_vector/bit_vector.hpp>
   
   namespace parblo {
   
   class AlphabetMapping {
   
       std::array<uint8_t, 256> m_to_ascii;
       std::array<uint8_t, 256> m_to_code;
       size_t m_alphabet_size;
   
     public:
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
   
       [[nodiscard]] inline auto to_ascii(uint8_t code) const -> uint8_t { return m_to_ascii[code]; }
   
       [[nodiscard]] inline auto to_code(uint8_t ext_ascii) const -> uint8_t { return m_to_code[ext_ascii]; }
   
       [[nodiscard]] inline auto size() const -> size_t { return m_alphabet_size; }
   };
   
   } // namespace parblo
