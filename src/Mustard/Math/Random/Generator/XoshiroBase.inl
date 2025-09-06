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

template<typename ADerived, std::size_t NBit>
    requires(NBit % 64 == 0)
constexpr XoshiroBase<ADerived, NBit>::XoshiroBase(std::array<std::uint64_t, NBit / 64> defaultState) :
    UniformPseudoRandomBitGeneratorBase<ADerived, std::uint64_t, std::uint64_t>{},
    fState{std::move(defaultState)} {
    static_assert(std::derived_from<ADerived, XoshiroBase<ADerived, NBit>>);
}

template<typename ADerived, std::size_t NBit>
    requires(NBit % 64 == 0)
constexpr XoshiroBase<ADerived, NBit>::XoshiroBase(std::uint64_t seed) :
    UniformPseudoRandomBitGeneratorBase<ADerived, std::uint64_t, std::uint64_t>{},
    fState{} {
    Seed(seed);
}

template<typename ADerived, std::size_t NBit>
    requires(NBit % 64 == 0)
constexpr auto XoshiroBase<ADerived, NBit>::Seed(std::uint64_t seed) -> void {
    SplitMix64 splitMix64{seed};
    std::ranges::generate(fState, splitMix64);
    static_cast<ADerived*>(this)->Step();
    static_cast<ADerived*>(this)->Step();
    static_cast<ADerived*>(this)->Step();
}

} // namespace Mustard::Math::Random::inline Generator
