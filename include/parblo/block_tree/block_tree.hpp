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

/// Concept representing a block tree construction algorithm.
/// Any type that
/// \tparam Algo The algorithm type.
template<typename Algo>
concept BlockTreeConstruction = requires(Algo algo, BlockTree *bt, const std::string &s) {
    { algo.construct(bt, s) };
};

using namespace internal;

class BlockTree {
    friend Sequential;

    /// \brief For each level, this vector contains a bit vector with rank support.
    /// A bit vector has a bit for each block on the level. If the bit is `1`, it is internal. If it is `0`, it is a
    /// back block.
    std::vector<std::unique_ptr<BitVector>> m_is_internal;

    /// \brief Rank data structures for \ref m_is_internal.
    std::vector<Rank> m_is_internal_rank;

    /// \brief For each level, this vector contains a packed int vector.
    /// A packed vector contains an entry for each back block on the level and is the index of its source block.
    /// Since the source block must always be an internal block, the index is only with respect to internal blocks.
    /// That is, if the block's source is the `i`'th (starting at zero) *internal* block on the level, then `i` is
    /// saved.
    std::vector<PackedIntVector> m_source_blocks;

    /// \brief For each level, this vector contains a packed int vector.
    /// A packed vector contains an entry for each back block on the level and is the offset inside its source block
    /// from which to copy. That is, if the source starts at the `k`'th (starting at zero) character in the source
    /// block, we save `k`.
    std::vector<PackedIntVector> m_offsets;

    /// \brief The number of characters in the input string.
    const size_t m_input_length;
    /// \brief The number of characters stored in a leaf
    const size_t m_leaf_length;
    /// \brief The arity for each internal node except for the root.
    const size_t m_arity;
    /// \brief The arity of the root node.
    size_t m_root_arity;

    /// \brief Sizes of a block for each level. Index 0 = most shallow level
    std::vector<size_t> m_level_block_sizes;

    /// \brief Maps the alphabet to a smaller value range.
    AlphabetMapping m_alphabet;

    /// \brief The concatenation of the content of all leaf blocks.
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

        //!("sizes: {:?}", self.level_block_sizes);
        // println!("count: {:?}", self.level_block_count);
    }

  public:
    /// \brief Constructs a block tree with the given parameters and construction algorithm.
    ///
    /// \tparam ConstrAlgo The construction algorithm to be used. This must be a class with a public method
    /// `construct(parblo::BlockTree *bt, const std::string &s)` that will construct the block tree. \param input The
    /// input string. \param arity The arity of each block, save for the root node. \param leaf_length The number of
    /// characters each leaf will store. These characters will be stored verbatim. \param algo An instance of the
    /// construction algorithm that should be used.
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

    /// \brief Returns the height of the block tree, which is the number of levels.
    /// \return The number of levels in the tree.
    [[nodiscard]] inline auto height() const -> size_t { return m_level_block_sizes.size(); }

    /// \brief Returns the leaf string.
    /// \return The leaf string.
    [[nodiscard]] inline auto leaf_string() const -> std::string {
        std::stringstream ss;
        for (uint8_t c : m_leaf_string) {
            ss << (char) m_alphabet.to_ascii(c);
        }
        return ss.str();
    }

    /// \brief Returns the length of the input string.
    /// \return The number of characters in the input string.
    [[nodiscard]] inline auto input_length() const -> size_t { return m_input_length; }

    /// \brief Calculates the space consumption of this block tree in working memory.
    /// \return The approximate number of bytes this block tree occupies.
    [[nodiscard("space consumption calculated but unused")]] inline auto space_consumption() const -> size_t {
        size_t is_internal_size = 0;
        for (const auto &bitvec : m_is_internal) {
            is_internal_size += bitvec->data().size() * sizeof(BitVector::RawDataType);
        }
        std::cout << "is_internal: " << is_internal_size << std::endl;
        size_t is_internal_rank_size = 0;
        for (const auto &rank_ds : m_is_internal_rank) {
            is_internal_rank_size += rank_ds.space_usage();
        }
        std::cout << "is_internal_rank: " << is_internal_rank_size << std::endl;
        size_t source_blocks_size = 0;
        for (const auto &vec : m_source_blocks) {
            source_blocks_size += vec.size() * vec.width() / 8;
        }
        std::cout << "source_blocks: " << source_blocks_size << std::endl;
        size_t offsets_size = 0;
        for (const auto &vec : m_offsets) {
            offsets_size += vec.size() * vec.width() / 8;
        }
        std::cout << "offsets_size: " << offsets_size << std::endl;

        size_t level_block_sizes_size = m_level_block_sizes.size() * sizeof(decltype(m_level_block_sizes)::value_type);
        size_t alphabet_size          = sizeof(m_alphabet);
        size_t leaf_string_size       = m_leaf_string.size() * m_leaf_string.width() / 8;

        std::cout << "level_block_sizes: " << level_block_sizes_size << std::endl;
        std::cout << "alphabet_size: " << alphabet_size << std::endl;
        std::cout << "leaf_string_size: " << leaf_string_size << std::endl;

        return is_internal_size + is_internal_rank_size + source_blocks_size + source_blocks_size + offsets_size +
               level_block_sizes_size + alphabet_size + leaf_string_size;
    }
};

} // namespace parblo
