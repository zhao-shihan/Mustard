#pragma once

#include "Mustard/Concept/FundamentalType.h++"
#include "Mustard/Math/Random/UniformPseudoRandomBitGeneratorBase.h++"

#include <random>

namespace Mustard::Math::Random::inline Generator {

class MT1993732 final : public UniformPseudoRandomBitGeneratorBase<MT1993732,
                                                                   std::mt19937::result_type,
                                                                   std::mt19937::result_type> {
public:
    MT1993732() = default;
    explicit MT1993732(SeedType seed);

    auto operator()() -> auto { return fMT(); }
    auto Seed(SeedType seed) -> void { fMT.seed(seed); }

    static constexpr auto Min() -> auto { return std::mt19937::min(); }
    static constexpr auto Max() -> auto { return std::mt19937::max(); }

    template<Concept::Character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const MT1993732& self) -> decltype(auto) { return os << self.fMT; }
    template<Concept::Character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, MT1993732& self) -> decltype(auto) { return is >> self.fMT; }

private:
    std::mt19937 fMT;
};

} // namespace Mustard::Math::Random::inline Generator

#include "Mustard/Math/Random/Generator/MT1993732.inl"
