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

#pragma once

#include "Mustard/Math/Random/UniformRandomBitGenerator.h++"

#include <concepts>

namespace Mustard::Math::Random {

/// @brief Well-formed derivation of this class fulfills
/// concepts UniformRandomBitGenerator and STDUniformRandomBitGenerator
/// (the C++ named requirements UniformRandomBitGenerator).
/// @tparam ADerived The finally derived class.
/// @tparam AResult The output type of derived URBG.
template<typename ADerived, std::unsigned_integral AResult>
class UniformRandomBitGeneratorBase {
public:
    using ResultType = AResult;
    using result_type = ResultType;

protected:
    constexpr UniformRandomBitGeneratorBase();
    constexpr ~UniformRandomBitGeneratorBase() = default;

public:
    static constexpr auto min() { return ADerived::Min(); }
    static constexpr auto max() { return ADerived::Max(); }

    constexpr bool operator==(const UniformRandomBitGeneratorBase&) const = default;
};

} // namespace Mustard::Math::Random

#include "Mustard/Math/Random/UniformRandomBitGeneratorBase.inl"
