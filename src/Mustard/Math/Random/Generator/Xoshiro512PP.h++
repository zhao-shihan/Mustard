#pragma once

#include "Mustard/Math/Random/Generator/Xoshiro512Base.h++"
#include "Mustard/Utility/InlineMacro.h++"

#include <bit>

namespace Mustard::Math::Random::inline Generator {

class Xoshiro512PP final : public Xoshiro512Base<Xoshiro512PP> {
public:
    constexpr Xoshiro512PP() = default;
    constexpr explicit Xoshiro512PP(SeedType seed);

    MUSTARD_ALWAYS_INLINE constexpr auto operator()() -> ResultType;
};

} // namespace Mustard::Math::Random::inline Generator

#include "Mustard/Math/Random/Generator/Xoshiro512PP.inl"
