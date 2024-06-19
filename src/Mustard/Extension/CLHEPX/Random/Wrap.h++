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

#include "Mustard/Env/Print.h++"
#include "Mustard/Math/Parity.h++"
#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/UniformPseudoRandomBitGenerator.h++"

#include "CLHEP/Random/RandomEngine.h"

#include "gsl/gsl"

#include <cstdio>
#include <fstream>
#include <limits>
#include <string>
#include <string_view>
#include <typeinfo>

namespace Mustard::inline Extension::CLHEPX::Random {

template<Math::Random::UniformPseudoRandomBitGenerator PRBG>
class Wrap : public CLHEP::HepRandomEngine {
public:
    Wrap();
    explicit Wrap(long seed);

    ~Wrap() = default;

    virtual auto flat() -> double override { return Math::Random::Uniform<double>{}(fPRBG); }
    virtual auto flatArray(const int size, double* vect) -> void override;

    virtual auto setSeed(long seed, int = 0) -> void override;
    virtual auto setSeeds(const long* seeds, int = 0) -> void override;

    virtual auto saveStatus(gsl::czstring filename) const -> void override;
    virtual auto restoreStatus(gsl::czstring filename) -> void override;
    virtual auto showStatus() const -> void override;

    virtual auto name() const -> std::string override { return typeid(Wrap).name(); }

    virtual auto put(std::ostream& os) const -> decltype(os) override;
    virtual auto get(std::istream& is) -> decltype(is) override;
    /// @brief Do not use.
    virtual auto getState(std::istream& is) -> decltype(is) override;

    virtual operator double() override { return Math::Random::Uniform<double>()(fPRBG); }
    virtual operator float() override { return Math::Random::Uniform<float>()(fPRBG); }
    virtual operator unsigned int() override { return fPRBG(); }

private:
    PRBG fPRBG;
};

} // namespace Mustard::inline Extension::CLHEPX::Random

#include "Mustard/Extension/CLHEPX/Random/Wrap.inl"
