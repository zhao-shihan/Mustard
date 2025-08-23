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

#include "Mustard/Utility/FormatToLocalTime.h++"

#include <format>

namespace Mustard::inline Utility {

auto FormatToLocalTime(const std::chrono::system_clock::time_point& now) -> std::string {
    namespace sc = std::chrono;
    const sc::sys_time<sc::milliseconds> msNow{sc::round<sc::milliseconds>(now.time_since_epoch())};
    const sc::zoned_time<sc::milliseconds> localNow{std::chrono::current_zone(), msNow};
    return std::format("{:%FT%T%z}", localNow);
}

} // namespace Mustard::inline Utility
