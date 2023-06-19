
.. _program_listing_file_parblo_rolling_hash_rabin_karp.hpp:

Program Listing for File rabin_karp.hpp
=======================================

|exhale_lsh| :ref:`Return to documentation for file <file_parblo_rolling_hash_rabin_karp.hpp>` (``parblo/rolling_hash/rabin_karp.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   #pragma once
   
   #include <cstddef>
   #include <cstdint>
   #include <string>
   
   #include <parblo/rolling_hash/hashed_slice.hpp>
   
   namespace parblo {
   class RabinKarp {
   
       static constexpr uint64_t BASE = 257;
       static constexpr uint64_t PRIME = 8589935681;
   
       const uint8_t *m_source;
       size_t m_string_len;
       size_t m_offset;
       size_t m_window_size;
       uint64_t m_remainder;
       uint64_t m_hash;
   
     public:
       static inline auto generate_remainder(const size_t window_size) -> uint64_t {
           uint64_t rem = 1;
           for (size_t i = 0; i < window_size - 1; i++) {
               rem = (rem * BASE) % PRIME;
           }
           return rem;
       }
   
       inline RabinKarp(const uint8_t *source, const size_t string_len, const size_t window_size, uint64_t remainder) :
           m_source{source},
           m_string_len{string_len},
           m_offset{0},
           m_window_size{window_size},
           m_remainder{remainder} {
           // Create the initial hash value
           m_hash = 0;
           uint64_t c;
           for (size_t i = 0; i < window_size; i++) {
               c = static_cast<uint64_t>(source[i]);
               m_hash *= BASE;
               m_hash += c;
               m_hash %= PRIME;
           }
       }
   
       inline RabinKarp(const uint8_t *source, const size_t string_len, const size_t window_size) :
           RabinKarp(source, string_len, window_size, generate_remainder(window_size)) {}
   
       inline RabinKarp(const char *source, const size_t string_len, const size_t window_size) :
           RabinKarp(reinterpret_cast<const uint8_t *>(source), string_len, window_size) {}
   
       inline RabinKarp(const std::string &source, const size_t start, const size_t window_size, const uint64_t remainder) :
           RabinKarp(reinterpret_cast<const uint8_t*>(source.c_str()), source.length(), window_size, remainder) {
           m_offset = start;
       }
   
       inline RabinKarp(const std::string &source, const size_t start, const size_t window_size) :
           RabinKarp(source.c_str(), source.length(), window_size) {
           m_offset = start;
       }
   
       inline RabinKarp(const RabinKarp &other) = default;
   
       inline RabinKarp(RabinKarp &&other) = default;
   
       inline auto operator=(const RabinKarp &other) -> RabinKarp & = default;
   
       inline auto operator=(RabinKarp &&other) -> RabinKarp & = default;
   
       [[nodiscard]] inline auto hash() const -> uint64_t { return this->m_hash; }
   
       inline auto advance() -> uint64_t {
           const uint8_t outchar = m_source[m_offset];
           const uint8_t inchar  = m_source[std::min(m_offset + m_window_size, m_string_len)];
   
           m_hash += PRIME;
           m_hash -= (m_remainder * outchar) % PRIME;
           m_hash *= BASE;
           m_hash += inchar;
           m_hash %= PRIME;
   
           m_offset++;
   
           return m_hash;
       }
   
       inline auto advance_n(size_t n) -> uint64_t {
           for (size_t i = 0; i < n; i++) {
               advance();
           }
           return hash();
       }
   
       [[nodiscard]] inline auto hashed_slice() const -> HashedSlice {
           return {m_source + m_offset, m_window_size, m_hash};
       }
   
       inline auto operator==(const RabinKarp &other) const -> bool {
           return m_hash == other.m_hash && (m_source + m_offset) == (other.m_source + other.m_offset);
       }
   };
   } // namespace parblo
