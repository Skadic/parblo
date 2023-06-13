#pragma once

#include <iostream>
#include <unordered_map>

#include <pasta/bit_vector/bit_vector.hpp>
#include <pasta/bit_vector/support/flat_rank.hpp>
#include <word_packing.hpp>

#include <parblo/block_tree/block_tree.hpp>
#include <parblo/defs.hpp>

namespace parblo {

namespace internal {

//#define PARBLO_DEBUG_PRINTS

/// \brief Contains information about a link from a back-pointing block to its source block.
/// This consists of the block's index and the offset from which to copy.
struct Link {
    /// \brief The number type used by this class.
    using IndexType = uint32_t;
    /// \brief The index of the block from which the pointer goes *out* on the current level of the tree.
    const IndexType block_index;
    /// \brief The index of the source block *to* which this link points on the current level of the tree.
    IndexType source_block_index;
    /// \brief The offset from which to copy from inside the source block.
    IndexType offset;

    /// \brief Creates a new source representing an invalid/non-existent source.
    /// \param block_index The index of the block *from* which the link goes out, i.e. the index of the back block.
    ///  This is the normal index on the current level.
    constexpr inline explicit Link(IndexType block_index) :
        block_index{block_index},
        source_block_index{std::numeric_limits<IndexType>::max()},
        offset{0} {}

    /// \brief Determines whether this source represents a valid block.
    ///
    /// This merely checks whether the `source_block_index` is equal to the maximum of its numeric type.
    /// \return `true`, if this refers to a valid block, `false` otherwise.
    [[nodiscard]] inline auto is_valid() const -> bool {
        return source_block_index != std::numeric_limits<IndexType>::max();
    }
};

} // namespace internal

using namespace parblo::internal;

/// \brief A sequential construction algorithm for a BlockTree.
struct Sequential {

    /// Construct the block tree sequentially.
    /// \param bt The block tree to construct.
    /// \param s The string from which to construct the block tree.
    static void construct(BlockTree *bt, const std::string &s) {
        const size_t    num_blocks = bt->m_level_block_count[0];
        const size_t    block_size = bt->m_level_block_sizes[0];
        BitVector       is_adjacent(num_blocks - 1, true);
        PackedIntVector block_starts(num_blocks, static_cast<size_t>(ceil(log2(s.length()))));
        for (int i = 0; i < num_blocks; ++i) {
            block_starts[i] = block_size * i;
        }
        scan_block_pairs(bt, s, 0, is_adjacent, block_starts);
#ifdef PARBLO_DEBUG_PRINTS
        std::cout << "is_internal: " << *bt->m_is_internal[0] << std::endl;
#endif
        RabinKarpMultiMap<Link> links = scan_blocks(bt, s, 0, is_adjacent, block_starts);
    }

  private:
    using MarkingAccessor = word_packing::internal::PackedFixedWidthIntAccessor<2>;

    /// Scan through the blocks pairwise in order to identify which blocks should be replaced with back blocks.
    ///
    ///
    /// \param bt The block tree under construction.
    /// \param s The input string.
    /// \param level The current level.
    /// \param is_adjacent A bit vector detailing whether two blocks on the current level are adjacent or not.
    ///     If a `is_adjacent[i]` is one, then blocks i and i+1 are adjacent.
    /// \param block_starts A vector holding the start position in the text for each block.
    static void scan_block_pairs(BlockTree             *bt,
                                 const std::string     &s,
                                 const size_t           level,
                                 BitVector             &is_adjacent,
                                 const PackedIntVector &block_starts) {
        const size_t block_size = bt->m_level_block_sizes[level];
        const size_t num_blocks = bt->m_level_block_count[level];
        const size_t pair_size  = 2 * block_size;

        // A map containing hashed slices mapped to their index of the pair's first block
        RabinKarpMap<int> map(num_blocks - 1);

        // Set up the packed array holding the markings for each block.
        // If for some block pair we find an earlier occurrence, we increment the marking for both blocks.
        // In the end, the blocks with a marking of two (or one, if it is the first or last block) are replaced by back
        // blocks
        const size_t        marking_buffer_size = word_packing::num_packs_required<size_t>(num_blocks - 1, 2);
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
        bt->m_is_internal_rank.emplace_back(is_internal);

#ifdef PARBLO_DEBUG_PRINTS
        std::cout << "markings(" << num_blocks << "): ";
        for (size_t i = 0; i < num_blocks; ++i) {
            std::cout << markings[i] << ", ";
        }
        std::cout << std::endl;
#endif
    }

