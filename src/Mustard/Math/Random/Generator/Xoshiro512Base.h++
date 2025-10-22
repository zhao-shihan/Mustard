// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
//
// This file is part of Mustard, an offline software framework for HEP experiments.
//
// Mustard is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// Mustard is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// Mustard. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "Mustard/Math/Random/Generator/XoshiroBase.h++"
#include "Mustard/Utility/FunctionAttribute.h++"

#include "muc/concepts"

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

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const Xoshiro512Base& self) -> decltype(os) { return self.StreamOutput(os); }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, Xoshiro512Base& self) -> decltype(is) { return self.StreamInput(is); }

private:
    template<muc::character AChar>
    auto StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os);
    template<muc::character AChar>
    auto StreamInput(std::basic_istream<AChar>& is) & -> decltype(is);
};

} // namespace Mustard::Math::Random::inline Generator

#include "Mustard/Math/Random/Generator/Xoshiro512Base.inl"
