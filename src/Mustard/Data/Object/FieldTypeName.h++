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

#include "Mustard/Data/Object/Value.h++"
#include "Mustard/Data/Object/ValueAcceptable.h++"
#include "Mustard/gslx/index_sequence.h++"

#include "muc/utility"

#include "fmt/format.h"

#include <concepts>
#include <cstdint>
#include <string>
#include <utility>

namespace Mustard::Data::inline Object {

/// @brief Returns the compact field type name for a value-acceptable payload type.
/// @tparam T Payload type.
template<ValueAcceptable T>
auto FieldTypeName() -> std::string;

/// @brief Returns the compact field type name for a value-acceptable payload object.
/// @tparam T Payload type.
template<ValueAcceptable T>
auto FieldTypeName(const T&) -> auto { return FieldTypeName<T>(); }

/// @brief Returns the compact field type name for a Field.
/// @tparam T Wrapper type whose nested payload type is used for the name.
template<typename T>
    requires impl2::IsValue<T>::value
auto FieldTypeName() -> auto { return FieldTypeName<typename T::Type>(); }

/// @brief Returns the compact field type name for a Field.
/// @tparam T Wrapper type whose nested payload type is used for the name.
template<typename T>
    requires impl2::IsValue<T>::value
auto FieldTypeName(const T&) -> auto { return FieldTypeName<T>(); }

} // namespace Mustard::Data::inline Object

#include "Mustard/Data/Object/FieldTypeName.inl"
