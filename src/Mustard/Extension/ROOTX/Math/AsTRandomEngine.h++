#pragma once

#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/UniformPseudoRandomBitGenerator.h++"

#include "Math/TRandomEngine.h"

#include <typeinfo>

namespace Mustard::inline Extension::ROOTX::Math {

template<Mustard::Math::Random::UniformPseudoRandomBitGenerator PRBG>
class AsTRandomEngine : public ROOT::Math::TRandomEngine {
public:
    AsTRandomEngine() = default;
    explicit AsTRandomEngine(typename PRBG::SeedType seed);

    virtual ~AsTRandomEngine() override = default;

    auto Rndm() -> double override final { Mustard::Math::Random::Uniform<double>{}(fPRBG); }

    auto operator()() { return Rndm(); }
    auto IntRndm() { return fPRBG(); }
    auto SetSeed(typename PRBG::SeedType seed) -> void { fPRBG.Seed(seed); }

    static auto MaxInt() { return PRBG::Max(); }
    static auto MinInt() { return PRBG::Min(); }
    static auto Name() -> std::string { return typeid(PRBG).name(); }

private:
    PRBG fPRBG;
};

} // namespace Mustard::inline Extension::ROOTX::Math

#include "Mustard/Extension/ROOTX/Math/AsTRandomEngine.inl"
