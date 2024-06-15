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
