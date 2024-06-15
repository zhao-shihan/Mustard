#pragma once

#include "Mustard/Env/Print.h++"

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/Random.h"
#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "CLHEP/Vector/ThreeVector.h"

#include "muc/math"
#include "muc/numeric"

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
    struct Event {
        double weight;
        std::array<CLHEP::HepLorentzVector, N> state;
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

    static constexpr auto fgTiny = 1e-9;
};

} // namespace Mustard::inline Extension::CLHEPX

#include "Mustard/Extension/CLHEPX/RAMBO.inl"
