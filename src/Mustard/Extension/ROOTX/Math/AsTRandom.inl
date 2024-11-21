// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
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

namespace Mustard::inline Extension::ROOTX::Math {

template<Mustard::Math::Random::UniformPseudoRandomBitGenerator PRBG>
AsTRandom<PRBG>::AsTRandom(typename PRBG::SeedType seed) :
    fPRBG{seed},
    fGaussian{} {}

template<Mustard::Math::Random::UniformPseudoRandomBitGenerator PRBG>
auto AsTRandom<PRBG>::RndmArray(Int_t n, Float_t* array) -> void {
    std::ranges::generate_n(array, n, [this] { return Mustard::Math::Random::Uniform<Float_t>{}(fPRBG); });
}

template<Mustard::Math::Random::UniformPseudoRandomBitGenerator PRBG>
auto AsTRandom<PRBG>::RndmArray(Int_t n, Double_t* array) -> void {
    std::ranges::generate_n(array, n, [this] { return Rndm(); });
}

template<Mustard::Math::Random::UniformPseudoRandomBitGenerator PRBG>
auto AsTRandom<PRBG>::GetSeed() const -> UInt_t {
    PrintError("AsTRandom<PRBG>::GetSeed has no effect. Do not use");
    return 0;
}

} // namespace Mustard::inline Extension::ROOTX::Math
