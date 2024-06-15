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
class Xoshiro512Base : public XoshiroBase<ADerived, 512> {
protected:
    constexpr Xoshiro512Base();
    constexpr explicit Xoshiro512Base(std::uint64_t seed);
    constexpr ~Xoshiro512Base() = default;

public:
    MUSTARD_ALWAYS_INLINE constexpr auto Step() -> void;

    template<Concept::Character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const Xoshiro512Base& self) -> decltype(os) { return self.StreamOutput(os); }
    template<Concept::Character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, Xoshiro512Base& self) -> decltype(is) { return self.StreamInput(is); }

private:
    template<Concept::Character AChar>
    auto StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os);
    template<Concept::Character AChar>
    auto StreamInput(std::basic_istream<AChar>& is) & -> decltype(is);
};

} // namespace Mustard::Math::Random::inline Generator

#include "Mustard/Math/Random/Generator/Xoshiro512Base.inl"
