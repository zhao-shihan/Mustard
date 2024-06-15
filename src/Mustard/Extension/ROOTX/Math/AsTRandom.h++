#pragma once

#include "Mustard/Env/Print.h++"
#include "Mustard/Math/Random/Distribution/Gaussian.h++"
#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/UniformPseudoRandomBitGenerator.h++"

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

    auto Gaus(Double_t mu = 0, Double_t sigma = 1) -> Double_t override final { return fGaussian(fPRBG, {mu, sigma}); }

    auto SetSeed(ULong_t seed) -> void override final { fPRBG.Seed(seed); }
    auto Rndm() -> Double_t override final { return Mustard::Math::Random::Uniform<Double_t>{}(fPRBG); }
    auto RndmArray(Int_t n, Float_t* array) -> void override final;
    auto RndmArray(Int_t n, Double_t* array) -> void override final;

private:
    auto GetSeed() const -> UInt_t override final;

private:
    PRBG fPRBG;
    Mustard::Math::Random::Gaussian<Double_t> fGaussian;
};

} // namespace Mustard::inline Extension::ROOTX::Math

#include "Mustard/Extension/ROOTX/Math/AsTRandom.inl"
