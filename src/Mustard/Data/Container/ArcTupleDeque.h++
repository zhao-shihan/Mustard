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

#include "Mustard/Data/Model.h++"
#include "Mustard/Data/Object/Tuple.h++"
#include "Mustard/Memory/Allocator.h++"

#include <deque>

namespace Mustard::Data::inline Container {

/// @brief Double-ended queue container of ArcTuple elements
/// @tparam M Tuple model type
/// @tparam AAlloc Allocator type used by the underlying deque
template<Modelized M,
         typename AAlloc = Allocator<ArcTuple<M>>>
class ArcTupleDeque : public std::deque<ArcTuple<M>, AAlloc> {
public:
    /// @brief Tuple model of value type
    using Model = M;

public:
    /// @brief Inherit all deque constructors
    using std::deque<ArcTuple<M>, AAlloc>::deque;
    /// @brief Inherit all deque assignment operators
    using std::deque<ArcTuple<M>, AAlloc>::operator=;
};

} // namespace Mustard::Data::inline Container
