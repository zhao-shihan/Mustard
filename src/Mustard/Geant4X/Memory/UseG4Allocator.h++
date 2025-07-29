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

#include "Mustard/Geant4X/Memory/SingletonG4Allocator.h++"
#include "Mustard/Utility/InlineMacro.h++"

#include <concepts>
#include <cstddef>
#include <type_traits>

namespace Mustard::Geant4X::inline Memory {

template<typename ADerived>
class UseG4Allocator {
protected:
    UseG4Allocator() = default;
    ~UseG4Allocator() = default;

public:
    [[nodiscard]] MUSTARD_ALWAYS_INLINE auto operator new(std::size_t) -> void*;
    [[nodiscard]] MUSTARD_ALWAYS_INLINE auto operator new[](std::size_t) -> void* = delete;

    MUSTARD_ALWAYS_INLINE auto operator delete(void* ptr) -> void;
    MUSTARD_ALWAYS_INLINE auto operator delete[](void*) -> void = delete;
};

} // namespace Mustard::Geant4X::inline Memory

#include "Mustard/Geant4X/Memory/UseG4Allocator.inl"
