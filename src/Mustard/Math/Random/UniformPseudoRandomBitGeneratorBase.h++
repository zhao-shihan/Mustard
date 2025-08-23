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

#include "Mustard/Math/Random/UniformPseudoRandomBitGenerator.h++"
#include "Mustard/Math/Random/UniformRandomBitGeneratorBase.h++"

#include <concepts>
#include <utility>

namespace Mustard::Math::Random {

/// @brief Well-formed derivation of this class fulfills
/// the concept UniformPseudoRandomBitGenerator.
/// @tparam ADerived The finally derived class.
/// @tparam AResult The output type of derived PRBG.
template<typename ADerived,
         std::unsigned_integral AResult,
         std::unsigned_integral ASeed>
class UniformPseudoRandomBitGeneratorBase : public UniformRandomBitGeneratorBase<ADerived, AResult> {
public:
    using SeedType = ASeed;

protected:
    constexpr UniformPseudoRandomBitGeneratorBase();
    constexpr ~UniformPseudoRandomBitGeneratorBase() = default;

public:
    auto seed(SeedType s) -> void { return static_cast<ADerived*>(this)->Seed(s); }
};

} // namespace Mustard::Math::Random

#include "Mustard/Math/Random/UniformPseudoRandomBitGeneratorBase.inl"
