#pragma once

#include "Mustard/Concept/FundamentalType.h++"
#include "Mustard/Math/Random/UniformPseudoRandomBitGeneratorBase.h++"
#include "Mustard/Utility/InlineMacro.h++"

#include <cstdint>
#include <istream>
#include <limits>
#include <ostream>

namespace Mustard::Math::Random::inline Generator {

class SplitMix64 final : public UniformPseudoRandomBitGeneratorBase<SplitMix64,
                                                                    std::uint64_t,
                                                                    std::uint64_t> {
public:
    constexpr SplitMix64();
    constexpr explicit SplitMix64(SeedType seed);

    MUSTARD_ALWAYS_INLINE constexpr auto operator()() -> SplitMix64::ResultType;
    constexpr auto Seed(SeedType seed) -> void;

    static constexpr auto Min() -> auto { return std::numeric_limits<ResultType>::min(); }
    static constexpr auto Max() -> auto { return std::numeric_limits<ResultType>::max(); }

    template<Concept::Character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const SplitMix64& self) -> decltype(auto) { return os << self.fState; }
    template<Concept::Character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, SplitMix64& self) -> decltype(auto) { return is >> self.fState; }

private:
    ResultType fState;
};

} // namespace Mustard::Math::Random::inline Generator

#include "Mustard/Math/Random/Generator/SplitMix64.inl"
