// -*- C++ -*-
//
// Copyright 2020-2025  The Mustard development team
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

#include "mplr/mplr.hpp"

#include <concepts>
#include <source_location>
#include <string>
#include <string_view>

namespace Mustard::inline IO {

/// @brief Print pretty information.
/// @param message The message.
/// @param location Source location. Default is current.
auto PrintInfo(std::string_view message, const std::source_location& location = std::source_location::current()) -> void;

/// @brief Print pretty warning.
/// @param message The message.
/// @param location Source location. Default is current.
auto PrintWarning(std::string_view message, const std::source_location& location = std::source_location::current()) -> void;

/// @brief Print pretty error.
/// @param message The message.
/// @param location Source location. Default is current.
auto PrintError(std::string_view message, const std::source_location& location = std::source_location::current()) -> void;

/// @brief Print pretty information from master proccess.
/// @param message The message.
/// @param location Source location. Default is current.
auto MasterPrintInfo(std::string_view message, const std::source_location& location = std::source_location::current()) -> void;

/// @brief Print pretty warning from master proccess.
/// @param message The message.
/// @param location Source location. Default is current.
auto MasterPrintWarning(std::string_view message, const std::source_location& location = std::source_location::current()) -> void;

/// @brief Print pretty error from master proccess.
/// @param message The message.
/// @param location Source location. Default is current.
auto MasterPrintError(std::string_view message, const std::source_location& location = std::source_location::current()) -> void;

/// @brief Throw pretty exception.
/// @tparam AException The exception type.
/// @param message The exception message.
/// @param location Source location. Default is current.
template<std::constructible_from<std::string> AException>
[[noreturn]] auto Throw(std::string_view message, const std::source_location& location = std::source_location::current()) -> void;

namespace internal {
auto PrettyException(std::string_view message, const std::source_location& location) -> std::string;
} // namespace internal

} // namespace Mustard::inline IO

#include "Mustard/IO/PrettyLog.inl"