    /// \brief Scan through the windows starting in a block and mark them accordingly if they represent the earliest
    /// occurrence of some block hash.
    ///
    /// The supplied `RabinKarp` hasher must be at the start of the block.
    /// \param rk A Rabin-Karp hasher whose state is at the start of the block.
    /// \param map The map containing the hashes of block pairs.
    /// \param markings Contains the number of marks on each block.
    /// \param block_size The size of blocks on the current level.
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
            rk.advance();
        }
    }

    /// Scan through the blocks of the given level and determine for each block, if applicable, where it can copy its
    /// content from.
    /// \param bt The block tree under construction.
    /// \param s The input string.
    /// \param level The curret level.
    /// \param is_adjacent A bit vector detailing whether two blocks on the current level are adjacent or not.
    ///     If a `is_adjacent[i]` is one, then blocks i and i+1 are adjacent.
    /// \param block_starts A vector holding the start position in the text for each block.
    static auto scan_blocks(BlockTree         *bt,
                            const std::string &s,
                            const size_t       level,
                            const BitVector   &is_adjacent,
                            PackedIntVector   &block_starts) -> RabinKarpMultiMap<Link> {
        const size_t block_size = bt->m_level_block_sizes[level];
        const size_t num_blocks = bt->m_level_block_count[level];

        const Rank  &is_internal_rank    = bt->m_is_internal_rank[level];
        const size_t num_internal_blocks = is_internal_rank.rank1(num_blocks);
        const size_t num_back_blocks     = num_blocks - num_internal_blocks;

        // Create new vectors in m_source_blocks and m_offsets to hold values for this level.
        {
            const size_t block_size_bits     = static_cast<size_t>(ceil(log2(block_size)));
            const size_t internal_block_bits = static_cast<size_t>(ceil(log2(num_internal_blocks)));

            // Add new packed int-vectors
            bt->m_source_blocks.push_back(PackedIntVector(num_back_blocks, internal_block_bits));
            bt->m_offsets.push_back(PackedIntVector(num_back_blocks, block_size_bits));
        }

        PackedIntVector &source_blocks = bt->m_source_blocks.back();
        PackedIntVector &offsets       = bt->m_offsets.back();

        // A map containing hashed slices mapped to a link to their (potential) source block.
        RabinKarpMultiMap<Link> links(num_blocks - 1);
        for (int i = 0; i < num_blocks; ++i) {
            const HashedSlice hash = RabinKarp(s.c_str() + block_starts[i], 0, block_size).hashed_slice();
            links.insert({hash, Link(i)});
        }

        // Hash every window and find the first occurrences for every block.
        RabinKarp rk(s.c_str(), s.length(), block_size);
        for (int current_block_index = 0; current_block_index < num_blocks; ++current_block_index) {
            // TODO: We could skip this loop iteration if the current block is a back block
            //  Nothing is ever going to point to this anyway.
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

            scan_windows_in_block(rk,
                                  links,
                                  is_internal_rank.rank1(current_block_index),
                                  num_hashes,
                                  is_internal_rank,
                                  source_blocks,
                                  offsets);

            if (next_block_not_adjacent) {
                rk = RabinKarp(s.c_str() + block_starts[current_block_index + 1], 0, block_size);
            }
        }
#ifdef PARBLO_DEBUG_PRINTS
        for (const auto &[hash, entry] : links) {
            std::cout << entry.block_index << ": (" << entry.source_block_index << ", " << entry.offset << ")"
                      << std::endl;
        }
        std::cout << "sources(" << source_blocks.size() << "): ";
        for (const auto source : source_blocks) {
            std::cout << source << ", ";
        }
        std::cout << std::endl;

        std::cout << "offsets(" << offsets.size() << "): ";
        for (const auto offset : offsets) {
            std::cout << offset << ", ";
        }
        std::cout << std::endl;
#endif

        return links;
    }

    /// \brief Scans through block-sized windows starting inside one block and tries to find earlier occurrences of
    ///     blocks. Non-internal blocks will have their respective m_source_blocks and m_offsets entries populated.
    /// \param rk A Rabin-Karp hasher whose current state is at the start of a block.
    /// \param links A multimap whose keys are hashed blocks and the values are information about (potential)
    ///     earlier occurrences.
    /// \param current_block_internal_index The index of the block which the Rabin-Karp hasher is situated in.
    ///     The index should only be with respect to *internal blocks* on the current level, disregarding back blocks.
    /// \param num_hashes The number of times the Rabin-Karp hasher should advance.
    /// \param is_internal_rank A rank data structure on the m_is_internal bit vectors.
    ///     See the respective BlockTree attribute for more information.
    /// \param source_blocks For each back block on the current level stores the index
    ///     (with respect to back blocks only) of its source. This is populated by this function.
    /// \param offsets For each back block on the current level stores offset into its source block from which it will
    ///     copy its content. This is populated by this function.
    static inline void scan_windows_in_block(RabinKarp               &rk,
                                             RabinKarpMultiMap<Link> &links,
                                             const size_t             current_block_internal_index,
                                             const size_t             num_hashes,
                                             const Rank              &is_internal_rank,
                                             PackedIntVector         &source_blocks,
                                             PackedIntVector         &offsets) {
        for (int offset = 0; offset < num_hashes; ++offset) {
            const HashedSlice current_hash = rk.hashed_slice();
            // Find all blocks in the multimap that match our hash
            const auto &[start, end] = links.equal_range(current_hash);
            for (auto elem = start; elem != end; ++elem) {
                const HashedSlice & found_hash = elem->first;
                Link        &link        = elem->second;
                const size_t block_index = link.block_index;
                // In this case, our current position is an earlier occurrence and has no other link set yet!
                if (current_hash.bytes() < found_hash.bytes() && !link.is_valid()) {
                    // Get the index of the back block only considering back blocks
                    const size_t back_block_index   = is_internal_rank.rank0(link.block_index);
                    link.source_block_index         = current_block_internal_index;
                    link.offset                     = offset;
                    source_blocks[back_block_index] = current_block_internal_index;
                    offsets[back_block_index]       = offset;
                }
            }
            rk.advance();
        }
    }

    /// Branchless saturating subtraction.
    ///
    /// Source: http://locklessinc.com/articles/sat_arithmetic/
    ///
    /// \param x The minuend.
    /// \param y The subtrahend.
    /// \return x - y. However, if the result would underflow, returns zero.
    static inline size_t sat_sub(size_t x, size_t y) {
        size_t res = x - y;
        res &= -(res <= x);
        return res;
    }
};

#ifdef PARBLO_DEBUG_PRINTS
#undef PARBLO_DEBUG_PRINTS
#endif

} // namespace parblo