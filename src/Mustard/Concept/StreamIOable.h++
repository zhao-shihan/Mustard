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

#include <istream>
#include <ostream>

namespace Mustard::Concept {

template<typename T>
concept StreamInputable =
    requires(std::istream& is, std::wistream& wis, T& obj) {
        { is >> obj } -> std::same_as<std::istream&>;
        { wis >> obj } -> std::same_as<std::wistream&>;
    };

template<typename T>
concept StreamOutputable =
    requires(std::ostream& os, std::wostream& wos, const T obj) {
        { os << obj } -> std::same_as<std::ostream&>;
        { wos << obj } -> std::same_as<std::wostream&>;
    };

template<typename T>
concept StreamIOable = StreamInputable<T> and StreamOutputable<T>;

} // namespace Mustard::Concept
