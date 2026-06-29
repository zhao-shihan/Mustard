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

namespace Mustard::inline Utility {

// note: rule of five, and
// C.21: If you define or =delete any copy, move, or destructor function, define or =delete them all
class MovableVirtualBase {
protected:
    constexpr MovableVirtualBase() = default;
    constexpr virtual ~MovableVirtualBase() = default;

public:
    constexpr MovableVirtualBase(const MovableVirtualBase&) noexcept = default;
    constexpr MovableVirtualBase(MovableVirtualBase&&) noexcept = default;
    constexpr auto operator=(const MovableVirtualBase&) noexcept -> MovableVirtualBase& = default;
    constexpr auto operator=(MovableVirtualBase&&) noexcept -> MovableVirtualBase& = default;
};

// note: rule of five, and
// C.21: If you define or =delete any copy, move, or destructor function, define or =delete them all
class MoveOnlyVirtualBase {
protected:
    constexpr MoveOnlyVirtualBase() = default;
    constexpr virtual ~MoveOnlyVirtualBase() = default;

public:
    constexpr MoveOnlyVirtualBase(MoveOnlyVirtualBase&&) noexcept = default;
    constexpr auto operator=(MoveOnlyVirtualBase&&) noexcept -> MoveOnlyVirtualBase& = default;
};

// C.21: If you define or =delete any copy, move, or destructor function, define or =delete them all
class NonCopyableVirtualBase {
protected:
    constexpr NonCopyableVirtualBase() = default;
    constexpr virtual ~NonCopyableVirtualBase() = default;

public:
    constexpr NonCopyableVirtualBase(const NonCopyableVirtualBase&) noexcept = delete;
    constexpr auto operator=(const NonCopyableVirtualBase&) noexcept -> NonCopyableVirtualBase& = delete;
};

} // namespace Mustard::inline Utility
