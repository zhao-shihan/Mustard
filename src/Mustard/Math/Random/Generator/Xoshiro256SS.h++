#pragma once

#include "Mustard/Math/Random/Generator/Xoshiro256Base.h++"
#include "Mustard/Utility/InlineMacro.h++"

#include <bit>

namespace Mustard::Math::Random::inline Generator {

class Xoshiro256SS final : public Xoshiro256Base<Xoshiro256SS> {
public:
    constexpr Xoshiro256SS() = default;
    constexpr explicit Xoshiro256SS(SeedType seed);

    MUSTARD_ALWAYS_INLINE constexpr auto operator()() -> ResultType;
};

} // namespace Mustard::Math::Random::inline Generator

#include "Mustard/Math/Random/Generator/Xoshiro256SS.inl"
