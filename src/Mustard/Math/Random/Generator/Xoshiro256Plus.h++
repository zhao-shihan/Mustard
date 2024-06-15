#pragma once

#include "Mustard/Math/Random/Generator/Xoshiro256Base.h++"
#include "Mustard/Utility/InlineMacro.h++"

namespace Mustard::Math::Random::inline Generator {

class Xoshiro256Plus final : public Xoshiro256Base<Xoshiro256Plus> {
public:
    constexpr Xoshiro256Plus() = default;
    constexpr explicit Xoshiro256Plus(SeedType seed);

    MUSTARD_ALWAYS_INLINE constexpr auto operator()() -> ResultType;
};

} // namespace Mustard::Math::Random::inline Generator

#include "Mustard/Math/Random/Generator/Xoshiro256Plus.inl"
