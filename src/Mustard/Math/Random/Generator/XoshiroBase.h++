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

#include "Mustard/Math/Random/Generator/SplitMix64.h++"
#include "Mustard/Math/Random/UniformPseudoRandomBitGeneratorBase.h++"

#include <algorithm>
#include <array>
#include <concepts>
#include <cstdint>
#include <limits>
#include <utility>

namespace Mustard::Math::Random::inline Generator {

template<typename ADerived, std::size_t NBit>
    requires(NBit % 64 == 0)
class XoshiroBase : public UniformPseudoRandomBitGeneratorBase<ADerived,
                                                               std::uint64_t,
                                                               std::uint64_t> {
protected:
    constexpr XoshiroBase(std::array<std::uint64_t, NBit / 64> defaultState);
    constexpr explicit XoshiroBase(std::uint64_t seed);
    constexpr ~XoshiroBase() = default;

public:
    constexpr auto Seed(std::uint64_t seed) -> void;

    static constexpr auto Min() -> auto { return std::numeric_limits<std::uint64_t>::min(); }
    static constexpr auto Max() -> auto { return std::numeric_limits<std::uint64_t>::max(); }

protected:
    std::array<std::uint64_t, NBit / 64> fState;
};

} // namespace Mustard::Math::Random::inline Generator

#include "Mustard/Math/Random/Generator/XoshiroBase.inl"
