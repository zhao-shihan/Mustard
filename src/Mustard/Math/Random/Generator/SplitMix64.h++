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

#include "Mustard/Math/Random/UniformPseudoRandomBitGeneratorBase.h++"
#include "Mustard/Utility/InlineMacro.h++"

#include "muc/concepts"

#include <cstdint>
#include <istream>
#include <limits>
#include <ostream>

namespace Mustard::Math::Random::inline Generator {

class SplitMix64 final : public UniformPseudoRandomBitGeneratorBase<SplitMix64,
                                                                    std::uint64_t,
                                                                    std::uint64_t> {
public:
    constexpr SplitMix64();
    constexpr explicit SplitMix64(SeedType seed);

    MUSTARD_ALWAYS_INLINE constexpr auto operator()() -> SplitMix64::ResultType;
    constexpr auto Seed(SeedType seed) -> void;

    static constexpr auto Min() -> auto { return std::numeric_limits<ResultType>::min(); }
    static constexpr auto Max() -> auto { return std::numeric_limits<ResultType>::max(); }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const SplitMix64& self) -> decltype(auto) { return os << self.fState; }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, SplitMix64& self) -> decltype(auto) { return is >> self.fState; }

private:
    ResultType fState;
};

} // namespace Mustard::Math::Random::inline Generator

#include "Mustard/Math/Random/Generator/SplitMix64.inl"
