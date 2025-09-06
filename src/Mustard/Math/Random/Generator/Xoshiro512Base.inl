// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

namespace Mustard::Math::Random::inline Generator {

template<typename ADerived>
constexpr Xoshiro512Base<ADerived>::Xoshiro512Base() : // clang-format off
    XoshiroBase<ADerived, 512>{{0x893C3E22C678FAA9ull, 
                                0x30589ADC78696ADAull,
                                0x1D541511D5F51D5Bull,
                                0xE3CBD397A993A9EEull,
                                0x1A7CB96107B6F389ull,
                                0xD947C32467BF2036ull,
                                0xD7A5E004646E6A58ull,
                                0x6426F7F8732F5C48ull}} { // clang-format on
    static_assert(std::derived_from<ADerived, Xoshiro512Base<ADerived>>);
}

template<typename ADerived>
constexpr Xoshiro512Base<ADerived>::Xoshiro512Base(std::uint64_t seed) :
    XoshiroBase<ADerived, 512>{seed} {}

template<typename ADerived>
MUSTARD_ALWAYS_INLINE constexpr auto Xoshiro512Base<ADerived>::Step() -> void {
    const auto t{this->fState[1] << 11};

    this->fState[2] ^= this->fState[0];
    this->fState[5] ^= this->fState[1];
    this->fState[1] ^= this->fState[2];
    this->fState[7] ^= this->fState[3];
    this->fState[3] ^= this->fState[4];
    this->fState[4] ^= this->fState[5];
    this->fState[0] ^= this->fState[6];
    this->fState[6] ^= this->fState[7];

    this->fState[6] ^= t;

    this->fState[7] = std::rotl(this->fState[7], 21);
}

template<typename ADerived>
template<muc::character AChar>
auto Xoshiro512Base<ADerived>::StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os) {
    return os << this->fState[0] << ' '
              << this->fState[1] << ' '
              << this->fState[2] << ' '
              << this->fState[3] << ' '
              << this->fState[4] << ' '
              << this->fState[5] << ' '
              << this->fState[6] << ' '
              << this->fState[7];
}

template<typename ADerived>
template<muc::character AChar>
auto Xoshiro512Base<ADerived>::StreamInput(std::basic_istream<AChar>& is) & -> decltype(is) {
    return is >> this->fState[0] // clang-format off
              >> this->fState[1]
              >> this->fState[2]
              >> this->fState[3]
              >> this->fState[4]
              >> this->fState[5]
              >> this->fState[6]
              >> this->fState[7]; // clang-format on
}

} // namespace Mustard::Math::Random::inline Generator
