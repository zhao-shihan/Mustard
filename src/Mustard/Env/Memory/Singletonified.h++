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

#include "Mustard/Concept/NonCopyable.h++"

#include <concepts>
#include <type_traits>

namespace Mustard::Env::Memory {

namespace internal {

class WeakSingletonBase;
class SingletonBase;

} // namespace internal

template<typename ADerived>
class Singleton;

template<typename T>
concept Singletonified =
    requires {
        { T::Instance() } -> std::same_as<T&>;
        requires std::derived_from<T, Singleton<T>>;
        requires std::derived_from<T, internal::SingletonBase>;
        requires not std::is_base_of_v<internal::WeakSingletonBase, T>;
        requires Concept::NonCopyable<T>;
        requires std::is_final_v<T>;
        requires not std::is_default_constructible_v<T>; // try to constrain to private or protected constructor
    };

} // namespace Mustard::Env::Memory
