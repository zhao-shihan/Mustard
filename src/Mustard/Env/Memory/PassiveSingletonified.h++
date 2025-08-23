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

#include "Mustard/Concept/NonCopyable.h++"

#include <concepts>
#include <type_traits>

namespace Mustard::Env::Memory {

namespace internal {

class SingletonBase;

} // namespace internal

template<typename ADerived>
class PassiveSingleton;

template<typename T>
concept PassiveSingletonified =
    requires {
        { T::Instance() } -> std::same_as<T&>;
        requires std::derived_from<T, PassiveSingleton<T>>;
        requires not std::is_base_of_v<internal::SingletonBase, T>;
        requires Concept::NonCopyable<T>;
    };

} // namespace Mustard::Env::Memory
