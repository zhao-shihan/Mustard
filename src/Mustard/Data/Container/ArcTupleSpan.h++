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

#include <cstddef>
#include <span>

namespace Mustard::Data::inline Container {

/// @brief Non-owning span container of ArcTuple elements
/// @tparam M Tuple model type
/// @tparam AExtent Extent of the span
template<Modelized M,
         std::size_t AExtent = std::dynamic_extent>
class ArcTupleSpan : public std::span<ArcTuple<M>, AExtent> {
public:
    /// @brief Tuple model of value type
    using Model = M;

public:
    /// @brief Inherit all span constructors
    using std::span<ArcTuple<M>, AExtent>::span;
    /// @brief Inherit all span assignment operators
    using std::span<ArcTuple<M>, AExtent>::operator=;
};

} // namespace Mustard::Data::inline Container
