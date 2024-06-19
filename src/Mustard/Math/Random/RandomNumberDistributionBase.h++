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

#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Math/Random/RandomNumberDistribution.h++"

#include <type_traits>

namespace Mustard::Math::Random {

template<typename ADerived, typename ADistribution>
class DistributionParameterBase {
public:
    using DistributionType = ADistribution;
    using distribution_type = DistributionType;

protected:
    constexpr DistributionParameterBase();
    constexpr ~DistributionParameterBase() = default;

public:
    constexpr bool operator==(const DistributionParameterBase&) const = default;
};

template<typename ADerived, typename AParameter, typename T>
    requires(std::is_arithmetic_v<T> or Concept::NumericVectorAny<T>)
class RandomNumberDistributionBase {
public:
    using ResultType = T;
    using ParameterType = AParameter;
    using result_type = ResultType;
    using param_type = ParameterType;

protected:
    constexpr RandomNumberDistributionBase();
    constexpr ~RandomNumberDistributionBase() = default;

public:
    auto reset() -> void { static_cast<ADerived*>(this)->Reset(); }

    auto param() const -> auto { return static_cast<const ADerived*>(this)->Parameter(); }
    auto param(const AParameter& p) -> void { static_cast<ADerived*>(this)->Parameter(p); }

    auto min() const -> auto { return static_cast<const ADerived*>(this)->Min(); }
    auto max() const -> auto { return static_cast<const ADerived*>(this)->Max(); }

    constexpr auto operator==(const RandomNumberDistributionBase&) const -> bool = default;
    constexpr auto operator<=>(const RandomNumberDistributionBase&) const -> auto = delete;
};

} // namespace Mustard::Math::Random

#include "Mustard/Math/Random/RandomNumberDistributionBase.inl"
