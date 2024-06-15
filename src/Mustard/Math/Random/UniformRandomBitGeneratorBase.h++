#pragma once

#include "Mustard/Math/Random/UniformRandomBitGenerator.h++"

#include <concepts>

namespace Mustard::Math::Random {

/// @brief Well-formed derivation of this class fulfills
/// concepts UniformRandomBitGenerator and STDUniformRandomBitGenerator
/// (the C++ named requirements UniformRandomBitGenerator).
/// @tparam ADerived The finally derived class.
/// @tparam AResult The output type of derived URBG.
template<typename ADerived,
         std::unsigned_integral AResult>
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
