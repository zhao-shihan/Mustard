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
