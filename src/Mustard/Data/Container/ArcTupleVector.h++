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

#include <vector>

namespace Mustard::Data::inline Container {

/// @brief Sequential vector container of ArcTuple elements
/// @tparam M Tuple model type
/// @tparam AAlloc Allocator type used by the underlying vector
template<Modelized M,
         typename AAlloc = Allocator<ArcTuple<M>>>
class ArcTupleVector : public std::vector<ArcTuple<M>, AAlloc> {
public:
    /// @brief Tuple model of value type
    using Model = M;

public:
    /// @brief Inherit all vector constructors
    using std::vector<ArcTuple<M>, AAlloc>::vector;
    /// @brief Inherit all vector assignment operators
    using std::vector<ArcTuple<M>, AAlloc>::operator=;
};

} // namespace Mustard::Data::inline Container
