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

#include <string>

namespace Mustard::inline Utility {

/// @brief Tag identifying the calling MPI process, `"r<rank>@<hostname>"`.
/// @return Empty string when MPI is unavailable.
/// @note This is the bare process identity; callers add any decoration
///       (for example a trailing `"> "` for a log line prefix).
auto ProcessTag() -> std::string;

} // namespace Mustard::inline Utility
