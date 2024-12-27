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

#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/UniformPseudoRandomBitGenerator.h++"

#include "Math/TRandomEngine.h"

#include "muc/utility"

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
    static auto Name() -> std::string { return muc::try_demangle(typeid(PRBG).name()); }

private:
    PRBG fPRBG;
};

} // namespace Mustard::inline Extension::ROOTX::Math

#include "Mustard/Extension/ROOTX/Math/AsTRandomEngine.inl"
