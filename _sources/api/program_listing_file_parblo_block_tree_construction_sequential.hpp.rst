
.. _program_listing_file_parblo_block_tree_construction_sequential.hpp:

Program Listing for File sequential.hpp
=======================================

|exhale_lsh| :ref:`Return to documentation for file <file_parblo_block_tree_construction_sequential.hpp>` (``parblo/block_tree/construction/sequential.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   #pragma once
   
   #include <iostream>
   #include <unordered_map>
   #include <unordered_set>
   
   #include <pasta/bit_vector/bit_vector.hpp>
   #include <word_packing.hpp>
   
   #include <parblo/block_tree/block_tree.hpp>
   
   namespace parblo {
   
   namespace internal {
   
   struct Link {
       using IndexType = uint32_t;
       IndexType back_block_index;
       IndexType source_block_index;
       IndexType offset;
   
       constexpr inline explicit Link(IndexType back_index) :
           back_block_index{back_index},
           source_block_index{std::numeric_limits<IndexType>::max()},
           offset{0} {}
   
       [[nodiscard]] inline auto is_valid() const -> bool {
           return source_block_index != std::numeric_limits<IndexType>::max();
       }
   };
   
   } // namespace internal
   
   using pasta::BitVector;
   using PackedIntVector = word_packing::PackedIntVector<size_t>;
   
   using RabinKarpSet = std::unordered_set<HashedSlice>;
   template<typename V>
   using RabinKarpMap = std::unordered_map<HashedSlice, V>;
   template<typename V>
   using RabinKarpMultiMap = std::unordered_multimap<HashedSlice, V>;
   
   using MarkingAccessor = word_packing::internal::PackedFixedWidthIntAccessor<2>;
   
   struct Sequential {
       static void construct(BlockTree *bt, const std::string &s) {
           const size_t    num_blocks = bt->m_level_block_count[0];
           const size_t    block_size = bt->m_level_block_sizes[0];
           BitVector       is_adjacent(num_blocks - 1, true);
           PackedIntVector block_starts(num_blocks, s.length());
           for (int i = 0; i < num_blocks; ++i) {
               block_starts[i] = block_size * i;
           }
           scan_block_pairs(bt, s, 0, is_adjacent, block_starts);
           std::cout << *bt->m_is_internal[0] << std::endl;
           scan_blocks(bt, s, 0, is_adjacent, block_starts);
       }
   
       static void scan_block_pairs(BlockTree         *bt,
                                    const std::string &s,
                                    const size_t       level,
                                    BitVector         &is_adjacent,
                                    PackedIntVector   &block_starts) {
           const size_t block_size = bt->m_level_block_sizes[level];
           const size_t num_blocks = bt->m_level_block_count[level];
           const size_t pair_size  = 2 * block_size;
   
           // A map containing hashed slices mapped to their index of the pair's first block
           RabinKarpMap<int> map(num_blocks - 1);
   
           // Set up the packed array holding the markings for each block.
           // If for some block pair we find an earlier occurrence, we increment the marking for both blocks.
           // In the end, the blocks with a marking of two (or one, if it is the first or last block) are replaced by back
           // blocks
           const size_t marking_buffer_size = word_packing::num_packs_required<size_t>(num_blocks - 1, 2);
           std::vector<size_t> marking_buffer(marking_buffer_size);
           marking_buffer.resize(marking_buffer_size);
           auto markings = word_packing::accessor<2>(marking_buffer.data());
   
           {
               RabinKarp rk(s.c_str(), 0, pair_size);
               for (int i = 0; i < num_blocks - 1; ++i) {
                   // If the next block is not adjacent, we must relocate the hasher to the next pair of adjacent blocks.
                   if (!is_adjacent[i]) {
                       // Find the next adjacent block
                       while (!is_adjacent[++i] && i < num_blocks - 1)
                           ;
                       rk = RabinKarp(s.c_str() + block_starts[i], 0, pair_size);
                       continue;
                   }
                   HashedSlice hash          = rk.hashed_slice();
                   auto [elem, was_inserted] = map.insert({hash, i});
                   // If the hash already exists, then it is an earlier occurrence of the pair we are hashing now.
                   if (!was_inserted) {
                       markings[i]     = markings[i] + 1;
                       markings[i + 1] = markings[i + 1] + 1;
                   }
                   rk.advance_n(block_size);
               }
           }
   
           // Hash every window and determine for all block pairs whether they have previous occurrences.
           RabinKarp rk(s.c_str(), s.length(), pair_size);
           for (int i = 0; i < num_blocks; ++i) {
               if (!is_adjacent[i]) {
                   continue;
               }
               scan_windows_in_block_pair(rk, map, markings, block_size);
           }
   
           bt->m_is_internal.push_back(std::make_unique<BitVector>(num_blocks));
           BitVector &is_internal      = *bt->m_is_internal.back();
           is_internal[0]              = true;
           is_internal[num_blocks - 1] = markings[num_blocks - 1] != 1;
           for (int i = 0; i < num_blocks; ++i) {
               is_internal[i] = markings[i] != 2;
           }
       }
   
       static inline void scan_windows_in_block_pair(RabinKarp         &rk,
                                                     RabinKarpMap<int> &map,
                                                     MarkingAccessor    markings,
                                                     const size_t       block_size) {
           for (int i = 0; i < block_size; ++i) {
               HashedSlice current_hash = rk.hashed_slice();
               // Find the hash of the current window among the hashed block pairs.
               auto found_hash_ptr = map.find(current_hash);
               if (found_hash_ptr == map.end()) {
                   continue;
               }
               // If there is a block pair with this hash, we check whether it is an earlier occurrence.
               // If so, we mark the respective blocks.
               auto &[block_pair_slice, block_index] = *found_hash_ptr;
               if (current_hash.bytes() < block_pair_slice.bytes()) {
                   markings[block_index]     = markings[block_index] + 1;
                   markings[block_index + 1] = markings[block_index + 1] + 1;
                   map.erase(found_hash_ptr);
               }
           }
       }
   
       static void scan_blocks(BlockTree         *bt,
                               const std::string &s,
                               const size_t       level,
                               const BitVector   &is_adjacent,
                               PackedIntVector   &block_starts) {
           const size_t     block_size  = bt->m_level_block_sizes[level];
           const size_t     num_blocks  = bt->m_level_block_count[level];
   
           // A map containing hashed slices mapped to their source block and the current block's id.
           RabinKarpMultiMap<internal::Link> map(num_blocks - 1);
           for (int i = 0; i < num_blocks; ++i) {
               const HashedSlice hash = RabinKarp(s.c_str(), 0, block_size).hashed_slice();
               map.insert({hash, internal::Link(i)});
           }
   
           // Hash every window and find the first occurrences for every block.
           RabinKarp rk(s.c_str(), s.length(), block_size);
           for (int current_block_index = 0; current_block_index < num_blocks; ++current_block_index) {
               // This is true iff there exists a next block and it is not adjacent
               const bool next_block_not_adjacent =
                   current_block_index < num_blocks - 1 && !is_adjacent[current_block_index];
               // If the next block is not adjacent, we just want to hash exactly this block.
               // If it either is adjacent or we are at the end of the string, we take care not to hash windows that start
               // beyond the end of the string
               const size_t num_hashes =
                   next_block_not_adjacent
                       ? 1
                       : block_size - sat_sub(block_starts[current_block_index] + block_size, s.length());
   
               scan_windows_in_block(rk, map, current_block_index, num_hashes);
   
               if (next_block_not_adjacent) {
                   rk = RabinKarp(s.c_str() + block_starts[current_block_index + 1], 0, block_size);
               }
           }
           for (const auto &[hash, entry] : map) {
               std::cout << entry.back_block_index << ": (" << entry.source_block_index << ", " << entry.offset << ")"
                         << std::endl;
           }
       }
   
       static inline void scan_windows_in_block(RabinKarp                         &rk,
                                                RabinKarpMultiMap<internal::Link> &map,
                                                const size_t                       current_block_index,
                                                const size_t                       num_hashes) {
           for (int offset = 0; offset < num_hashes; ++offset) {
               const HashedSlice current_hash = rk.hashed_slice();
               // Find all blocks in the multimap that match our hash
               const size_t bucket = map.bucket(current_hash);
               for (auto elem = map.begin(bucket); elem != map.end(bucket); ++elem) {
                   internal::Link &link        = elem->second;
                   const size_t    block_index = link.back_block_index;
                   // In this case, our current position is an earlier occurrence and has no other link set yet!
                   if (current_block_index < block_index && !link.is_valid()) {
                       link.source_block_index = current_block_index;
                       link.offset             = offset;
                   }
               }
           }
       }
   
       static inline size_t sat_sub(size_t x, size_t y) {
           size_t res = x - y;
           res &= -(res <= x);
           return res;
       }
   };
   
   } // namespace parblo
