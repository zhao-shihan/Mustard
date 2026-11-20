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

#include "Mustard/IO/PrettyLog.h++"

#include "TBase64.h"

#include "fmt/format.h"

#include <array>
#include <bit>
#include <cstring>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace Mustard::inline Utility {

/// @brief Encode a trivially copyable object to a Base64 string.
/// @tparam T A trivially copyable type
/// @param pod The object to encode
/// @return Base64-encoded representation
template<typename T>
    requires std::is_trivially_copyable_v<T>
auto ToBase64(const T& pod) -> std::string;

/// @brief Decode a Base64 string into a trivially copyable object.
/// @tparam T A trivially copyable type
/// @param base64 Base64-encoded byte representation
/// @return The decoded object
/// @throws std::runtime_error if the decoded length does not match sizeof(T)
template<typename T>
    requires std::is_trivially_copyable_v<T>
auto FromBase64(const std::string& base64) -> T;

} // namespace Mustard::inline Utility

#include "Mustard/Utility/Base64.inl"
