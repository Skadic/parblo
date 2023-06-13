#pragma once

#include <unordered_map>

#include <pasta/bit_vector/bit_vector.hpp>
#include <pasta/bit_vector/support/flat_rank.hpp>
#include <word_packing.hpp>

#include <parblo/rolling_hash/hashed_slice.hpp>

namespace parblo::internal {
    /// \brief An uncompressed bit vector.
    using BitVector = pasta::BitVector;
    /// \brief A rank data structure on a \ref BitVector.
    using Rank = pasta::FlatRank<pasta::OptimizedFor::ONE_QUERIES, BitVector>;
    /// \brief A dynamically resizable vector for packed integers.
    using PackedIntVector = word_packing::PackedIntVector<size_t>;

    /** \brief \verbatim embed:rst
        A map with :cpp:class:`hashed slices of text<parblo::HashedSlice>` as keys.
        \endverbatim
     *  \tparam V The type of values stored in this map.
     */
    template<typename V>
    using RabinKarpMap = std::unordered_map<HashedSlice, V>;

    /** \brief \verbatim embed:rst
        A multimap with :cpp:class:`hashed slices of text<parblo::HashedSlice>` as keys.
        \endverbatim
     *  \tparam V The type of values stored in this map.
     */
    template<typename V>
    using RabinKarpMultiMap = std::unordered_multimap<HashedSlice, V>;
}