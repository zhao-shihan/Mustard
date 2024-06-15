#pragma once

#include "Mustard/Concept/FundamentalType.h++"
#include "Mustard/Math/Random/Generator/XoshiroBase.h++"
#include "Mustard/Utility/InlineMacro.h++"

#include <bit>
#include <concepts>
#include <cstdint>
#include <istream>
#include <ostream>

namespace Mustard::Math::Random::inline Generator {

template<typename ADerived>
class Xoshiro256Base : public XoshiroBase<ADerived, 256> {
protected:
    constexpr Xoshiro256Base();
    constexpr explicit Xoshiro256Base(std::uint64_t seed);
    constexpr ~Xoshiro256Base() = default;

public:
    MUSTARD_ALWAYS_INLINE constexpr auto Step() -> void;

    template<Concept::Character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const Xoshiro256Base& self) -> decltype(os) { return self.StreamOutput(os); }
    template<Concept::Character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, Xoshiro256Base& self) -> decltype(is) { return self.StreamInput(is); }

private:
    template<Concept::Character AChar>
    auto StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os);
    template<Concept::Character AChar>
    auto StreamInput(std::basic_istream<AChar>& is) & -> decltype(is);
};

} // namespace Mustard::Math::Random::inline Generator

#include "Mustard/Math/Random/Generator/Xoshiro256Base.inl"
