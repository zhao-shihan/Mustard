// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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

#include <version>

#if __cpp_lib_flat_map >= 202207L

#    include "Mustard/Data/Container/ArcTupleVector.h++"
#    include "Mustard/Data/Model.h++"
#    include "Mustard/Data/Object/Tuple.h++"

#    include <flat_map>
#    include <functional>
#    include <memory>
#    include <vector>

namespace Mustard::Data::inline Container {

/// @brief Flat map from keys to ArcTuple values
/// @tparam AKey Key type
/// @tparam M Tuple model type
/// @tparam AComp Comparison function type
/// @tparam AKeyCntr Key container type
/// @tparam AMappedCntr Mapped container type
template<typename AKey, Modelized M,
         typename AComp = std::less<AKey>,
         typename AKeyCntr = std::vector<AKey, typename std::allocator_traits<typename ArcTupleVector<M>::allocator_type>::rebind_alloc<AKey>>,
         typename AMappedCntr = ArcTupleVector<M>>
class ArcTupleFlatMap : public std::flat_map<AKey, ArcTuple<M>, AComp, AKeyCntr, AMappedCntr> {
public:
    /// @brief Tuple model of value type
    using Model = M;

public:
    /// @brief Inherit all flat_map constructors
    using std::flat_map<AKey, ArcTuple<M>, AComp, AKeyCntr, AMappedCntr>::flat_map;
    /// @brief Inherit all flat_map assignment operators
    using std::flat_map<AKey, ArcTuple<M>, AComp, AKeyCntr, AMappedCntr>::operator=;
};

template<typename AKey, Modelized M,
         typename AComp = std::less<AKey>,
         typename AKeyCntr = std::vector<AKey, typename std::allocator_traits<typename ArcTupleVector<M>::allocator_type>::rebind_alloc<AKey>>,
         typename AMappedCntr = ArcTupleVector<M>>
class ArcTupleFlatMultiMap : public std::flat_multimap<AKey, ArcTuple<M>, AComp, AKeyCntr, AMappedCntr> {
public:
    /// @brief Tuple model of value type
    using Model = M;

public:
    /// @brief Inherit all flat_multimap constructors
    using std::flat_multimap<AKey, ArcTuple<M>, AComp, AKeyCntr, AMappedCntr>::flat_multimap;
    /// @brief Inherit all flat_multimap assignment operators
    using std::flat_multimap<AKey, ArcTuple<M>, AComp, AKeyCntr, AMappedCntr>::operator=;
};

} // namespace Mustard::Data::inline Container

#endif
