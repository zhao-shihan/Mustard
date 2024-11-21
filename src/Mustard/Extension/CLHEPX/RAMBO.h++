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

#include "Mustard/Utility/PrettyLog.h++"

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/Random.h"
#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "CLHEP/Vector/ThreeVector.h"

#include "muc/math"
#include "muc/numeric"

#include "fmt/core.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <utility>

namespace Mustard::inline Extension::CLHEPX {

template<int N>
    requires(N >= 2)
class RAMBO {
public:
    constexpr RAMBO(double eCM, std::array<double, N> mass);

public:
    using State = std::array<CLHEP::HepLorentzVector, N>;
    struct Event {
        double weight;
        State state;
    };

public:
    auto operator()(const std::array<double, 4 * N>& u) const -> Event;
    auto operator()(CLHEP::HepRandomEngine& rng) const -> Event;
    auto operator()() const -> Event { return (*this)(*CLHEP::HepRandom::getTheEngine()); }

    auto operator()(const std::array<double, 4 * N>& u, const CLHEP::Hep3Vector& beta) const -> Event;
    auto operator()(CLHEP::HepRandomEngine& rng, const CLHEP::Hep3Vector& beta) const -> Event;
    auto operator()(const CLHEP::Hep3Vector& beta) const -> Event { return (*this)(*CLHEP::HepRandom::getTheEngine(), beta); }

private:
    double fECM;
    std::array<double, N> fMass;
    bool fAllMassAreTiny;

    static constexpr auto fgTiny{1e-9};
};

} // namespace Mustard::inline Extension::CLHEPX

#include "Mustard/Extension/CLHEPX/RAMBO.inl"
