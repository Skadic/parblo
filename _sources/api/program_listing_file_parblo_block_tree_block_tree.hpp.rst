
.. _program_listing_file_parblo_block_tree_block_tree.hpp:

Program Listing for File block_tree.hpp
=======================================

|exhale_lsh| :ref:`Return to documentation for file <file_parblo_block_tree_block_tree.hpp>` (``parblo/block_tree/block_tree.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   #pragma once
   
   #include <cassert>
   #include <cmath>
   #include <memory>
   #include <sstream>
   #include <string>
   #include <vector>
   
   #include <pasta/bit_vector/bit_vector.hpp>
   #include <word_packing.hpp>
   
   #include <parblo/block_tree/alphabet_mapping.hpp>
   #include <parblo/defs.hpp>
   #include <parblo/rolling_hash/rabin_karp.hpp>
   #include <parblo/utils.hpp>
   
   namespace parblo {
   
   class BlockTree;
   struct Sequential;
   
   template<typename Algo>
   concept BlockTreeConstruction = requires(Algo algo, BlockTree *bt, const std::string &s) {
       { algo.construct(bt, s) };
   };
   
   using namespace internal;
   
   class BlockTree {
       friend Sequential;
   
       std::vector<std::unique_ptr<BitVector>> m_is_internal;
   
       std::vector<Rank> m_is_internal_rank;
   
       std::vector<PackedIntVector> m_source_blocks;
   
       std::vector<PackedIntVector> m_offsets;
   
       const size_t m_input_length;
       const size_t m_leaf_length;
       const size_t m_arity;
       size_t m_root_arity;
   
       std::vector<size_t> m_level_block_sizes;
   
       AlphabetMapping m_alphabet;
   
       PackedIntVector m_leaf_string;
   
       void calculate_level_block_sizes() {
           const auto num_levels   = static_cast<size_t>(std::ceil(
               std::log(static_cast<double>(m_input_length) / static_cast<double>(m_leaf_length)) / std::log(m_arity)));
           const auto float_length = static_cast<double>(m_input_length);
           m_level_block_sizes     = std::vector<size_t>();
           m_level_block_sizes.reserve(num_levels);
   
           size_t block_size = m_leaf_length;
   
           while (block_size < m_input_length) {
               m_level_block_sizes.push_back(block_size);
               block_size *= m_arity;
           }
           m_root_arity = static_cast<size_t>(std::ceil(float_length / static_cast<double>(block_size / m_arity)));
   
           std::reverse(m_level_block_sizes.begin(), m_level_block_sizes.end());
   
           // println!("count: {:?}", self.level_block_count);
       }
   
     public:
       template<BlockTreeConstruction ConstrAlgo>
       BlockTree(const std::string &input, const size_t arity, const size_t leaf_length, ConstrAlgo algo) :
           m_is_internal{},
           m_is_internal_rank{},
           m_source_blocks{},
           m_offsets{},
           m_input_length{input.length()},
           m_leaf_length{leaf_length},
           m_arity{arity},
           m_level_block_sizes{0},
           m_alphabet{input},
           m_leaf_string{0, internal::bit_size(m_alphabet.size())} {
           assert(input.length() > 0);
           assert(arity > 1);
           assert(leaf_length > 0);
           calculate_level_block_sizes();
           algo.construct(this, input);
       }
   
       [[nodiscard]] inline auto height() const -> size_t { return m_level_block_sizes.size(); }
   
       [[nodiscard]] inline auto leaf_string() const -> std::string {
           std::stringstream ss;
           for (uint8_t c : m_leaf_string) {
               ss << (char) m_alphabet.to_ascii(c);
           }
           return ss.str();
       }
   };
   
   } // namespace parblo
