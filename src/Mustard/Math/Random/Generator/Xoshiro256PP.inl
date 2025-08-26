// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

namespace Mustard::Math::Random::inline Generator {

constexpr Xoshiro256PP::Xoshiro256PP(Xoshiro256PP::SeedType seed) :
    Xoshiro256Base{seed} {}

MUSTARD_ALWAYS_INLINE constexpr auto Xoshiro256PP::operator()() -> Xoshiro256PP::ResultType {
    const auto result{std::rotl(fState[0] + fState[3], 23) + fState[0]};
    Step();
    return result;
}

} // namespace Mustard::Math::Random::inline Generator
