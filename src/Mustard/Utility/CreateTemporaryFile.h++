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

#include <filesystem>
#include <string_view>

namespace Mustard::inline Utility {

/// @brief Create an empty temporary file and close it. The file is guaranteed to be created if this function return, so its file name can be safely reused.
///        In other words, the intention of this function is to occupy a available temporary file name (p.s. std::tmpnam is dangerous).
/// @param signature An optional file name signature.
/// @param extension The extension of the file name. No extension if not provided.
/// @return A std::filesystem::path refering to the created file.
/// @exception Throw a std::runtime_error if failed.
auto CreateTemporaryFile(std::string_view signature = {}, std::filesystem::path extension = {}) -> std::filesystem::path;

} // namespace Mustard::inline Utility
