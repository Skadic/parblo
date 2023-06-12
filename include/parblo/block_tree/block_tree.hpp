#pragma once

#include <cassert>
#include <cmath>
#include <vector>
#include <string>
#include <memory>

#include <pasta/bit_vector/bit_vector.hpp>

#include <parblo/rolling_hash/rabin_karp.hpp>

namespace parblo {

class BlockTree;
struct Sequential;

/// Concept representing a block tree construction algorithm.
/// Any type that
/// \tparam Algo The algorithm type.
template<typename Algo>
concept BlockTreeConstruction = requires(Algo algo, BlockTree *bt, const std::string &s) {
    { algo.construct(bt, s) };
};

using pasta::BitVector;

class BlockTree {

    friend Sequential;

    /// @brief For each level, this vector contains a bit vector.
    /// A bit vector has a bit for each block on the level. If the bit is `1`, it is internal. If it is `0`, it is a
    /// back block.
    std::vector<std::unique_ptr<BitVector>> m_is_internal;

    /// @brief The number of characters in the input string.
    size_t m_input_length;
    /// @brief The number of characters stored in a leaf
    size_t m_leaf_length;
    /// @brief The arity for each internal node except for the root.
    size_t m_arity;

    /// @brief Sizes of a block for each level. Index 0 = most shallow level
    std::vector<size_t> m_level_block_sizes;
    /// @brief The number blocks for each level. Index 0 = most shallow level
    std::vector<size_t> m_level_block_count;

    void calculate_level_block_sizes() {
        const auto num_levels   = static_cast<size_t>(std::ceil(
            std::log(static_cast<double>(m_input_length) / static_cast<double>(m_leaf_length)) / std::log(m_arity)));
        const auto float_length = static_cast<double>(m_input_length);
        m_level_block_sizes     = std::vector<size_t>(num_levels);
        m_level_block_count     = std::vector<size_t>(num_levels);

        size_t block_size = m_leaf_length;

        while (block_size < m_input_length) {
            m_level_block_sizes.push_back(block_size);
            m_level_block_count.push_back(
                static_cast<size_t>(std::ceil(float_length / static_cast<double>(block_size))));
            block_size *= m_arity;
        }

        std::reverse(m_level_block_sizes.begin(), m_level_block_sizes.end());
        std::reverse(m_level_block_count.begin(), m_level_block_count.end());

        //!("sizes: {:?}", self.level_block_sizes);
        // println!("count: {:?}", self.level_block_count);
    }

  public:
    template<BlockTreeConstruction ConstrAlgo>
    BlockTree(const std::string &input, const size_t arity, const size_t leaf_length, ConstrAlgo algo) :
        m_input_length{input.length()},
        m_leaf_length{leaf_length},
        m_arity{arity},
        m_level_block_sizes{0},
        m_level_block_count{0} {
        assert(arity > 1);
        assert(leaf_length > 0);
        calculate_level_block_sizes();
        algo.construct(this, input);
    }

};

} // namespace parblo
