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

namespace Mustard::inline Utility {

class MerelyMoveableBase {
protected:
    constexpr MerelyMoveableBase() noexcept = default;
    constexpr ~MerelyMoveableBase() noexcept = default;

    constexpr MerelyMoveableBase(const MerelyMoveableBase&) noexcept = delete;
    constexpr MerelyMoveableBase(MerelyMoveableBase&&) noexcept = default;

    constexpr auto operator=(const MerelyMoveableBase&) noexcept -> MerelyMoveableBase& = delete;
    constexpr auto operator=(MerelyMoveableBase&&) noexcept -> MerelyMoveableBase& = default;
};

} // namespace Mustard::inline Utility
