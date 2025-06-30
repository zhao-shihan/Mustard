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

#pragma once

#include "Mustard/Math/Random/Distribution/Gaussian.h++"
#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/UniformPseudoRandomBitGenerator.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "TRandom.h"

#include <algorithm>
#include <cstdio>

namespace Mustard::inline Extension::ROOTX::Math {

/// @brief A wrapper class that wraps a PRBG as a TRandom.
/// @tparam PRBG A uniform pseudo random bit generator.
/// @warning DO NOT USE any member involves TRandom::fSeed. GetSeed,
template<Mustard::Math::Random::UniformPseudoRandomBitGenerator PRBG>
class AsTRandom : public TRandom {
public:
    AsTRandom() = default;
    AsTRandom(typename PRBG::SeedType seed);

    virtual ~AsTRandom() override = default;

    virtual auto Gaus(Double_t mu = 0, Double_t sigma = 1) -> Double_t override { return fGaussian(fPRBG, {mu, sigma}); }

    virtual auto SetSeed(ULong_t seed) -> void override { fPRBG.Seed(seed); }
    virtual auto Rndm() -> Double_t override { return Mustard::Math::Random::Uniform<Double_t>{}(fPRBG); }
    virtual auto RndmArray(Int_t n, Float_t* array) -> void override;
    virtual auto RndmArray(Int_t n, Double_t* array) -> void override;

private:
    virtual auto GetSeed() const -> UInt_t override;

private:
    PRBG fPRBG;
    Mustard::Math::Random::Gaussian<Double_t> fGaussian;
};

} // namespace Mustard::inline Extension::ROOTX::Math

#include "Mustard/Extension/ROOTX/Math/AsTRandom.inl"
