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

#include "Mustard/Utility/PrettyLog.h++"

#include "muc/utility"

#include "fmt/core.h"

#include <cstdint>
#include <stdexcept>

namespace Mustard::inline Extension::MPIX {

class DutyRatio {
public:
    enum Preset : std::int8_t {
        Active = 'A',   // 0.1
        Moderate = 'M', // 0.01
        Relaxed = 'R'   // 0.001
    };

public:
    constexpr DutyRatio(Preset preset);
    constexpr explicit DutyRatio(double dutyRatio);

    constexpr operator double() const { return fDutyRatio; };
    constexpr auto SleepRatio() const -> auto { return 1 - fDutyRatio; };

private:
    double fDutyRatio;
};

} // namespace Mustard::inline Extension::MPIX

#include "Mustard/Extension/MPIX/DutyRatio.inl"
