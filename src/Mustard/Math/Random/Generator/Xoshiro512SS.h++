#pragma once

#include "Mustard/Math/Random/Generator/Xoshiro512Base.h++"
#include "Mustard/Utility/InlineMacro.h++"

#include <bit>

namespace Mustard::Math::Random::inline Generator {

class Xoshiro512SS final : public Xoshiro512Base<Xoshiro512SS> {
public:
    constexpr Xoshiro512SS() = default;
    constexpr explicit Xoshiro512SS(SeedType seed);

    MUSTARD_ALWAYS_INLINE constexpr auto operator()() -> ResultType;
};

} // namespace Mustard::Math::Random::inline Generator

#include "Mustard/Math/Random/Generator/Xoshiro512SS.inl"
