// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
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

#include "Mustard/Env/Print.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "fmt/color.h"

#include <source_location>
#include <string_view>

namespace Mustard::Env {

/// @brief Print pretty information.
/// @param message The message.
/// @param location Source location. Default is current.
template<typename... Ts>
auto PrintPrettyInfo(std::string_view message, const std::source_location& location = std::source_location::current()) -> void;
/// @brief Print pretty warning.
/// @param message The message.
/// @param location Source location. Default is current.
template<typename... Ts>
auto PrintPrettyWarning(std::string_view message, const std::source_location& location = std::source_location::current()) -> void;
/// @brief Print pretty error.
/// @param message The message.
/// @param location Source location. Default is current.
template<typename... Ts>
auto PrintPrettyError(std::string_view message, const std::source_location& location = std::source_location::current()) -> void;

} // namespace Mustard::Env

#include "Mustard/Env/Logging.inl"
