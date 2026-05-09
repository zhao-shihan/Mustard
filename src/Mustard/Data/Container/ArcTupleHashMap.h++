// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
//
// This file is part of Mustard, an offline software framework for HEP experiments.
//
// Mustard is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// Mustard is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// Mustard. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "Mustard/Data/Model.h++"
#include "Mustard/Data/Object/Tuple.h++"
#include "Mustard/Memory/Allocator.h++"

#include "gtl/phmap.hpp"

#include "muc/mutex"

#include <cstddef>
#include <utility>

namespace Mustard::Data::inline Container {

/// @brief Flat hash map from keys to ArcTuple values
/// @tparam AKey Key type
/// @tparam M Tuple model type
/// @tparam AHash Hash functor type
/// @tparam AEq Key equality functor type
/// @tparam AAlloc Allocator type for key-value pairs
template<typename AKey, Modelized M,
         typename AHash = gtl::Hash<AKey>,
         typename AEq = gtl::EqualTo<AKey>,
         typename AAlloc = Allocator<std::pair<const AKey, ArcTuple<M>>>>
class ArcTupleHashMap : public gtl::flat_hash_map<AKey, ArcTuple<M>, AHash, AEq, AAlloc> {
public:
    /// @brief Tuple model of mapped type
    using Model = M;

public:
    /// @brief Inherit all flat_hash_map constructors
    using gtl::flat_hash_map<AKey, ArcTuple<M>, AHash, AEq, AAlloc>::flat_hash_map;
    /// @brief Inherit all flat_hash_map assignment operators
    using gtl::flat_hash_map<AKey, ArcTuple<M>, AHash, AEq, AAlloc>::operator=;
};

/// @brief Parallel flat hash map from keys to ArcTuple values
/// @tparam AKey Key type
/// @tparam M Tuple model type
/// @tparam AHash Hash functor type
/// @tparam AEq Key equality functor type
/// @tparam AAlloc Allocator type for key-value pairs
/// @tparam N 2^N submaps used for sharding (e.g. 4 means 16 submaps)
/// @tparam AMutex Mutex type protecting each submap
/// @tparam AAuxCntr Auxiliary container type used by the map implementation
template<typename AKey, Modelized M,
         typename AHash = gtl::Hash<AKey>,
         typename AEq = gtl::EqualTo<AKey>,
         typename AAlloc = Allocator<std::pair<const AKey, ArcTuple<M>>>,
         std::size_t N = 4,
         typename AMutex = muc::spin_mutex,
         typename AAuxCntr = gtl::priv::empty>
class ArcTupleParHashMap : public gtl::parallel_flat_hash_map<AKey, ArcTuple<M>, AHash, AEq, AAlloc, N, AMutex, AAuxCntr> {
public:
    /// @brief Tuple model of mapped type
    using Model = M;

public:
    /// @brief Inherit all parallel_flat_hash_map constructors
    using gtl::parallel_flat_hash_map<AKey, ArcTuple<M>, AHash, AEq, AAlloc, N, AMutex, AAuxCntr>::parallel_flat_hash_map;
    /// @brief Inherit all parallel_flat_hash_map assignment operators
    using gtl::parallel_flat_hash_map<AKey, ArcTuple<M>, AHash, AEq, AAlloc, N, AMutex, AAuxCntr>::operator=;
};

/// @brief Convenience alias of ArcTupleParHashMap with default hash/equality/allocator
/// @tparam AKey Key type
/// @tparam M Tuple model type
/// @tparam N 2^N submaps used for sharding (e.g. 4 means 16 submaps)
/// @tparam AMutex Mutex type protecting each submap
/// @tparam AAuxCntr Auxiliary container type used by the map implementation
template<typename AKey, Modelized M,
         std::size_t N = 4,
         typename AMutex = muc::spin_mutex,
         typename AAuxCntr = gtl::priv::empty>
using ArcTupleParHashMap0 = ArcTupleParHashMap<AKey, M, gtl::Hash<AKey>, gtl::EqualTo<AKey>,
                                               Allocator<std::pair<const AKey, ArcTuple<M>>>, N, AMutex, AAuxCntr>;

} // namespace Mustard::Data::inline Container
