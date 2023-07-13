
.. _program_listing_file_parblo_defs.hpp:

Program Listing for File defs.hpp
=================================

|exhale_lsh| :ref:`Return to documentation for file <file_parblo_defs.hpp>` (``parblo/defs.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   #pragma once
   
   #include <unordered_map>
   
   #include <pasta/bit_vector/bit_vector.hpp>
   #include <pasta/bit_vector/support/flat_rank.hpp>
   #include <word_packing.hpp>
   #include <ankerl/unordered_dense.h>
   
   #include <parblo/rolling_hash/hashed_slice.hpp>
   
   namespace parblo::internal {
       using BitVector = pasta::BitVector;
       using Rank = pasta::FlatRank<pasta::OptimizedFor::ZERO_QUERIES, BitVector>;
       using PackedIntVector = word_packing::PackedIntVector<size_t>;
   
       template<typename Key, typename Value, typename Hash=ankerl::unordered_dense::hash<Key>>
       using HashMap = ankerl::unordered_dense::map<Key, Value, Hash>;
       //using HashMap = std::unordered_map<Key, Value, Hash>;
       
       template<typename V>
       using RabinKarpMap = HashMap<HashedSlice, V, std::hash<HashedSlice>>;
   }
