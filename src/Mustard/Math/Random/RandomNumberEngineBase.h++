// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
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

#include "Mustard/Math/Random/RandomNumberEngine.h++"
#include "Mustard/Math/Random/UniformRandomBitGeneratorBase.h++"
#include "Mustard/Utility/ClassSemanticHelper.h++"

#include <concepts>
#include <utility>

namespace Mustard::inline Math::Random {

/// @brief Well-formed derivation of this class fulfills
/// the concept RandomNumberEngine.
/// @tparam ADerived The finally derived class.
/// @tparam AResult The output type of derived PRNG.
template<typename ADerived, std::unsigned_integral AResult, std::unsigned_integral ASeed>
class RandomNumberEngineBase : public UniformRandomBitGeneratorBase<ADerived, AResult> {
public:
    using SeedType = ASeed;

protected:
    constexpr RandomNumberEngineBase();
    MUSTARD_UTILITY_DECLARE_NONVIRTUAL_PROTECTED_DEFAULT_DESTRUCTOR_IN_DEFAULT_MOVEABLE(RandomNumberEngineBase)

public:
    constexpr auto Seed() -> void { Self() = ADerived{}; }
    constexpr auto Discard(unsigned long long z) -> void;

    constexpr auto seed() -> void { Self().Seed(); }
    constexpr auto seed(SeedType s) -> void { Self().Seed(s); }
    constexpr auto discard(unsigned long long z) -> void { Self().Discard(z); }

private:
    auto Self() & -> auto&& { return static_cast<ADerived&>(*this); }
    auto Self() const& -> auto&& { return static_cast<const ADerived&>(*this); }
    auto Self() && -> auto&& { return static_cast<ADerived&&>(*this); }
    auto Self() const&& -> auto&& { return static_cast<const ADerived&&>(*this); }
};

} // namespace Mustard::inline Math::Random

#include "Mustard/Math/Random/RandomNumberEngineBase.inl"
