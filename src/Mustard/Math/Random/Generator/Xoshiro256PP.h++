#pragma once

#include "Mustard/Math/Random/Generator/Xoshiro256Base.h++"
#include "Mustard/Utility/InlineMacro.h++"

#include <bit>

namespace Mustard::Math::Random::inline Generator {

class Xoshiro256PP final : public Xoshiro256Base<Xoshiro256PP> {
public:
    constexpr Xoshiro256PP() = default;
    constexpr explicit Xoshiro256PP(SeedType seed);

    MUSTARD_ALWAYS_INLINE constexpr auto operator()() -> ResultType;
};

} // namespace Mustard::Math::Random::inline Generator

#include "Mustard/Math/Random/Generator/Xoshiro256PP.inl"
