// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

#include <memory>
#include <string>
#include <string_view>

class TMacro;

namespace Mustard::ROOTX {

/// @brief Creates a TMacro object that prints text when executed
///
/// Generates a temporary ROOT macro file (.C) containing a function that prints the provided text.
/// The macro is read into a TMacro object and the temporary file is automatically cleaned up.
///
/// @param text The content to be printed by the macro (raw string, including newlines)
/// @param name The name of the generated macro function (used as function name and TMacro name)
/// @param title The title metadata for the TMacro object
/// @return std::unique_ptr<TMacro> Ownership of the created macro object
///
/// @throws std::runtime_error If:
///   - Temporary file creation fails
///   - Writing to the temporary file fails
///   - Reading the generated macro fails
///
/// @note Implementation details:
///   1. Creates a uniquely-named temporary file in the system temp directory
///   2. Writes a C++ function that prints the raw `text` using a raw string literal
///   3. Reads the file contents into a TMacro object
///   4. Silently removes the temporary file (errors ignored)
///
/// @warning The `text` parameter should not contain the sequence `)"{0}"` as this
///          would break the raw string delimiter.
///
/// @see TMacro ROOT documentation for macro execution details
auto MakeTextTMacro(std::string_view text, const std::string& name, const std::string& title = {}) -> std::unique_ptr<TMacro>;

} // namespace Mustard::ROOTX
