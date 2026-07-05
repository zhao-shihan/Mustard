// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
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

namespace Mustard::ROOTX::Math {

template<muc::random_number_engine PRNG>
AsTRandom<PRNG>::AsTRandom(typename PRNG::SeedType seed) :
    fPRNG{seed},
    fGaus{} {}

template<muc::random_number_engine PRNG>
auto AsTRandom<PRNG>::RndmArray(int n, float* array) -> void {
    std::ranges::generate_n(array, n, [this] { return Random::Uniform<float>{}(fPRNG); });
}

template<muc::random_number_engine PRNG>
auto AsTRandom<PRNG>::RndmArray(int n, double* array) -> void {
    std::ranges::generate_n(array, n, [this] { return Random::Uniform<double>{}(fPRNG); });
}

template<muc::random_number_engine PRNG>
auto AsTRandom<PRNG>::GetSeed() const -> unsigned {
    PrintError("AsTRandom<PRNG>::GetSeed has no effect. Do not use.");
    return 0;
}

} // namespace Mustard::ROOTX::Math
