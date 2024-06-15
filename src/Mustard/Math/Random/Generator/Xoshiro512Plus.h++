#pragma once

#include "Mustard/Math/Random/Generator/Xoshiro512Base.h++"
#include "Mustard/Utility/InlineMacro.h++"

namespace Mustard::Math::Random::inline Generator {

class Xoshiro512Plus final : public Xoshiro512Base<Xoshiro512Plus> {
public:
    constexpr Xoshiro512Plus() = default;
    constexpr explicit Xoshiro512Plus(SeedType seed);

    MUSTARD_ALWAYS_INLINE constexpr auto operator()() -> ResultType;
};

} // namespace Mustard::Math::Random::inline Generator

#include "Mustard/Math/Random/Generator/Xoshiro512Plus.inl"
