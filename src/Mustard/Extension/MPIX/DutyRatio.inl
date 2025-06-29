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

namespace Mustard::inline Extension::MPIX {

constexpr DutyRatio::DutyRatio(Preset dutyRatio) :
    DutyRatio{[&] {
        switch (dutyRatio) {
        case Active:
            return 0.1;
        case Moderate:
            return 0.01;
        case Relaxed:
            return 0.001;
        }
        muc::unreachable();
    }()} {}

constexpr DutyRatio::DutyRatio(double dutyRatio) :
    fDutyRatio{dutyRatio} {
    if (fDutyRatio <= 0 or fDutyRatio > 1) {
        Mustard::Throw<std::invalid_argument>(fmt::format("Duty ratio {} out of (0,1]", fDutyRatio));
    }
}

} // namespace Mustard::inline Extension::MPIX
