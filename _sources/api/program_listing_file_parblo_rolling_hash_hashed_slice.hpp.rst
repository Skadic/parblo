
.. _program_listing_file_parblo_rolling_hash_hashed_slice.hpp:

Program Listing for File hashed_slice.hpp
=========================================

|exhale_lsh| :ref:`Return to documentation for file <file_parblo_rolling_hash_hashed_slice.hpp>` (``parblo/rolling_hash/hashed_slice.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   #pragma once
   
   #include <cstdint>
   #include <string>
   #include <string_view>
   
   namespace parblo {
   
   class HashedSlice {
       const uint8_t *m_bytes;
       size_t m_length;
       uint64_t m_hash;
   
     public:
       inline HashedSlice(const uint8_t *bytes, size_t len, uint64_t hash) : m_bytes{bytes}, m_length{len}, m_hash{hash} {}
   
       inline HashedSlice(const char *bytes, size_t len, uint64_t hash) :
           HashedSlice(reinterpret_cast<const uint8_t *>(bytes), len, hash) {}
   
       inline HashedSlice(const std::string &s, size_t start, size_t end, uint64_t hash) :
           HashedSlice(s.c_str(), end - start, hash) {}
   
       inline HashedSlice(const std::string_view &view, uint64_t hash) : HashedSlice(view.cbegin(), view.length(), hash) {}
   
       template<std::input_iterator It>
       inline HashedSlice(It start, It end, uint64_t hash) :
           HashedSlice(reinterpret_cast<const char *>(start), std::distance(start, end), hash) {}
   
       inline HashedSlice(uint64_t hash) : m_bytes{nullptr}, m_length{0}, m_hash{hash} {}
   
       [[nodiscard]] inline auto operator[](const size_t i) const -> const uint8_t { return *(m_bytes + i); }
   
       [[nodiscard]] inline auto bytes() const -> const uint8_t * { return m_bytes; }
   
       [[nodiscard]] inline auto length() const -> size_t { return m_length; }
   
       [[nodiscard]] inline auto hash() const -> uint64_t { return m_hash; }
   
       inline auto operator==(const HashedSlice &other) const -> bool { return m_hash == other.m_hash; }
   };
   
   } // namespace parblo
   
   namespace std {
   template<>
   struct hash<parblo::HashedSlice> {
       inline std::size_t operator()(const parblo::HashedSlice &slice) const { return slice.hash(); }
   };
   } // namespace std
