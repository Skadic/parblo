#pragma once

#include <unordered_map>

#include <pasta/bit_vector/bit_vector.hpp>
#include <pasta/bit_vector/support/flat_rank.hpp>
#include <word_packing.hpp>
#include <ankerl/unordered_dense.h>

#include <parblo/rolling_hash/hashed_slice.hpp>

namespace parblo::internal {
    /// \brief An uncompressed bit vector.
    using BitVector = pasta::BitVector;
    /// \brief A rank data structure on a \ref BitVector.
    using Rank = pasta::FlatRank<pasta::OptimizedFor::ZERO_QUERIES, BitVector>;
    /// \brief A dynamically resizable vector for packed integers.
    using PackedIntVector = word_packing::PackedIntVector<size_t>;

    /** \brief The hash map type used.
     *  \tparam Key The type of keys stored in this map.
     *  \tparam Value The type of values stored in this map.
     *  \tparam Hash The hash used by this map. Defaults to `ankerl`'s hash function.
     */
    template<typename Key, typename Value, typename Hash=ankerl::unordered_dense::hash<Key>>
    using HashMap = ankerl::unordered_dense::map<Key, Value, Hash>;
    //using HashMap = std::unordered_map<Key, Value, Hash>;
    
    /** \brief \verbatim embed:rst
        A map with :cpp:class:`hashed slices of text<parblo::HashedSlice>` as keys.
        \endverbatim
     *  \tparam V The type of values stored in this map.
     */
    template<typename V>
    using RabinKarpMap = HashMap<HashedSlice, V, std::hash<HashedSlice>>;
}