// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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

#include "muc/concepts"

#include <random>

namespace Mustard::Math::Random::inline Generator {

class MT1993732 final : public UniformPseudoRandomBitGeneratorBase<MT1993732,
                                                                   std::mt19937::result_type,
                                                                   std::mt19937::result_type> {
public:
    MT1993732() = default;
    explicit MT1993732(SeedType seed);

    auto operator()() -> auto { return fMT(); }
    auto Seed(SeedType seed) -> void { fMT.seed(seed); }

    static constexpr auto Min() -> auto { return std::mt19937::min(); }
    static constexpr auto Max() -> auto { return std::mt19937::max(); }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const MT1993732& self) -> decltype(auto) { return os << self.fMT; }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, MT1993732& self) -> decltype(auto) { return is >> self.fMT; }

private:
    std::mt19937 fMT;
};

} // namespace Mustard::Math::Random::inline Generator

#include "Mustard/Math/Random/Generator/MT1993732.inl"
