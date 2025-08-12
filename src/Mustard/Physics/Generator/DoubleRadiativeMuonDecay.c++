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

#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Physics/Generator/DoubleRadiativeMuonDecay.h++"
#include "Mustard/Utility/PhysicalConstant.h++"

#include "CLHEP/Vector/LorentzVector.h"

#include "muc/math"
#include "muc/utility"

#include "fmt/core.h"

#include <stdexcept>

namespace Mustard::inline Physics::inline Generator {

using namespace PhysicalConstant;

DoubleRadiativeMuonDecay::DoubleRadiativeMuonDecay(std::string_view parent, std::function<auto(const Momenta&)->double> B) :
    MetropolisHastingsGenerator{muon_mass_c2, {}, {}, 0.001, 1000} {
    Parent(parent);
    Mass({electron_mass_c2, 0, 0, 0, 0});
    Bias(std::move(B));
}

auto DoubleRadiativeMuonDecay::Parent(std::string_view parent) -> void {
    if (parent == "mu-") {
        PDGID({11, -12, 14, 22, 22});
    } else if (parent == "mu+") {
        PDGID({-11, 12, -14, 22, 22});
    } else {
        Throw<std::invalid_argument>(fmt::format("Parent should be mu- or mu+, got '{}'", parent));
    }
}

auto DoubleRadiativeMuonDecay::Polarization(CLHEP::Hep3Vector pol) -> void {
    fPolarization = pol;
    BurnInRequired();
}

auto DoubleRadiativeMuonDecay::SquaredAmplitude(const Momenta& momenta) const -> double {
    const CLHEP::HepLorentzVector p1{CMSEnergy()};
    const auto& [p2, p3, p4, p5, p6]{momenta};
    const CLHEP::HepLorentzVector pol1{fPolarization};

    // Adapt from McMule v0.5.0, mudec/mudec_pm2ennggav.f95
    //
    // Copyright 2020-2024  Yannick Ulrich and others (The McMule development team)
    //

    const auto mm2{p1.m2()};
    const auto me2{p2.m2()};
    const auto s12{p1 * p2};
    const auto s15{p1 * p5};
    const auto s16{p1 * p6};
    const auto s25{p2 * p5};
    const auto s26{p2 * p6};
    const auto s56{p5 * p6};

    const auto den1{s25 * (s25 + s26 + s56)};
    const auto den2{s26 * (s25 + s26 + s56)};
    const auto den3{-(s15 * (s15 + s16 - s56))};
    const auto den4{s15 * s26};
    const auto den5{-(s16 * (s15 + s16 - s56))};
    const auto den6{s16 * s25};

    double pm2ennggav{};
    if (not fPolarization.isNear({}, muc::default_tolerance<double>)) {
        pm2ennggav += pol1 * p2 *
                      MSqPolarizedS2n(mm2, me2, s12, s15, s16, s25, s26, s56,
                                      den1, den2, den3, den4, den5, den6);
        pm2ennggav += pol1 * p5 *
                      MSqPolarizedS5n(mm2, me2, s12, s15, s16, s25, s26, s56,
                                      den1, den2, den3, den4, den5, den6);
        pm2ennggav += pol1 * p6 *
                      MSqPolarizedS6n(mm2, me2, s12, s15, s16, s25, s26, s56,
                                      den1, den2, den3, den4, den5, den6);
        pm2ennggav *= std::sqrt(mm2);
    }
    pm2ennggav += MSqUnpolarized(mm2, me2, s12, s15, s16, s25, s26, s56,
                                 den1, den2, den3, den4, den5, den6);
    return -4. / 3 * pm2ennggav;
}

auto DoubleRadiativeMuonDecay::MSqUnpolarized(double mm2, double me2, double s12, double s15, double s16, double s25, double s26, double s56,
                                              double den1, double den2, double den3, double den4, double den5, double den6) -> double {
    using muc::pow;

    // Adapt from McMule v0.5.0, mudec/mudec_pm2ennggav.f95
    //
    // Copyright 2020-2024  Yannick Ulrich and others (The McMule development team)
    //

    const auto if11{
        32 * pow<3>(me2) * mm2 - 24 * pow<3>(me2) * s12 -
        24 * pow<2>(me2) * mm2 * s12 + 16 * pow<2>(me2) * pow<2>(s12) -
        24 * pow<3>(me2) * s15 - 24 * pow<2>(me2) * mm2 * s15 +
        32 * pow<2>(me2) * s12 * s15 + 16 * pow<2>(me2) * pow<2>(s15) -
        24 * pow<3>(me2) * s16 - 24 * pow<2>(me2) * mm2 * s16 +
        32 * pow<2>(me2) * s12 * s16 + 32 * pow<2>(me2) * s15 * s16 +
        16 * pow<2>(me2) * pow<2>(s16) + 48 * pow<2>(me2) * mm2 * s25 -
        36 * pow<2>(me2) * s12 * s25 - 12 * me2 * mm2 * s12 * s25 +
        8 * me2 * pow<2>(s12) * s25 - 36 * pow<2>(me2) * s15 * s25 -
        12 * me2 * mm2 * s15 * s25 + 16 * me2 * s12 * s15 * s25 +
        8 * me2 * pow<2>(s15) * s25 - 32 * pow<2>(me2) * s16 * s25 +
        8 * me2 * s12 * s16 * s25 + 8 * me2 * s15 * s16 * s25 +
        24 * me2 * mm2 * pow<2>(s25) - 18 * me2 * s12 * pow<2>(s25) -
        6 * mm2 * s12 * pow<2>(s25) + 4 * pow<2>(s12) * pow<2>(s25) -
        16 * me2 * s15 * pow<2>(s25) + 4 * s12 * s15 * pow<2>(s25) -
        14 * me2 * s16 * pow<2>(s25) - 6 * mm2 * s16 * pow<2>(s25) +
        8 * s12 * s16 * pow<2>(s25) + 4 * s15 * s16 * pow<2>(s25) +
        4 * pow<2>(s16) * pow<2>(s25) + 4 * mm2 * pow<3>(s25) -
        4 * s12 * pow<3>(s25) - 2 * s15 * pow<3>(s25) - 4 * s16 * pow<3>(s25) +
        32 * pow<2>(me2) * mm2 * s26 - 24 * pow<2>(me2) * s12 * s26 -
        24 * pow<2>(me2) * s15 * s26 - 20 * pow<2>(me2) * s16 * s26 +
        12 * me2 * mm2 * s16 * s26 - 8 * me2 * s12 * s16 * s26 -
        8 * me2 * s15 * s16 * s26 - 8 * me2 * pow<2>(s16) * s26 +
        8 * me2 * mm2 * s25 * s26 - 8 * me2 * s12 * s25 * s26 -
        6 * me2 * s15 * s25 * s26 + 6 * mm2 * s15 * s25 * s26 -
        4 * s12 * s15 * s25 * s26 - 4 * pow<2>(s15) * s25 * s26 -
        4 * s15 * s16 * s25 * s26 + 4 * mm2 * pow<2>(s25) * s26 -
        4 * s12 * pow<2>(s25) * s26 - 4 * s16 * pow<2>(s25) * s26 -
        8 * me2 * mm2 * pow<2>(s26) + 4 * me2 * s12 * pow<2>(s26) +
        4 * me2 * s15 * pow<2>(s26) + 8 * me2 * s16 * pow<2>(s26) +
        2 * s15 * s25 * pow<2>(s26) + 32 * pow<2>(me2) * mm2 * s56 -
        24 * pow<2>(me2) * s12 * s56 - 24 * pow<2>(me2) * s15 * s56 -
        20 * pow<2>(me2) * s16 * s56 + 12 * me2 * mm2 * s16 * s56 -
        8 * me2 * s12 * s16 * s56 - 8 * me2 * s15 * s16 * s56 -
        8 * me2 * pow<2>(s16) * s56 + 16 * me2 * mm2 * s25 * s56 -
        14 * me2 * s12 * s25 * s56 - 6 * mm2 * s12 * s25 * s56 +
        4 * pow<2>(s12) * s25 * s56 - 12 * me2 * s15 * s25 * s56 +
        4 * s12 * s15 * s25 * s56 - 6 * me2 * s16 * s25 * s56 -
        6 * mm2 * s16 * s25 * s56 + 8 * s12 * s16 * s25 * s56 +
        4 * s15 * s16 * s25 * s56 + 4 * pow<2>(s16) * s25 * s56 +
        8 * mm2 * pow<2>(s25) * s56 - 8 * s12 * pow<2>(s25) * s56 -
        4 * s15 * pow<2>(s25) * s56 - 8 * s16 * pow<2>(s25) * s56 -
        16 * me2 * mm2 * s26 * s56 + 8 * me2 * s12 * s26 * s56 +
        8 * me2 * s15 * s26 * s56 + 16 * me2 * s16 * s26 * s56 +
        4 * mm2 * s25 * s26 * s56 - 4 * s12 * s25 * s26 * s56 -
        4 * s16 * s25 * s26 * s56 - 8 * me2 * mm2 * pow<2>(s56) +
        4 * me2 * s12 * pow<2>(s56) + 4 * me2 * s15 * pow<2>(s56) +
        8 * me2 * s16 * pow<2>(s56) + 4 * mm2 * s25 * pow<2>(s56) -
        4 * s12 * s25 * pow<2>(s56) - 2 * s15 * s25 * pow<2>(s56) -
        4 * s16 * s25 * pow<2>(s56)};
    const auto if12{
        32 * pow<3>(me2) * mm2 - 24 * pow<3>(me2) * s12 -
        24 * pow<2>(me2) * mm2 * s12 + 16 * pow<2>(me2) * pow<2>(s12) -
        24 * pow<3>(me2) * s15 - 24 * pow<2>(me2) * mm2 * s15 +
        32 * pow<2>(me2) * s12 * s15 + 16 * pow<2>(me2) * pow<2>(s15) -
        24 * pow<3>(me2) * s16 - 24 * pow<2>(me2) * mm2 * s16 +
        32 * pow<2>(me2) * s12 * s16 + 32 * pow<2>(me2) * s15 * s16 +
        16 * pow<2>(me2) * pow<2>(s16) + 40 * pow<2>(me2) * mm2 * s25 -
        30 * pow<2>(me2) * s12 * s25 - 6 * me2 * mm2 * s12 * s25 +
        4 * me2 * pow<2>(s12) * s25 - 30 * pow<2>(me2) * s15 * s25 -
        6 * me2 * mm2 * s15 * s25 + 8 * me2 * s12 * s15 * s25 +
        4 * me2 * pow<2>(s15) * s25 - 30 * pow<2>(me2) * s16 * s25 -
        6 * me2 * mm2 * s16 * s25 + 8 * me2 * s12 * s16 * s25 +
        8 * me2 * s15 * s16 * s25 + 4 * me2 * pow<2>(s16) * s25 +
        8 * me2 * mm2 * pow<2>(s25) - 6 * me2 * s12 * pow<2>(s25) -
        4 * me2 * s15 * pow<2>(s25) + 6 * mm2 * s15 * pow<2>(s25) -
        4 * s12 * s15 * pow<2>(s25) - 4 * pow<2>(s15) * pow<2>(s25) -
        4 * me2 * s16 * pow<2>(s25) + 6 * mm2 * s16 * pow<2>(s25) -
        4 * s12 * s16 * pow<2>(s25) - 8 * s15 * s16 * pow<2>(s25) -
        4 * pow<2>(s16) * pow<2>(s25) - 4 * mm2 * pow<3>(s25) +
        2 * s12 * pow<3>(s25) + 4 * s15 * pow<3>(s25) + 4 * s16 * pow<3>(s25) +
        40 * pow<2>(me2) * mm2 * s26 - 30 * pow<2>(me2) * s12 * s26 -
        6 * me2 * mm2 * s12 * s26 + 4 * me2 * pow<2>(s12) * s26 -
        30 * pow<2>(me2) * s15 * s26 - 6 * me2 * mm2 * s15 * s26 +
        8 * me2 * s12 * s15 * s26 + 4 * me2 * pow<2>(s15) * s26 -
        30 * pow<2>(me2) * s16 * s26 - 6 * me2 * mm2 * s16 * s26 +
        8 * me2 * s12 * s16 * s26 + 8 * me2 * s15 * s16 * s26 +
        4 * me2 * pow<2>(s16) * s26 + 16 * me2 * mm2 * s25 * s26 -
        12 * me2 * s12 * s25 * s26 - 8 * me2 * s15 * s25 * s26 +
        12 * mm2 * s15 * s25 * s26 - 8 * s12 * s15 * s25 * s26 -
        8 * pow<2>(s15) * s25 * s26 - 8 * me2 * s16 * s25 * s26 +
        12 * mm2 * s16 * s25 * s26 - 8 * s12 * s16 * s25 * s26 -
        16 * s15 * s16 * s25 * s26 - 8 * pow<2>(s16) * s25 * s26 -
        12 * mm2 * pow<2>(s25) * s26 + 6 * s12 * pow<2>(s25) * s26 +
        12 * s15 * pow<2>(s25) * s26 + 12 * s16 * pow<2>(s25) * s26 +
        8 * me2 * mm2 * pow<2>(s26) - 6 * me2 * s12 * pow<2>(s26) -
        4 * me2 * s15 * pow<2>(s26) + 6 * mm2 * s15 * pow<2>(s26) -
        4 * s12 * s15 * pow<2>(s26) - 4 * pow<2>(s15) * pow<2>(s26) -
        4 * me2 * s16 * pow<2>(s26) + 6 * mm2 * s16 * pow<2>(s26) -
        4 * s12 * s16 * pow<2>(s26) - 8 * s15 * s16 * pow<2>(s26) -
        4 * pow<2>(s16) * pow<2>(s26) - 12 * mm2 * s25 * pow<2>(s26) +
        6 * s12 * s25 * pow<2>(s26) + 12 * s15 * s25 * pow<2>(s26) +
        12 * s16 * s25 * pow<2>(s26) - 4 * mm2 * pow<3>(s26) +
        2 * s12 * pow<3>(s26) + 4 * s15 * pow<3>(s26) + 4 * s16 * pow<3>(s26) +
        32 * pow<2>(me2) * mm2 * s56 - 24 * pow<2>(me2) * s12 * s56 -
        24 * pow<2>(me2) * s15 * s56 - 24 * pow<2>(me2) * s16 * s56 +
        16 * me2 * mm2 * s25 * s56 - 12 * me2 * s12 * s25 * s56 -
        6 * mm2 * s12 * s25 * s56 + 4 * pow<2>(s12) * s25 * s56 -
        8 * me2 * s15 * s25 * s56 + 6 * mm2 * s15 * s25 * s56 -
        4 * pow<2>(s15) * s25 * s56 - 8 * me2 * s16 * s25 * s56 +
        6 * mm2 * s16 * s25 * s56 - 8 * s15 * s16 * s25 * s56 -
        4 * pow<2>(s16) * s25 * s56 - 8 * mm2 * pow<2>(s25) * s56 +
        2 * s12 * pow<2>(s25) * s56 + 8 * s15 * pow<2>(s25) * s56 +
        8 * s16 * pow<2>(s25) * s56 + 16 * me2 * mm2 * s26 * s56 -
        12 * me2 * s12 * s26 * s56 - 6 * mm2 * s12 * s26 * s56 +
        4 * pow<2>(s12) * s26 * s56 - 8 * me2 * s15 * s26 * s56 +
        6 * mm2 * s15 * s26 * s56 - 4 * pow<2>(s15) * s26 * s56 -
        8 * me2 * s16 * s26 * s56 + 6 * mm2 * s16 * s26 * s56 -
        8 * s15 * s16 * s26 * s56 - 4 * pow<2>(s16) * s26 * s56 -
        16 * mm2 * s25 * s26 * s56 + 4 * s12 * s25 * s26 * s56 +
        16 * s15 * s25 * s26 * s56 + 16 * s16 * s25 * s26 * s56 -
        8 * mm2 * pow<2>(s26) * s56 + 2 * s12 * pow<2>(s26) * s56 +
        8 * s15 * pow<2>(s26) * s56 + 8 * s16 * pow<2>(s26) * s56 +
        8 * me2 * mm2 * pow<2>(s56) - 6 * me2 * s12 * pow<2>(s56) -
        6 * mm2 * s12 * pow<2>(s56) + 4 * pow<2>(s12) * pow<2>(s56) -
        4 * me2 * s15 * pow<2>(s56) + 4 * s12 * s15 * pow<2>(s56) -
        4 * me2 * s16 * pow<2>(s56) + 4 * s12 * s16 * pow<2>(s56) -
        4 * mm2 * s25 * pow<2>(s56) - 2 * s12 * s25 * pow<2>(s56) +
        4 * s15 * s25 * pow<2>(s56) + 4 * s16 * s25 * pow<2>(s56) -
        4 * mm2 * s26 * pow<2>(s56) - 2 * s12 * s26 * pow<2>(s56) +
        4 * s15 * s26 * pow<2>(s56) + 4 * s16 * s26 * pow<2>(s56) -
        2 * s12 * pow<3>(s56)};
    const auto if22{
        32 * pow<3>(me2) * mm2 - 24 * pow<3>(me2) * s12 -
        24 * pow<2>(me2) * mm2 * s12 + 16 * pow<2>(me2) * pow<2>(s12) -
        24 * pow<3>(me2) * s15 - 24 * pow<2>(me2) * mm2 * s15 +
        32 * pow<2>(me2) * s12 * s15 + 16 * pow<2>(me2) * pow<2>(s15) -
        24 * pow<3>(me2) * s16 - 24 * pow<2>(me2) * mm2 * s16 +
        32 * pow<2>(me2) * s12 * s16 + 32 * pow<2>(me2) * s15 * s16 +
        16 * pow<2>(me2) * pow<2>(s16) + 32 * pow<2>(me2) * mm2 * s25 -
        24 * pow<2>(me2) * s12 * s25 - 20 * pow<2>(me2) * s15 * s25 +
        12 * me2 * mm2 * s15 * s25 - 8 * me2 * s12 * s15 * s25 -
        8 * me2 * pow<2>(s15) * s25 - 24 * pow<2>(me2) * s16 * s25 -
        8 * me2 * s15 * s16 * s25 - 8 * me2 * mm2 * pow<2>(s25) +
        4 * me2 * s12 * pow<2>(s25) + 8 * me2 * s15 * pow<2>(s25) +
        4 * me2 * s16 * pow<2>(s25) + 48 * pow<2>(me2) * mm2 * s26 -
        36 * pow<2>(me2) * s12 * s26 - 12 * me2 * mm2 * s12 * s26 +
        8 * me2 * pow<2>(s12) * s26 - 32 * pow<2>(me2) * s15 * s26 +
        8 * me2 * s12 * s15 * s26 - 36 * pow<2>(me2) * s16 * s26 -
        12 * me2 * mm2 * s16 * s26 + 16 * me2 * s12 * s16 * s26 +
        8 * me2 * s15 * s16 * s26 + 8 * me2 * pow<2>(s16) * s26 +
        8 * me2 * mm2 * s25 * s26 - 8 * me2 * s12 * s25 * s26 -
        6 * me2 * s16 * s25 * s26 + 6 * mm2 * s16 * s25 * s26 -
        4 * s12 * s16 * s25 * s26 - 4 * s15 * s16 * s25 * s26 -
        4 * pow<2>(s16) * s25 * s26 + 2 * s16 * pow<2>(s25) * s26 +
        24 * me2 * mm2 * pow<2>(s26) - 18 * me2 * s12 * pow<2>(s26) -
        6 * mm2 * s12 * pow<2>(s26) + 4 * pow<2>(s12) * pow<2>(s26) -
        14 * me2 * s15 * pow<2>(s26) - 6 * mm2 * s15 * pow<2>(s26) +
        8 * s12 * s15 * pow<2>(s26) + 4 * pow<2>(s15) * pow<2>(s26) -
        16 * me2 * s16 * pow<2>(s26) + 4 * s12 * s16 * pow<2>(s26) +
        4 * s15 * s16 * pow<2>(s26) + 4 * mm2 * s25 * pow<2>(s26) -
        4 * s12 * s25 * pow<2>(s26) - 4 * s15 * s25 * pow<2>(s26) +
        4 * mm2 * pow<3>(s26) - 4 * s12 * pow<3>(s26) - 4 * s15 * pow<3>(s26) -
        2 * s16 * pow<3>(s26) + 32 * pow<2>(me2) * mm2 * s56 -
        24 * pow<2>(me2) * s12 * s56 - 20 * pow<2>(me2) * s15 * s56 +
        12 * me2 * mm2 * s15 * s56 - 8 * me2 * s12 * s15 * s56 -
        8 * me2 * pow<2>(s15) * s56 - 24 * pow<2>(me2) * s16 * s56 -
        8 * me2 * s15 * s16 * s56 - 16 * me2 * mm2 * s25 * s56 +
        8 * me2 * s12 * s25 * s56 + 16 * me2 * s15 * s25 * s56 +
        8 * me2 * s16 * s25 * s56 + 16 * me2 * mm2 * s26 * s56 -
        14 * me2 * s12 * s26 * s56 - 6 * mm2 * s12 * s26 * s56 +
        4 * pow<2>(s12) * s26 * s56 - 6 * me2 * s15 * s26 * s56 -
        6 * mm2 * s15 * s26 * s56 + 8 * s12 * s15 * s26 * s56 +
        4 * pow<2>(s15) * s26 * s56 - 12 * me2 * s16 * s26 * s56 +
        4 * s12 * s16 * s26 * s56 + 4 * s15 * s16 * s26 * s56 +
        4 * mm2 * s25 * s26 * s56 - 4 * s12 * s25 * s26 * s56 -
        4 * s15 * s25 * s26 * s56 + 8 * mm2 * pow<2>(s26) * s56 -
        8 * s12 * pow<2>(s26) * s56 - 8 * s15 * pow<2>(s26) * s56 -
        4 * s16 * pow<2>(s26) * s56 - 8 * me2 * mm2 * pow<2>(s56) +
        4 * me2 * s12 * pow<2>(s56) + 8 * me2 * s15 * pow<2>(s56) +
        4 * me2 * s16 * pow<2>(s56) + 4 * mm2 * s26 * pow<2>(s56) -
        4 * s12 * s26 * pow<2>(s56) - 4 * s15 * s26 * pow<2>(s56) -
        2 * s16 * s26 * pow<2>(s56)};
    const auto if13{
        -8 * me2 * mm2 * pow<2>(s12) + 6 * me2 * pow<3>(s12) +
        6 * mm2 * pow<3>(s12) - 4 * pow<4>(s12) - 8 * pow<2>(me2) * mm2 * s15 +
        12 * pow<2>(me2) * s12 * s15 - 4 * me2 * mm2 * s12 * s15 +
        6 * me2 * pow<2>(s12) * s15 + 12 * mm2 * pow<2>(s12) * s15 -
        12 * pow<3>(s12) * s15 + 6 * pow<2>(me2) * pow<2>(s15) -
        2 * me2 * mm2 * pow<2>(s15) + 2 * me2 * s12 * pow<2>(s15) +
        6 * mm2 * s12 * pow<2>(s15) - 12 * pow<2>(s12) * pow<2>(s15) +
        2 * me2 * pow<3>(s15) - 4 * s12 * pow<3>(s15) +
        6 * pow<2>(me2) * s12 * s16 - 2 * me2 * mm2 * s12 * s16 +
        4 * me2 * pow<2>(s12) * s16 + 6 * mm2 * pow<2>(s12) * s16 -
        8 * pow<3>(s12) * s16 + 8 * pow<2>(me2) * s15 * s16 -
        2 * me2 * s12 * s15 * s16 + 6 * mm2 * s12 * s15 * s16 -
        16 * pow<2>(s12) * s15 * s16 - 8 * s12 * pow<2>(s15) * s16 -
        4 * pow<2>(s12) * pow<2>(s16) - 2 * me2 * s15 * pow<2>(s16) -
        4 * s12 * s15 * pow<2>(s16) + 8 * me2 * pow<2>(mm2) * s25 +
        4 * me2 * mm2 * s12 * s25 - 12 * pow<2>(mm2) * s12 * s25 -
        12 * me2 * pow<2>(s12) * s25 - 6 * mm2 * pow<2>(s12) * s25 +
        12 * pow<3>(s12) * s25 - 6 * pow<2>(me2) * s15 * s25 -
        12 * me2 * mm2 * s15 * s25 - 6 * pow<2>(mm2) * s15 * s25 +
        22 * pow<2>(s12) * s15 * s25 + 4 * me2 * pow<2>(s15) * s25 +
        4 * mm2 * pow<2>(s15) * s25 + 10 * s12 * pow<2>(s15) * s25 -
        3 * pow<2>(me2) * s16 * s25 - 6 * me2 * mm2 * s16 * s25 -
        3 * pow<2>(mm2) * s16 * s25 + 6 * mm2 * s12 * s16 * s25 +
        14 * pow<2>(s12) * s16 * s25 + 8 * me2 * s15 * s16 * s25 +
        5 * mm2 * s15 * s16 * s25 + 14 * s12 * s15 * s16 * s25 +
        me2 * pow<2>(s16) * s25 + 2 * mm2 * pow<2>(s16) * s25 +
        4 * s12 * pow<2>(s16) * s25 - 2 * me2 * mm2 * pow<2>(s25) +
        6 * pow<2>(mm2) * pow<2>(s25) + 6 * me2 * s12 * pow<2>(s25) +
        2 * mm2 * s12 * pow<2>(s25) - 12 * pow<2>(s12) * pow<2>(s25) -
        4 * me2 * s15 * pow<2>(s25) - 4 * mm2 * s15 * pow<2>(s25) -
        10 * s12 * s15 * pow<2>(s25) - 3 * me2 * s16 * pow<2>(s25) -
        4 * mm2 * s16 * pow<2>(s25) - 6 * s12 * s16 * pow<2>(s25) -
        2 * mm2 * pow<3>(s25) + 4 * s12 * pow<3>(s25) +
        2 * me2 * mm2 * s12 * s26 - 6 * pow<2>(mm2) * s12 * s26 -
        6 * me2 * pow<2>(s12) * s26 - 4 * mm2 * pow<2>(s12) * s26 +
        8 * pow<3>(s12) * s26 - 3 * pow<2>(me2) * s15 * s26 -
        6 * me2 * mm2 * s15 * s26 - 3 * pow<2>(mm2) * s15 * s26 +
        6 * me2 * s12 * s15 * s26 + 14 * pow<2>(s12) * s15 * s26 +
        4 * me2 * pow<2>(s15) * s26 + 3 * mm2 * pow<2>(s15) * s26 +
        6 * s12 * pow<2>(s15) * s26 + 4 * me2 * s12 * s16 * s26 +
        4 * mm2 * s12 * s16 * s26 + 6 * pow<2>(s12) * s16 * s26 +
        7 * me2 * s15 * s16 * s26 + 2 * mm2 * s15 * s16 * s26 +
        6 * s12 * s15 * s16 * s26 + 8 * pow<2>(mm2) * s25 * s26 +
        6 * me2 * s12 * s25 * s26 - 2 * mm2 * s12 * s25 * s26 -
        16 * pow<2>(s12) * s25 * s26 - 5 * me2 * s15 * s25 * s26 -
        8 * mm2 * s15 * s25 * s26 - 14 * s12 * s15 * s25 * s26 -
        2 * me2 * s16 * s25 * s26 - 7 * mm2 * s16 * s25 * s26 -
        6 * s12 * s16 * s25 * s26 + 8 * s12 * pow<2>(s25) * s26 -
        4 * pow<2>(s12) * pow<2>(s26) - 2 * me2 * s15 * pow<2>(s26) -
        mm2 * s15 * pow<2>(s26) - 4 * s12 * s15 * pow<2>(s26) +
        2 * mm2 * s25 * pow<2>(s26) + 4 * s12 * s25 * pow<2>(s26) +
        8 * pow<2>(me2) * mm2 * s56 + 8 * me2 * pow<2>(mm2) * s56 -
        3 * pow<2>(me2) * s12 * s56 - 2 * me2 * mm2 * s12 * s56 -
        3 * pow<2>(mm2) * s12 * s56 - 10 * me2 * pow<2>(s12) * s56 -
        10 * mm2 * pow<2>(s12) * s56 + 12 * pow<3>(s12) * s56 -
        8 * pow<2>(me2) * s15 * s56 - 10 * me2 * mm2 * s15 * s56 -
        9 * mm2 * s12 * s15 * s56 + 22 * pow<2>(s12) * s15 * s56 +
        2 * me2 * pow<2>(s15) * s56 + 10 * s12 * pow<2>(s15) * s56 -
        6 * me2 * mm2 * s16 * s56 + me2 * s12 * s16 * s56 +
        2 * mm2 * s12 * s16 * s56 + 10 * pow<2>(s12) * s16 * s56 +
        6 * me2 * s15 * s16 * s56 + 10 * s12 * s15 * s16 * s56 +
        10 * me2 * mm2 * s25 * s56 + 8 * pow<2>(mm2) * s25 * s56 +
        9 * me2 * s12 * s25 * s56 - 22 * pow<2>(s12) * s25 * s56 -
        6 * me2 * s15 * s25 * s56 - 6 * mm2 * s15 * s25 * s56 -
        20 * s12 * s15 * s25 * s56 - 2 * me2 * s16 * s25 * s56 -
        6 * mm2 * s16 * s25 * s56 - 10 * s12 * s16 * s25 * s56 +
        2 * mm2 * pow<2>(s25) * s56 + 10 * s12 * pow<2>(s25) * s56 +
        6 * me2 * mm2 * s26 * s56 - 2 * me2 * s12 * s26 * s56 -
        mm2 * s12 * s26 * s56 - 10 * pow<2>(s12) * s26 * s56 -
        6 * me2 * s15 * s26 * s56 - 2 * mm2 * s15 * s26 * s56 -
        10 * s12 * s15 * s26 * s56 + 6 * mm2 * s25 * s26 * s56 +
        10 * s12 * s25 * s26 * s56 + 4 * me2 * mm2 * pow<2>(s56) -
        6 * pow<2>(s12) * pow<2>(s56) - 2 * me2 * s15 * pow<2>(s56) -
        6 * s12 * s15 * pow<2>(s56) + 2 * mm2 * s25 * pow<2>(s56) +
        6 * s12 * s25 * pow<2>(s56)};
    const auto if23{
        -8 * me2 * mm2 * pow<2>(s12) + 6 * me2 * pow<3>(s12) +
        6 * mm2 * pow<3>(s12) - 4 * pow<4>(s12) - 8 * pow<2>(me2) * mm2 * s15 +
        12 * pow<2>(me2) * s12 * s15 + 4 * me2 * mm2 * s12 * s15 +
        6 * mm2 * pow<2>(s12) * s15 - 8 * pow<3>(s12) * s15 +
        6 * pow<2>(me2) * pow<2>(s15) + 2 * me2 * mm2 * pow<2>(s15) -
        6 * me2 * s12 * pow<2>(s15) - 4 * pow<2>(s12) * pow<2>(s15) +
        6 * pow<2>(me2) * s12 * s16 - 10 * me2 * mm2 * s12 * s16 +
        10 * me2 * pow<2>(s12) * s16 + 12 * mm2 * pow<2>(s12) * s16 -
        12 * pow<3>(s12) * s16 + 14 * pow<2>(me2) * s15 * s16 +
        2 * me2 * mm2 * s15 * s16 - 6 * me2 * s12 * s15 * s16 +
        6 * mm2 * s12 * s15 * s16 - 16 * pow<2>(s12) * s15 * s16 -
        4 * me2 * pow<2>(s15) * s16 - 4 * s12 * pow<2>(s15) * s16 +
        6 * pow<2>(me2) * pow<2>(s16) - 2 * me2 * mm2 * pow<2>(s16) +
        4 * me2 * s12 * pow<2>(s16) + 6 * mm2 * s12 * pow<2>(s16) -
        12 * pow<2>(s12) * pow<2>(s16) - 4 * me2 * s15 * pow<2>(s16) -
        8 * s12 * s15 * pow<2>(s16) - 4 * s12 * pow<3>(s16) +
        10 * me2 * mm2 * s12 * s25 - 6 * pow<2>(mm2) * s12 * s25 -
        12 * me2 * pow<2>(s12) * s25 - 10 * mm2 * pow<2>(s12) * s25 +
        12 * pow<3>(s12) * s25 - 6 * pow<2>(me2) * s15 * s25 -
        2 * me2 * mm2 * s15 * s25 + 6 * me2 * s12 * s15 * s25 +
        12 * pow<2>(s12) * s15 * s25 + 4 * me2 * pow<2>(s15) * s25 -
        3 * pow<2>(me2) * s16 * s25 + 10 * me2 * mm2 * s16 * s25 -
        3 * pow<2>(mm2) * s16 * s25 - 10 * me2 * s12 * s16 * s25 -
        10 * mm2 * s12 * s16 * s25 + 24 * pow<2>(s12) * s16 * s25 +
        7 * me2 * s15 * s16 * s25 + mm2 * s15 * s16 * s25 +
        12 * s12 * s15 * s16 * s25 - me2 * pow<2>(s16) * s25 -
        mm2 * pow<2>(s16) * s25 + 14 * s12 * pow<2>(s16) * s25 +
        2 * s15 * pow<2>(s16) * s25 + 2 * pow<3>(s16) * s25 -
        2 * me2 * mm2 * pow<2>(s25) + 6 * pow<2>(mm2) * pow<2>(s25) +
        6 * me2 * s12 * pow<2>(s25) + 4 * mm2 * s12 * pow<2>(s25) -
        12 * pow<2>(s12) * pow<2>(s25) - 4 * me2 * s15 * pow<2>(s25) -
        2 * mm2 * s15 * pow<2>(s25) - 4 * s12 * s15 * pow<2>(s25) +
        me2 * s16 * pow<2>(s25) + mm2 * s16 * pow<2>(s25) -
        14 * s12 * s16 * pow<2>(s25) - 3 * s15 * s16 * pow<2>(s25) -
        5 * pow<2>(s16) * pow<2>(s25) + 4 * s12 * pow<3>(s25) +
        2 * s16 * pow<3>(s25) + 8 * me2 * pow<2>(mm2) * s26 -
        4 * me2 * mm2 * s12 * s26 - 12 * pow<2>(mm2) * s12 * s26 -
        6 * me2 * pow<2>(s12) * s26 + 8 * pow<3>(s12) * s26 -
        3 * pow<2>(me2) * s15 * s26 - 6 * me2 * mm2 * s15 * s26 -
        3 * pow<2>(mm2) * s15 * s26 + 12 * me2 * s12 * s15 * s26 +
        12 * mm2 * s12 * s15 * s26 + 4 * pow<2>(s12) * s15 * s26 +
        3 * me2 * pow<2>(s15) * s26 + 3 * mm2 * pow<2>(s15) * s26 -
        4 * s12 * pow<2>(s15) * s26 - 2 * me2 * mm2 * s16 * s26 -
        6 * pow<2>(mm2) * s16 * s26 + 6 * mm2 * s12 * s16 * s26 +
        12 * pow<2>(s12) * s16 * s26 + 9 * me2 * s15 * s16 * s26 +
        9 * mm2 * s15 * s16 * s26 - 2 * s12 * s15 * s16 * s26 -
        2 * pow<2>(s15) * s16 * s26 + 2 * me2 * pow<2>(s16) * s26 +
        4 * mm2 * pow<2>(s16) * s26 + 4 * s12 * pow<2>(s16) * s26 -
        2 * s15 * pow<2>(s16) * s26 + 2 * me2 * mm2 * s25 * s26 +
        14 * pow<2>(mm2) * s25 * s26 + 6 * me2 * s12 * s25 * s26 -
        6 * mm2 * s12 * s25 * s26 - 16 * pow<2>(s12) * s25 * s26 -
        9 * me2 * s15 * s25 * s26 - 9 * mm2 * s15 * s25 * s26 +
        2 * s12 * s15 * s25 * s26 + pow<2>(s15) * s25 * s26 -
        me2 * s16 * s25 * s26 - 7 * mm2 * s16 * s25 * s26 -
        12 * s12 * s16 * s25 * s26 + 2 * s15 * s16 * s25 * s26 -
        3 * pow<2>(s16) * s25 * s26 + 4 * mm2 * pow<2>(s25) * s26 +
        8 * s12 * pow<2>(s25) * s26 - 2 * s15 * pow<2>(s25) * s26 +
        2 * s16 * pow<2>(s25) * s26 + 2 * me2 * mm2 * pow<2>(s26) +
        6 * pow<2>(mm2) * pow<2>(s26) - 6 * mm2 * s12 * pow<2>(s26) -
        4 * pow<2>(s12) * pow<2>(s26) - 3 * me2 * s15 * pow<2>(s26) -
        3 * mm2 * s15 * pow<2>(s26) + 4 * s12 * s15 * pow<2>(s26) -
        pow<2>(s15) * pow<2>(s26) - 4 * mm2 * s16 * pow<2>(s26) +
        s15 * s16 * pow<2>(s26) + 4 * mm2 * s25 * pow<2>(s26) +
        4 * s12 * s25 * pow<2>(s26) - 2 * s15 * s25 * pow<2>(s26) +
        8 * pow<2>(me2) * mm2 * s56 + 8 * me2 * pow<2>(mm2) * s56 -
        3 * pow<2>(me2) * s12 * s56 - 2 * me2 * mm2 * s12 * s56 -
        3 * pow<2>(mm2) * s12 * s56 - 10 * me2 * pow<2>(s12) * s56 -
        10 * mm2 * pow<2>(s12) * s56 + 12 * pow<3>(s12) * s56 -
        8 * pow<2>(me2) * s15 * s56 - 8 * me2 * mm2 * s15 * s56 +
        7 * me2 * s12 * s15 * s56 - mm2 * s12 * s15 * s56 +
        12 * pow<2>(s12) * s15 * s56 + 2 * me2 * pow<2>(s15) * s56 -
        6 * pow<2>(me2) * s16 * s56 - 2 * me2 * mm2 * s16 * s56 -
        7 * me2 * s12 * s16 * s56 - 9 * mm2 * s12 * s16 * s56 +
        22 * pow<2>(s12) * s16 * s56 + 4 * me2 * s15 * s16 * s56 +
        10 * s12 * s15 * s16 * s56 + 10 * s12 * pow<2>(s16) * s56 +
        2 * me2 * mm2 * s25 * s56 + 6 * pow<2>(mm2) * s25 * s56 +
        9 * me2 * s12 * s25 * s56 + 7 * mm2 * s12 * s25 * s56 -
        22 * pow<2>(s12) * s25 * s56 - 8 * me2 * s15 * s25 * s56 -
        2 * mm2 * s15 * s25 * s56 - 5 * s12 * s15 * s25 * s56 -
        21 * s12 * s16 * s25 * s56 - 3 * s15 * s16 * s25 * s56 -
        5 * pow<2>(s16) * s25 * s56 + 10 * s12 * pow<2>(s25) * s56 +
        5 * s16 * pow<2>(s25) * s56 + 8 * me2 * mm2 * s26 * s56 +
        8 * pow<2>(mm2) * s26 * s56 + me2 * s12 * s26 * s56 -
        7 * mm2 * s12 * s26 * s56 - 12 * pow<2>(s12) * s26 * s56 -
        8 * me2 * s15 * s26 * s56 - 8 * mm2 * s15 * s26 * s56 +
        7 * s12 * s15 * s26 * s56 + pow<2>(s15) * s26 * s56 -
        2 * me2 * s16 * s26 * s56 - 8 * mm2 * s16 * s26 * s56 -
        5 * s12 * s16 * s26 * s56 + 3 * s15 * s16 * s26 * s56 +
        4 * mm2 * s25 * s26 * s56 + 10 * s12 * s25 * s26 * s56 -
        3 * s15 * s25 * s26 * s56 + 3 * s16 * s25 * s26 * s56 +
        2 * mm2 * pow<2>(s26) * s56 - s15 * pow<2>(s26) * s56 +
        4 * me2 * mm2 * pow<2>(s56) + 4 * me2 * s12 * pow<2>(s56) +
        4 * mm2 * s12 * pow<2>(s56) - 10 * pow<2>(s12) * pow<2>(s56) -
        2 * me2 * s15 * pow<2>(s56) - s12 * s15 * pow<2>(s56) -
        7 * s12 * s16 * pow<2>(s56) + 7 * s12 * s25 * pow<2>(s56) +
        3 * s16 * s25 * pow<2>(s56) + 2 * mm2 * s26 * pow<2>(s56) +
        s12 * s26 * pow<2>(s56) - s15 * s26 * pow<2>(s56) + s12 * pow<3>(s56)};
    const auto if33{
        32 * me2 * pow<3>(mm2) - 24 * me2 * pow<2>(mm2) * s12 -
        24 * pow<3>(mm2) * s12 + 16 * pow<2>(mm2) * pow<2>(s12) -
        48 * me2 * pow<2>(mm2) * s15 + 12 * me2 * mm2 * s12 * s15 +
        36 * pow<2>(mm2) * s12 * s15 - 8 * mm2 * pow<2>(s12) * s15 +
        24 * me2 * mm2 * pow<2>(s15) - 6 * me2 * s12 * pow<2>(s15) -
        18 * mm2 * s12 * pow<2>(s15) + 4 * pow<2>(s12) * pow<2>(s15) -
        4 * me2 * pow<3>(s15) + 4 * s12 * pow<3>(s15) -
        32 * me2 * pow<2>(mm2) * s16 + 24 * pow<2>(mm2) * s12 * s16 +
        8 * me2 * mm2 * s15 * s16 - 8 * mm2 * s12 * s15 * s16 -
        4 * me2 * pow<2>(s15) * s16 + 4 * s12 * pow<2>(s15) * s16 -
        8 * me2 * mm2 * pow<2>(s16) + 4 * mm2 * s12 * pow<2>(s16) +
        24 * me2 * pow<2>(mm2) * s25 + 24 * pow<3>(mm2) * s25 -
        32 * pow<2>(mm2) * s12 * s25 - 12 * me2 * mm2 * s15 * s25 -
        36 * pow<2>(mm2) * s15 * s25 + 16 * mm2 * s12 * s15 * s25 +
        16 * mm2 * pow<2>(s15) * s25 - 4 * s12 * pow<2>(s15) * s25 -
        2 * pow<3>(s15) * s25 - 24 * pow<2>(mm2) * s16 * s25 -
        6 * me2 * s15 * s16 * s25 + 6 * mm2 * s15 * s16 * s25 +
        4 * s12 * s15 * s16 * s25 - 4 * mm2 * pow<2>(s16) * s25 +
        2 * s15 * pow<2>(s16) * s25 + 16 * pow<2>(mm2) * pow<2>(s25) -
        8 * mm2 * s15 * pow<2>(s25) - 4 * s15 * s16 * pow<2>(s25) +
        24 * me2 * pow<2>(mm2) * s26 + 24 * pow<3>(mm2) * s26 -
        32 * pow<2>(mm2) * s12 * s26 - 32 * pow<2>(mm2) * s15 * s26 +
        8 * mm2 * s12 * s15 * s26 + 6 * me2 * pow<2>(s15) * s26 +
        14 * mm2 * pow<2>(s15) * s26 - 8 * s12 * pow<2>(s15) * s26 -
        4 * pow<3>(s15) * s26 + 12 * me2 * mm2 * s16 * s26 -
        20 * pow<2>(mm2) * s16 * s26 - 8 * mm2 * s12 * s16 * s26 -
        4 * pow<2>(s15) * s16 * s26 - 8 * mm2 * pow<2>(s16) * s26 +
        32 * pow<2>(mm2) * s25 * s26 - 8 * mm2 * s15 * s25 * s26 +
        4 * pow<2>(s15) * s25 * s26 + 8 * mm2 * s16 * s25 * s26 -
        4 * s15 * s16 * s25 * s26 + 16 * pow<2>(mm2) * pow<2>(s26) +
        4 * pow<2>(s15) * pow<2>(s26) + 8 * mm2 * s16 * pow<2>(s26) +
        32 * me2 * pow<2>(mm2) * s56 - 24 * pow<2>(mm2) * s12 * s56 -
        16 * me2 * mm2 * s15 * s56 + 6 * me2 * s12 * s15 * s56 +
        14 * mm2 * s12 * s15 * s56 - 4 * pow<2>(s12) * s15 * s56 +
        8 * me2 * pow<2>(s15) * s56 - 8 * s12 * pow<2>(s15) * s56 +
        16 * me2 * mm2 * s16 * s56 - 8 * mm2 * s12 * s16 * s56 +
        4 * me2 * s15 * s16 * s56 - 4 * s12 * s15 * s16 * s56 +
        24 * pow<2>(mm2) * s25 * s56 - 12 * mm2 * s15 * s25 * s56 +
        4 * s12 * s15 * s25 * s56 + 4 * pow<2>(s15) * s25 * s56 +
        8 * mm2 * s16 * s25 * s56 - 12 * me2 * mm2 * s26 * s56 +
        20 * pow<2>(mm2) * s26 * s56 + 8 * mm2 * s12 * s26 * s56 -
        6 * me2 * s15 * s26 * s56 - 6 * mm2 * s15 * s26 * s56 +
        8 * s12 * s15 * s26 * s56 + 8 * pow<2>(s15) * s26 * s56 +
        16 * mm2 * s16 * s26 * s56 + 4 * s15 * s16 * s26 * s56 -
        8 * mm2 * s25 * s26 * s56 - 4 * s15 * s25 * s26 * s56 -
        8 * mm2 * pow<2>(s26) * s56 - 4 * s15 * pow<2>(s26) * s56 -
        8 * me2 * mm2 * pow<2>(s56) + 4 * mm2 * s12 * pow<2>(s56) -
        4 * me2 * s15 * pow<2>(s56) + 4 * s12 * s15 * pow<2>(s56) -
        4 * mm2 * s25 * pow<2>(s56) - 2 * s15 * s25 * pow<2>(s56) -
        8 * mm2 * s26 * pow<2>(s56) - 4 * s15 * s26 * pow<2>(s56)};
    const auto if14{
        -16 * pow<2>(me2) * mm2 * s12 + 12 * pow<2>(me2) * pow<2>(s12) +
        12 * me2 * mm2 * pow<2>(s12) - 8 * me2 * pow<3>(s12) +
        12 * pow<3>(me2) * s15 - 4 * pow<2>(me2) * mm2 * s15 +
        8 * pow<2>(me2) * s12 * s15 + 12 * me2 * mm2 * s12 * s15 -
        16 * me2 * pow<2>(s12) * s15 - 8 * me2 * s12 * pow<2>(s15) +
        12 * pow<2>(me2) * s12 * s16 + 12 * me2 * mm2 * s12 * s16 -
        16 * me2 * pow<2>(s12) * s16 - 2 * pow<2>(me2) * s15 * s16 +
        6 * me2 * mm2 * s15 * s16 - 20 * me2 * s12 * s15 * s16 -
        4 * me2 * pow<2>(s15) * s16 - 8 * me2 * s12 * pow<2>(s16) -
        4 * me2 * s15 * pow<2>(s16) + 8 * pow<2>(me2) * mm2 * s25 -
        12 * pow<2>(me2) * s12 * s25 - 20 * me2 * mm2 * s12 * s25 +
        18 * me2 * pow<2>(s12) * s25 + 12 * pow<2>(me2) * s15 * s25 -
        8 * me2 * mm2 * s15 * s25 + 18 * me2 * s12 * s15 * s25 +
        6 * mm2 * s12 * s15 * s25 - 4 * pow<2>(s12) * s15 * s25 +
        2 * me2 * pow<2>(s15) * s25 - 4 * s12 * pow<2>(s15) * s25 -
        6 * pow<2>(me2) * s16 * s25 - 10 * me2 * mm2 * s16 * s25 +
        24 * me2 * s12 * s16 * s25 + 10 * me2 * s15 * s16 * s25 +
        3 * mm2 * s15 * s16 * s25 - 6 * s12 * s15 * s16 * s25 -
        2 * pow<2>(s15) * s16 * s25 + 7 * me2 * pow<2>(s16) * s25 +
        3 * mm2 * pow<2>(s16) * s25 - 2 * s12 * pow<2>(s16) * s25 -
        4 * s15 * pow<2>(s16) * s25 - 2 * pow<3>(s16) * s25 +
        6 * me2 * mm2 * pow<2>(s25) - 6 * pow<2>(mm2) * pow<2>(s25) -
        10 * me2 * s12 * pow<2>(s25) + 4 * mm2 * s12 * pow<2>(s25) +
        2 * me2 * s15 * pow<2>(s25) + 4 * mm2 * s15 * pow<2>(s25) +
        2 * s12 * s15 * pow<2>(s25) - 6 * me2 * s16 * pow<2>(s25) +
        3 * mm2 * s16 * pow<2>(s25) + 2 * s15 * s16 * pow<2>(s25) +
        2 * pow<2>(s16) * pow<2>(s25) - 2 * mm2 * pow<3>(s25) -
        16 * me2 * mm2 * s12 * s26 + 12 * me2 * pow<2>(s12) * s26 +
        18 * pow<2>(me2) * s15 * s26 - 6 * me2 * mm2 * s15 * s26 +
        12 * me2 * s12 * s15 * s26 + 2 * me2 * pow<2>(s15) * s26 -
        3 * mm2 * pow<2>(s15) * s26 + 2 * s12 * pow<2>(s15) * s26 +
        2 * pow<3>(s15) * s26 + 10 * me2 * s12 * s16 * s26 -
        6 * mm2 * s12 * s16 * s26 + 4 * pow<2>(s12) * s16 * s26 +
        3 * me2 * s15 * s16 * s26 - 3 * mm2 * s15 * s16 * s26 +
        6 * s12 * s15 * s16 * s26 + 4 * pow<2>(s15) * s16 * s26 +
        4 * s12 * pow<2>(s16) * s26 + 2 * s15 * pow<2>(s16) * s26 +
        6 * me2 * mm2 * s25 * s26 - 6 * pow<2>(mm2) * s25 * s26 -
        12 * me2 * s12 * s25 * s26 + 8 * mm2 * s12 * s25 * s26 -
        2 * pow<2>(s12) * s25 * s26 + 6 * me2 * s15 * s25 * s26 +
        7 * mm2 * s15 * s25 * s26 - 2 * s12 * s15 * s25 * s26 -
        2 * pow<2>(s15) * s25 * s26 - 5 * me2 * s16 * s25 * s26 +
        5 * mm2 * s16 * s25 * s26 - 6 * s12 * s16 * s25 * s26 -
        2 * s15 * s16 * s25 * s26 - 6 * mm2 * pow<2>(s25) * s26 +
        2 * s12 * pow<2>(s25) * s26 + 2 * s16 * pow<2>(s25) * s26 +
        4 * mm2 * s12 * pow<2>(s26) - 2 * pow<2>(s12) * pow<2>(s26) +
        3 * me2 * s15 * pow<2>(s26) + mm2 * s15 * pow<2>(s26) -
        2 * s12 * s15 * pow<2>(s26) - 4 * s12 * s16 * pow<2>(s26) -
        4 * mm2 * s25 * pow<2>(s26) + 2 * s12 * s25 * pow<2>(s26) -
        2 * s15 * s25 * pow<2>(s26) + 2 * s16 * s25 * pow<2>(s26) -
        2 * s15 * pow<3>(s26) - 6 * pow<2>(me2) * s12 * s56 -
        14 * me2 * mm2 * s12 * s56 + 14 * me2 * pow<2>(s12) * s56 +
        10 * pow<2>(me2) * s15 * s56 - 8 * me2 * mm2 * s15 * s56 +
        16 * me2 * s12 * s15 * s56 + 3 * mm2 * s12 * s15 * s56 -
        2 * pow<2>(s12) * s15 * s56 + 4 * me2 * pow<2>(s15) * s56 -
        2 * s12 * pow<2>(s15) * s56 + 13 * me2 * s12 * s16 * s56 -
        3 * mm2 * s12 * s16 * s56 + 2 * pow<2>(s12) * s16 * s56 +
        8 * me2 * s15 * s16 * s56 + 2 * s12 * pow<2>(s16) * s56 +
        8 * me2 * mm2 * s25 * s56 - 6 * pow<2>(mm2) * s25 * s56 -
        16 * me2 * s12 * s25 * s56 + 5 * mm2 * s12 * s25 * s56 +
        2 * me2 * s15 * s25 * s56 + 4 * mm2 * s15 * s25 * s56 +
        2 * s12 * s15 * s25 * s56 - 7 * me2 * s16 * s25 * s56 +
        2 * mm2 * s16 * s25 * s56 - 2 * s12 * s16 * s25 * s56 +
        2 * s15 * s16 * s25 * s56 + 2 * pow<2>(s16) * s25 * s56 -
        4 * mm2 * pow<2>(s25) * s56 - 5 * me2 * s12 * s26 * s56 +
        7 * mm2 * s12 * s26 * s56 - 4 * pow<2>(s12) * s26 * s56 +
        3 * me2 * s15 * s26 * s56 + 2 * mm2 * s15 * s26 * s56 -
        6 * s12 * s15 * s26 * s56 - 2 * pow<2>(s15) * s26 * s56 -
        8 * s12 * s16 * s26 * s56 - 2 * s15 * s16 * s26 * s56 -
        4 * mm2 * s25 * s26 * s56 + 4 * s12 * s25 * s26 * s56 +
        2 * s16 * s25 * s26 * s56 + 2 * s12 * pow<2>(s26) * s56 -
        2 * s15 * pow<2>(s26) * s56 - 2 * me2 * mm2 * pow<2>(s56) -
        5 * me2 * s12 * pow<2>(s56) + 2 * mm2 * s12 * pow<2>(s56) -
        2 * s12 * s16 * pow<2>(s56) - 2 * mm2 * s25 * pow<2>(s56) +
        2 * s12 * s26 * pow<2>(s56)};
    const auto if24{
        -16 * pow<2>(me2) * mm2 * s12 + 12 * pow<2>(me2) * pow<2>(s12) +
        12 * me2 * mm2 * pow<2>(s12) - 8 * me2 * pow<3>(s12) +
        12 * pow<3>(me2) * s15 - 4 * pow<2>(me2) * mm2 * s15 +
        8 * pow<2>(me2) * s12 * s15 + 12 * me2 * mm2 * s12 * s15 -
        16 * me2 * pow<2>(s12) * s15 - 8 * me2 * s12 * pow<2>(s15) -
        16 * pow<2>(me2) * mm2 * s16 + 24 * pow<2>(me2) * s12 * s16 +
        24 * me2 * mm2 * s12 * s16 - 24 * me2 * pow<2>(s12) * s16 +
        8 * pow<2>(me2) * s15 * s16 + 12 * me2 * mm2 * s15 * s16 -
        32 * me2 * s12 * s15 * s16 - 8 * me2 * pow<2>(s15) * s16 +
        12 * pow<2>(me2) * pow<2>(s16) + 12 * me2 * mm2 * pow<2>(s16) -
        24 * me2 * s12 * pow<2>(s16) - 16 * me2 * s15 * pow<2>(s16) -
        8 * me2 * pow<3>(s16) + 4 * pow<2>(me2) * mm2 * s25 -
        12 * me2 * pow<2>(mm2) * s25 - 12 * pow<2>(me2) * s12 * s25 -
        8 * me2 * mm2 * s12 * s25 + 16 * me2 * pow<2>(s12) * s25 +
        8 * pow<2>(me2) * s15 * s25 + 8 * me2 * mm2 * s15 * s25 +
        12 * me2 * s12 * s15 * s25 - 12 * pow<2>(me2) * s16 * s25 -
        8 * me2 * mm2 * s16 * s25 + 32 * me2 * s12 * s16 * s25 +
        12 * me2 * s15 * s16 * s25 + 16 * me2 * pow<2>(s16) * s25 -
        8 * me2 * s12 * pow<2>(s25) - 8 * me2 * s16 * pow<2>(s25) -
        4 * pow<2>(me2) * mm2 * s26 - 12 * me2 * pow<2>(mm2) * s26 -
        4 * me2 * mm2 * s12 * s26 + 10 * me2 * pow<2>(s12) * s26 +
        20 * pow<2>(me2) * s15 * s26 + 8 * me2 * mm2 * s15 * s26 +
        6 * me2 * s12 * s15 * s26 + 16 * me2 * s12 * s16 * s26 -
        6 * mm2 * s12 * s16 * s26 + 4 * pow<2>(s12) * s16 * s26 +
        4 * me2 * s15 * s16 * s26 - 6 * mm2 * s15 * s16 * s26 +
        8 * s12 * s15 * s16 * s26 + 4 * pow<2>(s15) * s16 * s26 +
        6 * me2 * pow<2>(s16) * s26 - 6 * mm2 * pow<2>(s16) * s26 +
        8 * s12 * pow<2>(s16) * s26 + 8 * s15 * pow<2>(s16) * s26 +
        4 * pow<3>(s16) * s26 - 4 * me2 * mm2 * s25 * s26 -
        10 * me2 * s12 * s25 * s26 + 6 * me2 * s15 * s25 * s26 -
        7 * me2 * s16 * s25 * s26 + 7 * mm2 * s16 * s25 * s26 -
        8 * s12 * s16 * s25 * s26 - 6 * s15 * s16 * s25 * s26 -
        8 * pow<2>(s16) * s25 * s26 + 3 * s16 * pow<2>(s25) * s26 -
        2 * me2 * mm2 * pow<2>(s26) + 6 * pow<2>(mm2) * pow<2>(s26) -
        4 * mm2 * s12 * pow<2>(s26) + 7 * me2 * s15 * pow<2>(s26) -
        5 * mm2 * s15 * pow<2>(s26) - 4 * s12 * s16 * pow<2>(s26) -
        2 * s15 * s16 * pow<2>(s26) - 4 * pow<2>(s16) * pow<2>(s26) +
        4 * mm2 * s25 * pow<2>(s26) - s15 * s25 * pow<2>(s26) +
        3 * s16 * s25 * pow<2>(s26) + 2 * mm2 * pow<3>(s26) -
        s15 * pow<3>(s26) + 4 * pow<2>(me2) * mm2 * s56 -
        12 * me2 * pow<2>(mm2) * s56 - 12 * pow<2>(me2) * s12 * s56 -
        8 * me2 * mm2 * s12 * s56 + 16 * me2 * pow<2>(s12) * s56 +
        8 * pow<2>(me2) * s15 * s56 + 8 * me2 * mm2 * s15 * s56 +
        12 * me2 * s12 * s15 * s56 - 12 * pow<2>(me2) * s16 * s56 -
        8 * me2 * mm2 * s16 * s56 + 32 * me2 * s12 * s16 * s56 +
        12 * me2 * s15 * s16 * s56 + 16 * me2 * pow<2>(s16) * s56 -
        16 * me2 * s12 * s25 * s56 - 16 * me2 * s16 * s25 * s56 -
        6 * me2 * mm2 * s26 * s56 + 6 * pow<2>(mm2) * s26 * s56 -
        9 * me2 * s12 * s26 * s56 - 3 * mm2 * s12 * s26 * s56 +
        6 * me2 * s15 * s26 * s56 - 4 * mm2 * s15 * s26 * s56 -
        6 * me2 * s16 * s26 * s56 + 4 * mm2 * s16 * s26 * s56 -
        8 * s12 * s16 * s26 * s56 - 6 * s15 * s16 * s26 * s56 -
        8 * pow<2>(s16) * s26 * s56 + s12 * s25 * s26 * s56 +
        7 * s16 * s25 * s26 * s56 + 2 * mm2 * pow<2>(s26) * s56 +
        s12 * pow<2>(s26) * s56 - s15 * pow<2>(s26) * s56 +
        4 * s16 * pow<2>(s26) * s56 - 8 * me2 * s12 * pow<2>(s56) -
        8 * me2 * s16 * pow<2>(s56) + s12 * s26 * pow<2>(s56) +
        4 * s16 * s26 * pow<2>(s56)};
    const auto if34{
        16 * me2 * pow<2>(mm2) * s12 - 12 * me2 * mm2 * pow<2>(s12) -
        12 * pow<2>(mm2) * pow<2>(s12) + 8 * mm2 * pow<3>(s12) -
        12 * pow<2>(me2) * mm2 * s15 - 4 * me2 * pow<2>(mm2) * s15 -
        4 * me2 * mm2 * s12 * s15 + 10 * mm2 * pow<2>(s12) * s15 -
        6 * pow<2>(me2) * pow<2>(s15) + 2 * me2 * mm2 * pow<2>(s15) +
        4 * me2 * s12 * pow<2>(s15) + 2 * me2 * pow<3>(s15) -
        12 * pow<2>(me2) * mm2 * s16 + 4 * me2 * pow<2>(mm2) * s16 -
        8 * me2 * mm2 * s12 * s16 - 12 * pow<2>(mm2) * s12 * s16 +
        16 * mm2 * pow<2>(s12) * s16 + 4 * me2 * mm2 * s15 * s16 +
        10 * mm2 * s12 * s15 * s16 + 4 * me2 * pow<2>(s15) * s16 +
        8 * mm2 * s12 * pow<2>(s16) - 16 * me2 * pow<2>(mm2) * s25 +
        24 * me2 * mm2 * s12 * s25 + 24 * pow<2>(mm2) * s12 * s25 -
        24 * mm2 * pow<2>(s12) * s25 + 6 * me2 * s12 * s15 * s25 -
        16 * mm2 * s12 * s15 * s25 - 4 * pow<2>(s12) * s15 * s25 -
        4 * s12 * pow<2>(s15) * s25 + 8 * me2 * mm2 * s16 * s25 +
        12 * pow<2>(mm2) * s16 * s25 - 32 * mm2 * s12 * s16 * s25 +
        7 * me2 * s15 * s16 * s25 - 7 * mm2 * s15 * s16 * s25 -
        8 * s12 * s15 * s16 * s25 - 3 * pow<2>(s15) * s16 * s25 -
        8 * mm2 * pow<2>(s16) * s25 - 3 * s15 * pow<2>(s16) * s25 -
        12 * me2 * mm2 * pow<2>(s25) - 12 * pow<2>(mm2) * pow<2>(s25) +
        24 * mm2 * s12 * pow<2>(s25) - 6 * me2 * s15 * pow<2>(s25) +
        6 * mm2 * s15 * pow<2>(s25) + 8 * s12 * s15 * pow<2>(s25) +
        4 * pow<2>(s15) * pow<2>(s25) + 16 * mm2 * s16 * pow<2>(s25) +
        8 * s15 * s16 * pow<2>(s25) - 8 * mm2 * pow<3>(s25) -
        4 * s15 * pow<3>(s25) - 4 * me2 * pow<2>(mm2) * s26 +
        12 * pow<3>(mm2) * s26 + 12 * me2 * mm2 * s12 * s26 +
        8 * pow<2>(mm2) * s12 * s26 - 16 * mm2 * pow<2>(s12) * s26 -
        8 * me2 * mm2 * s15 * s26 - 20 * pow<2>(mm2) * s15 * s26 -
        6 * mm2 * s12 * s15 * s26 - 5 * me2 * pow<2>(s15) * s26 +
        7 * mm2 * pow<2>(s15) * s26 + pow<3>(s15) * s26 -
        8 * me2 * mm2 * s16 * s26 - 8 * pow<2>(mm2) * s16 * s26 -
        12 * mm2 * s12 * s16 * s26 + 6 * mm2 * s15 * s16 * s26 +
        pow<2>(s15) * s16 * s26 - 12 * me2 * mm2 * s25 * s26 -
        8 * pow<2>(mm2) * s25 * s26 + 32 * mm2 * s12 * s25 * s26 -
        6 * me2 * s15 * s25 * s26 + 4 * mm2 * s15 * s25 * s26 +
        8 * s12 * s15 * s25 * s26 + 2 * pow<2>(s15) * s25 * s26 +
        12 * mm2 * s16 * s25 * s26 + 6 * s15 * s16 * s25 * s26 -
        16 * mm2 * pow<2>(s25) * s26 - 8 * s15 * pow<2>(s25) * s26 +
        8 * mm2 * s12 * pow<2>(s26) - 8 * mm2 * s25 * pow<2>(s26) -
        4 * s15 * s25 * pow<2>(s26) + 12 * pow<2>(me2) * mm2 * s56 -
        4 * me2 * pow<2>(mm2) * s56 + 8 * me2 * mm2 * s12 * s56 +
        12 * pow<2>(mm2) * s12 * s56 - 16 * mm2 * pow<2>(s12) * s56 +
        6 * pow<2>(me2) * s15 * s56 - 6 * me2 * mm2 * s15 * s56 -
        3 * me2 * s12 * s15 * s56 - 9 * mm2 * s12 * s15 * s56 -
        2 * me2 * pow<2>(s15) * s56 - s12 * pow<2>(s15) * s56 -
        16 * mm2 * s12 * s16 * s56 - s12 * s15 * s16 * s56 -
        8 * me2 * mm2 * s25 * s56 - 12 * pow<2>(mm2) * s25 * s56 +
        32 * mm2 * s12 * s25 * s56 - 4 * me2 * s15 * s25 * s56 +
        6 * mm2 * s15 * s25 * s56 + 8 * s12 * s15 * s25 * s56 +
        4 * pow<2>(s15) * s25 * s56 + 16 * mm2 * s16 * s25 * s56 +
        7 * s15 * s16 * s25 * s56 - 16 * mm2 * pow<2>(s25) * s56 -
        8 * s15 * pow<2>(s25) * s56 + 8 * me2 * mm2 * s26 * s56 +
        8 * pow<2>(mm2) * s26 * s56 + 12 * mm2 * s12 * s26 * s56 +
        4 * me2 * s15 * s26 * s56 - 6 * mm2 * s15 * s26 * s56 -
        pow<2>(s15) * s26 * s56 - 12 * mm2 * s25 * s26 * s56 -
        6 * s15 * s25 * s26 * s56 + 8 * mm2 * s12 * pow<2>(s56) +
        s12 * s15 * pow<2>(s56) - 8 * mm2 * s25 * pow<2>(s56) -
        4 * s15 * s25 * pow<2>(s56)};
    const auto if44{
        32 * pow<2>(me2) * pow<2>(mm2) - 24 * pow<2>(me2) * mm2 * s12 -
        24 * me2 * pow<2>(mm2) * s12 + 16 * me2 * mm2 * pow<2>(s12) -
        32 * pow<2>(me2) * mm2 * s15 + 24 * me2 * mm2 * s12 * s15 -
        8 * pow<2>(me2) * pow<2>(s15) + 4 * me2 * s12 * pow<2>(s15) -
        24 * pow<2>(me2) * mm2 * s16 - 24 * me2 * pow<2>(mm2) * s16 +
        32 * me2 * mm2 * s12 * s16 + 24 * me2 * mm2 * s15 * s16 +
        4 * me2 * pow<2>(s15) * s16 + 16 * me2 * mm2 * pow<2>(s16) +
        24 * pow<2>(me2) * mm2 * s25 + 24 * me2 * pow<2>(mm2) * s25 -
        32 * me2 * mm2 * s12 * s25 + 12 * pow<2>(me2) * s15 * s25 -
        20 * me2 * mm2 * s15 * s25 - 8 * me2 * s12 * s15 * s25 -
        8 * me2 * pow<2>(s15) * s25 - 32 * me2 * mm2 * s16 * s25 -
        8 * me2 * s15 * s16 * s25 + 16 * me2 * mm2 * pow<2>(s25) +
        8 * me2 * s15 * pow<2>(s25) + 32 * me2 * pow<2>(mm2) * s26 -
        24 * me2 * mm2 * s12 * s26 - 32 * me2 * mm2 * s15 * s26 -
        8 * me2 * pow<2>(s15) * s26 - 20 * me2 * mm2 * s16 * s26 +
        12 * pow<2>(mm2) * s16 * s26 - 8 * mm2 * s12 * s16 * s26 -
        12 * mm2 * s15 * s16 * s26 - 2 * pow<2>(s15) * s16 * s26 -
        8 * mm2 * pow<2>(s16) * s26 + 24 * me2 * mm2 * s25 * s26 +
        12 * me2 * s15 * s25 * s26 + 8 * mm2 * s16 * s25 * s26 +
        2 * s15 * s16 * s25 * s26 - 8 * pow<2>(mm2) * pow<2>(s26) +
        4 * mm2 * s12 * pow<2>(s26) + 8 * mm2 * s15 * pow<2>(s26) +
        2 * pow<2>(s15) * pow<2>(s26) + 8 * mm2 * s16 * pow<2>(s26) -
        4 * mm2 * s25 * pow<2>(s26) - 2 * s15 * s25 * pow<2>(s26) +
        24 * pow<2>(me2) * mm2 * s56 + 24 * me2 * pow<2>(mm2) * s56 -
        32 * me2 * mm2 * s12 * s56 + 12 * pow<2>(me2) * s15 * s56 -
        20 * me2 * mm2 * s15 * s56 - 8 * me2 * s12 * s15 * s56 -
        8 * me2 * pow<2>(s15) * s56 - 32 * me2 * mm2 * s16 * s56 -
        8 * me2 * s15 * s16 * s56 + 32 * me2 * mm2 * s25 * s56 +
        16 * me2 * s15 * s25 * s56 + 20 * me2 * mm2 * s26 * s56 -
        12 * pow<2>(mm2) * s26 * s56 + 8 * mm2 * s12 * s26 * s56 +
        10 * me2 * s15 * s26 * s56 + 10 * mm2 * s15 * s26 * s56 +
        2 * s12 * s15 * s26 * s56 + 4 * pow<2>(s15) * s26 * s56 +
        16 * mm2 * s16 * s26 * s56 + 4 * s15 * s16 * s26 * s56 -
        8 * mm2 * s25 * s26 * s56 - 4 * s15 * s25 * s26 * s56 -
        8 * mm2 * pow<2>(s26) * s56 - 4 * s15 * pow<2>(s26) * s56 +
        16 * me2 * mm2 * pow<2>(s56) + 8 * me2 * s15 * pow<2>(s56) -
        8 * mm2 * s26 * pow<2>(s56) - 4 * s15 * s26 * pow<2>(s56)};
    const auto if15{
        -8 * me2 * mm2 * pow<2>(s12) + 6 * me2 * pow<3>(s12) +
        6 * mm2 * pow<3>(s12) - 4 * pow<4>(s12) + 6 * pow<2>(me2) * s12 * s15 -
        10 * me2 * mm2 * s12 * s15 + 10 * me2 * pow<2>(s12) * s15 +
        12 * mm2 * pow<2>(s12) * s15 - 12 * pow<3>(s12) * s15 +
        6 * pow<2>(me2) * pow<2>(s15) - 2 * me2 * mm2 * pow<2>(s15) +
        4 * me2 * s12 * pow<2>(s15) + 6 * mm2 * s12 * pow<2>(s15) -
        12 * pow<2>(s12) * pow<2>(s15) - 4 * s12 * pow<3>(s15) -
        8 * pow<2>(me2) * mm2 * s16 + 12 * pow<2>(me2) * s12 * s16 +
        4 * me2 * mm2 * s12 * s16 + 6 * mm2 * pow<2>(s12) * s16 -
        8 * pow<3>(s12) * s16 + 14 * pow<2>(me2) * s15 * s16 +
        2 * me2 * mm2 * s15 * s16 - 6 * me2 * s12 * s15 * s16 +
        6 * mm2 * s12 * s15 * s16 - 16 * pow<2>(s12) * s15 * s16 -
        4 * me2 * pow<2>(s15) * s16 - 8 * s12 * pow<2>(s15) * s16 +
        6 * pow<2>(me2) * pow<2>(s16) + 2 * me2 * mm2 * pow<2>(s16) -
        6 * me2 * s12 * pow<2>(s16) - 4 * pow<2>(s12) * pow<2>(s16) -
        4 * me2 * s15 * pow<2>(s16) - 4 * s12 * s15 * pow<2>(s16) +
        8 * me2 * pow<2>(mm2) * s25 - 4 * me2 * mm2 * s12 * s25 -
        12 * pow<2>(mm2) * s12 * s25 - 6 * me2 * pow<2>(s12) * s25 +
        8 * pow<3>(s12) * s25 - 2 * me2 * mm2 * s15 * s25 -
        6 * pow<2>(mm2) * s15 * s25 + 6 * mm2 * s12 * s15 * s25 +
        12 * pow<2>(s12) * s15 * s25 + 2 * me2 * pow<2>(s15) * s25 +
        4 * mm2 * pow<2>(s15) * s25 + 4 * s12 * pow<2>(s15) * s25 -
        3 * pow<2>(me2) * s16 * s25 - 6 * me2 * mm2 * s16 * s25 -
        3 * pow<2>(mm2) * s16 * s25 + 12 * me2 * s12 * s16 * s25 +
        12 * mm2 * s12 * s16 * s25 + 4 * pow<2>(s12) * s16 * s25 +
        9 * me2 * s15 * s16 * s25 + 9 * mm2 * s15 * s16 * s25 -
        2 * s12 * s15 * s16 * s25 - 2 * pow<2>(s15) * s16 * s25 +
        3 * me2 * pow<2>(s16) * s25 + 3 * mm2 * pow<2>(s16) * s25 -
        4 * s12 * pow<2>(s16) * s25 - 2 * s15 * pow<2>(s16) * s25 +
        2 * me2 * mm2 * pow<2>(s25) + 6 * pow<2>(mm2) * pow<2>(s25) -
        6 * mm2 * s12 * pow<2>(s25) - 4 * pow<2>(s12) * pow<2>(s25) -
        4 * mm2 * s15 * pow<2>(s25) - 3 * me2 * s16 * pow<2>(s25) -
        3 * mm2 * s16 * pow<2>(s25) + 4 * s12 * s16 * pow<2>(s25) +
        s15 * s16 * pow<2>(s25) - pow<2>(s16) * pow<2>(s25) +
        10 * me2 * mm2 * s12 * s26 - 6 * pow<2>(mm2) * s12 * s26 -
        12 * me2 * pow<2>(s12) * s26 - 10 * mm2 * pow<2>(s12) * s26 +
        12 * pow<3>(s12) * s26 - 3 * pow<2>(me2) * s15 * s26 +
        10 * me2 * mm2 * s15 * s26 - 3 * pow<2>(mm2) * s15 * s26 -
        10 * me2 * s12 * s15 * s26 - 10 * mm2 * s12 * s15 * s26 +
        24 * pow<2>(s12) * s15 * s26 - me2 * pow<2>(s15) * s26 -
        mm2 * pow<2>(s15) * s26 + 14 * s12 * pow<2>(s15) * s26 +
        2 * pow<3>(s15) * s26 - 6 * pow<2>(me2) * s16 * s26 -
        2 * me2 * mm2 * s16 * s26 + 6 * me2 * s12 * s16 * s26 +
        12 * pow<2>(s12) * s16 * s26 + 7 * me2 * s15 * s16 * s26 +
        mm2 * s15 * s16 * s26 + 12 * s12 * s15 * s16 * s26 +
        2 * pow<2>(s15) * s16 * s26 + 4 * me2 * pow<2>(s16) * s26 +
        2 * me2 * mm2 * s25 * s26 + 14 * pow<2>(mm2) * s25 * s26 +
        6 * me2 * s12 * s25 * s26 - 6 * mm2 * s12 * s25 * s26 -
        16 * pow<2>(s12) * s25 * s26 - me2 * s15 * s25 * s26 -
        7 * mm2 * s15 * s25 * s26 - 12 * s12 * s15 * s25 * s26 -
        3 * pow<2>(s15) * s25 * s26 - 9 * me2 * s16 * s25 * s26 -
        9 * mm2 * s16 * s25 * s26 + 2 * s12 * s16 * s25 * s26 +
        2 * s15 * s16 * s25 * s26 + pow<2>(s16) * s25 * s26 +
        4 * mm2 * pow<2>(s25) * s26 + 4 * s12 * pow<2>(s25) * s26 -
        2 * s16 * pow<2>(s25) * s26 - 2 * me2 * mm2 * pow<2>(s26) +
        6 * pow<2>(mm2) * pow<2>(s26) + 6 * me2 * s12 * pow<2>(s26) +
        4 * mm2 * s12 * pow<2>(s26) - 12 * pow<2>(s12) * pow<2>(s26) +
        me2 * s15 * pow<2>(s26) + mm2 * s15 * pow<2>(s26) -
        14 * s12 * s15 * pow<2>(s26) - 5 * pow<2>(s15) * pow<2>(s26) -
        4 * me2 * s16 * pow<2>(s26) - 2 * mm2 * s16 * pow<2>(s26) -
        4 * s12 * s16 * pow<2>(s26) - 3 * s15 * s16 * pow<2>(s26) +
        4 * mm2 * s25 * pow<2>(s26) + 8 * s12 * s25 * pow<2>(s26) +
        2 * s15 * s25 * pow<2>(s26) - 2 * s16 * s25 * pow<2>(s26) +
        4 * s12 * pow<3>(s26) + 2 * s15 * pow<3>(s26) +
        8 * pow<2>(me2) * mm2 * s56 + 8 * me2 * pow<2>(mm2) * s56 -
        3 * pow<2>(me2) * s12 * s56 - 2 * me2 * mm2 * s12 * s56 -
        3 * pow<2>(mm2) * s12 * s56 - 10 * me2 * pow<2>(s12) * s56 -
        10 * mm2 * pow<2>(s12) * s56 + 12 * pow<3>(s12) * s56 -
        6 * pow<2>(me2) * s15 * s56 - 2 * me2 * mm2 * s15 * s56 -
        7 * me2 * s12 * s15 * s56 - 9 * mm2 * s12 * s15 * s56 +
        22 * pow<2>(s12) * s15 * s56 + 10 * s12 * pow<2>(s15) * s56 -
        8 * pow<2>(me2) * s16 * s56 - 8 * me2 * mm2 * s16 * s56 +
        7 * me2 * s12 * s16 * s56 - mm2 * s12 * s16 * s56 +
        12 * pow<2>(s12) * s16 * s56 + 4 * me2 * s15 * s16 * s56 +
        10 * s12 * s15 * s16 * s56 + 2 * me2 * pow<2>(s16) * s56 +
        8 * me2 * mm2 * s25 * s56 + 8 * pow<2>(mm2) * s25 * s56 +
        me2 * s12 * s25 * s56 - 7 * mm2 * s12 * s25 * s56 -
        12 * pow<2>(s12) * s25 * s56 - 2 * me2 * s15 * s25 * s56 -
        8 * mm2 * s15 * s25 * s56 - 5 * s12 * s15 * s25 * s56 -
        8 * me2 * s16 * s25 * s56 - 8 * mm2 * s16 * s25 * s56 +
        7 * s12 * s16 * s25 * s56 + 3 * s15 * s16 * s25 * s56 +
        pow<2>(s16) * s25 * s56 + 2 * mm2 * pow<2>(s25) * s56 -
        s16 * pow<2>(s25) * s56 + 2 * me2 * mm2 * s26 * s56 +
        6 * pow<2>(mm2) * s26 * s56 + 9 * me2 * s12 * s26 * s56 +
        7 * mm2 * s12 * s26 * s56 - 22 * pow<2>(s12) * s26 * s56 -
        21 * s12 * s15 * s26 * s56 - 5 * pow<2>(s15) * s26 * s56 -
        8 * me2 * s16 * s26 * s56 - 2 * mm2 * s16 * s26 * s56 -
        5 * s12 * s16 * s26 * s56 - 3 * s15 * s16 * s26 * s56 +
        4 * mm2 * s25 * s26 * s56 + 10 * s12 * s25 * s26 * s56 +
        3 * s15 * s25 * s26 * s56 - 3 * s16 * s25 * s26 * s56 +
        10 * s12 * pow<2>(s26) * s56 + 5 * s15 * pow<2>(s26) * s56 +
        4 * me2 * mm2 * pow<2>(s56) + 4 * me2 * s12 * pow<2>(s56) +
        4 * mm2 * s12 * pow<2>(s56) - 10 * pow<2>(s12) * pow<2>(s56) -
        7 * s12 * s15 * pow<2>(s56) - 2 * me2 * s16 * pow<2>(s56) -
        s12 * s16 * pow<2>(s56) + 2 * mm2 * s25 * pow<2>(s56) +
        s12 * s25 * pow<2>(s56) - s16 * s25 * pow<2>(s56) +
        7 * s12 * s26 * pow<2>(s56) + 3 * s15 * s26 * pow<2>(s56) +
        s12 * pow<3>(s56)};
    const auto if25{
        -8 * me2 * mm2 * pow<2>(s12) + 6 * me2 * pow<3>(s12) +
        6 * mm2 * pow<3>(s12) - 4 * pow<4>(s12) + 6 * pow<2>(me2) * s12 * s15 -
        2 * me2 * mm2 * s12 * s15 + 4 * me2 * pow<2>(s12) * s15 +
        6 * mm2 * pow<2>(s12) * s15 - 8 * pow<3>(s12) * s15 -
        4 * pow<2>(s12) * pow<2>(s15) - 8 * pow<2>(me2) * mm2 * s16 +
        12 * pow<2>(me2) * s12 * s16 - 4 * me2 * mm2 * s12 * s16 +
        6 * me2 * pow<2>(s12) * s16 + 12 * mm2 * pow<2>(s12) * s16 -
        12 * pow<3>(s12) * s16 + 8 * pow<2>(me2) * s15 * s16 -
        2 * me2 * s12 * s15 * s16 + 6 * mm2 * s12 * s15 * s16 -
        16 * pow<2>(s12) * s15 * s16 - 2 * me2 * pow<2>(s15) * s16 -
        4 * s12 * pow<2>(s15) * s16 + 6 * pow<2>(me2) * pow<2>(s16) -
        2 * me2 * mm2 * pow<2>(s16) + 2 * me2 * s12 * pow<2>(s16) +
        6 * mm2 * s12 * pow<2>(s16) - 12 * pow<2>(s12) * pow<2>(s16) -
        8 * s12 * s15 * pow<2>(s16) + 2 * me2 * pow<3>(s16) -
        4 * s12 * pow<3>(s16) + 2 * me2 * mm2 * s12 * s25 -
        6 * pow<2>(mm2) * s12 * s25 - 6 * me2 * pow<2>(s12) * s25 -
        4 * mm2 * pow<2>(s12) * s25 + 8 * pow<3>(s12) * s25 +
        4 * me2 * s12 * s15 * s25 + 4 * mm2 * s12 * s15 * s25 +
        6 * pow<2>(s12) * s15 * s25 - 3 * pow<2>(me2) * s16 * s25 -
        6 * me2 * mm2 * s16 * s25 - 3 * pow<2>(mm2) * s16 * s25 +
        6 * me2 * s12 * s16 * s25 + 14 * pow<2>(s12) * s16 * s25 +
        7 * me2 * s15 * s16 * s25 + 2 * mm2 * s15 * s16 * s25 +
        6 * s12 * s15 * s16 * s25 + 4 * me2 * pow<2>(s16) * s25 +
        3 * mm2 * pow<2>(s16) * s25 + 6 * s12 * pow<2>(s16) * s25 -
        4 * pow<2>(s12) * pow<2>(s25) - 2 * me2 * s16 * pow<2>(s25) -
        mm2 * s16 * pow<2>(s25) - 4 * s12 * s16 * pow<2>(s25) +
        8 * me2 * pow<2>(mm2) * s26 + 4 * me2 * mm2 * s12 * s26 -
        12 * pow<2>(mm2) * s12 * s26 - 12 * me2 * pow<2>(s12) * s26 -
        6 * mm2 * pow<2>(s12) * s26 + 12 * pow<3>(s12) * s26 -
        3 * pow<2>(me2) * s15 * s26 - 6 * me2 * mm2 * s15 * s26 -
        3 * pow<2>(mm2) * s15 * s26 + 6 * mm2 * s12 * s15 * s26 +
        14 * pow<2>(s12) * s15 * s26 + me2 * pow<2>(s15) * s26 +
        2 * mm2 * pow<2>(s15) * s26 + 4 * s12 * pow<2>(s15) * s26 -
        6 * pow<2>(me2) * s16 * s26 - 12 * me2 * mm2 * s16 * s26 -
        6 * pow<2>(mm2) * s16 * s26 + 22 * pow<2>(s12) * s16 * s26 +
        8 * me2 * s15 * s16 * s26 + 5 * mm2 * s15 * s16 * s26 +
        14 * s12 * s15 * s16 * s26 + 4 * me2 * pow<2>(s16) * s26 +
        4 * mm2 * pow<2>(s16) * s26 + 10 * s12 * pow<2>(s16) * s26 +
        8 * pow<2>(mm2) * s25 * s26 + 6 * me2 * s12 * s25 * s26 -
        2 * mm2 * s12 * s25 * s26 - 16 * pow<2>(s12) * s25 * s26 -
        2 * me2 * s15 * s25 * s26 - 7 * mm2 * s15 * s25 * s26 -
        6 * s12 * s15 * s25 * s26 - 5 * me2 * s16 * s25 * s26 -
        8 * mm2 * s16 * s25 * s26 - 14 * s12 * s16 * s25 * s26 +
        2 * mm2 * pow<2>(s25) * s26 + 4 * s12 * pow<2>(s25) * s26 -
        2 * me2 * mm2 * pow<2>(s26) + 6 * pow<2>(mm2) * pow<2>(s26) +
        6 * me2 * s12 * pow<2>(s26) + 2 * mm2 * s12 * pow<2>(s26) -
        12 * pow<2>(s12) * pow<2>(s26) - 3 * me2 * s15 * pow<2>(s26) -
        4 * mm2 * s15 * pow<2>(s26) - 6 * s12 * s15 * pow<2>(s26) -
        4 * me2 * s16 * pow<2>(s26) - 4 * mm2 * s16 * pow<2>(s26) -
        10 * s12 * s16 * pow<2>(s26) + 8 * s12 * s25 * pow<2>(s26) -
        2 * mm2 * pow<3>(s26) + 4 * s12 * pow<3>(s26) +
        8 * pow<2>(me2) * mm2 * s56 + 8 * me2 * pow<2>(mm2) * s56 -
        3 * pow<2>(me2) * s12 * s56 - 2 * me2 * mm2 * s12 * s56 -
        3 * pow<2>(mm2) * s12 * s56 - 10 * me2 * pow<2>(s12) * s56 -
        10 * mm2 * pow<2>(s12) * s56 + 12 * pow<3>(s12) * s56 -
        6 * me2 * mm2 * s15 * s56 + me2 * s12 * s15 * s56 +
        2 * mm2 * s12 * s15 * s56 + 10 * pow<2>(s12) * s15 * s56 -
        8 * pow<2>(me2) * s16 * s56 - 10 * me2 * mm2 * s16 * s56 -
        9 * mm2 * s12 * s16 * s56 + 22 * pow<2>(s12) * s16 * s56 +
        6 * me2 * s15 * s16 * s56 + 10 * s12 * s15 * s16 * s56 +
        2 * me2 * pow<2>(s16) * s56 + 10 * s12 * pow<2>(s16) * s56 +
        6 * me2 * mm2 * s25 * s56 - 2 * me2 * s12 * s25 * s56 -
        mm2 * s12 * s25 * s56 - 10 * pow<2>(s12) * s25 * s56 -
        6 * me2 * s16 * s25 * s56 - 2 * mm2 * s16 * s25 * s56 -
        10 * s12 * s16 * s25 * s56 + 10 * me2 * mm2 * s26 * s56 +
        8 * pow<2>(mm2) * s26 * s56 + 9 * me2 * s12 * s26 * s56 -
        22 * pow<2>(s12) * s26 * s56 - 2 * me2 * s15 * s26 * s56 -
        6 * mm2 * s15 * s26 * s56 - 10 * s12 * s15 * s26 * s56 -
        6 * me2 * s16 * s26 * s56 - 6 * mm2 * s16 * s26 * s56 -
        20 * s12 * s16 * s26 * s56 + 6 * mm2 * s25 * s26 * s56 +
        10 * s12 * s25 * s26 * s56 + 2 * mm2 * pow<2>(s26) * s56 +
        10 * s12 * pow<2>(s26) * s56 + 4 * me2 * mm2 * pow<2>(s56) -
        6 * pow<2>(s12) * pow<2>(s56) - 2 * me2 * s16 * pow<2>(s56) -
        6 * s12 * s16 * pow<2>(s56) + 2 * mm2 * s26 * pow<2>(s56) +
        6 * s12 * s26 * pow<2>(s56)};
    const auto if35{
        32 * me2 * pow<3>(mm2) - 24 * me2 * pow<2>(mm2) * s12 -
        24 * pow<3>(mm2) * s12 + 16 * pow<2>(mm2) * pow<2>(s12) -
        40 * me2 * pow<2>(mm2) * s15 + 6 * me2 * mm2 * s12 * s15 +
        30 * pow<2>(mm2) * s12 * s15 - 4 * mm2 * pow<2>(s12) * s15 +
        8 * me2 * mm2 * pow<2>(s15) - 6 * mm2 * s12 * pow<2>(s15) +
        4 * me2 * pow<3>(s15) - 2 * s12 * pow<3>(s15) -
        40 * me2 * pow<2>(mm2) * s16 + 6 * me2 * mm2 * s12 * s16 +
        30 * pow<2>(mm2) * s12 * s16 - 4 * mm2 * pow<2>(s12) * s16 +
        16 * me2 * mm2 * s15 * s16 - 12 * mm2 * s12 * s15 * s16 +
        12 * me2 * pow<2>(s15) * s16 - 6 * s12 * pow<2>(s15) * s16 +
        8 * me2 * mm2 * pow<2>(s16) - 6 * mm2 * s12 * pow<2>(s16) +
        12 * me2 * s15 * pow<2>(s16) - 6 * s12 * s15 * pow<2>(s16) +
        4 * me2 * pow<3>(s16) - 2 * s12 * pow<3>(s16) +
        24 * me2 * pow<2>(mm2) * s25 + 24 * pow<3>(mm2) * s25 -
        32 * pow<2>(mm2) * s12 * s25 - 6 * me2 * mm2 * s15 * s25 -
        30 * pow<2>(mm2) * s15 * s25 + 8 * mm2 * s12 * s15 * s25 -
        6 * me2 * pow<2>(s15) * s25 + 4 * mm2 * pow<2>(s15) * s25 +
        4 * s12 * pow<2>(s15) * s25 + 4 * pow<3>(s15) * s25 -
        6 * me2 * mm2 * s16 * s25 - 30 * pow<2>(mm2) * s16 * s25 +
        8 * mm2 * s12 * s16 * s25 - 12 * me2 * s15 * s16 * s25 +
        8 * mm2 * s15 * s16 * s25 + 8 * s12 * s15 * s16 * s25 +
        12 * pow<2>(s15) * s16 * s25 - 6 * me2 * pow<2>(s16) * s25 +
        4 * mm2 * pow<2>(s16) * s25 + 4 * s12 * pow<2>(s16) * s25 +
        12 * s15 * pow<2>(s16) * s25 + 4 * pow<3>(s16) * s25 +
        16 * pow<2>(mm2) * pow<2>(s25) - 4 * mm2 * s15 * pow<2>(s25) -
        4 * pow<2>(s15) * pow<2>(s25) - 4 * mm2 * s16 * pow<2>(s25) -
        8 * s15 * s16 * pow<2>(s25) - 4 * pow<2>(s16) * pow<2>(s25) +
        24 * me2 * pow<2>(mm2) * s26 + 24 * pow<3>(mm2) * s26 -
        32 * pow<2>(mm2) * s12 * s26 - 6 * me2 * mm2 * s15 * s26 -
        30 * pow<2>(mm2) * s15 * s26 + 8 * mm2 * s12 * s15 * s26 -
        6 * me2 * pow<2>(s15) * s26 + 4 * mm2 * pow<2>(s15) * s26 +
        4 * s12 * pow<2>(s15) * s26 + 4 * pow<3>(s15) * s26 -
        6 * me2 * mm2 * s16 * s26 - 30 * pow<2>(mm2) * s16 * s26 +
        8 * mm2 * s12 * s16 * s26 - 12 * me2 * s15 * s16 * s26 +
        8 * mm2 * s15 * s16 * s26 + 8 * s12 * s15 * s16 * s26 +
        12 * pow<2>(s15) * s16 * s26 - 6 * me2 * pow<2>(s16) * s26 +
        4 * mm2 * pow<2>(s16) * s26 + 4 * s12 * pow<2>(s16) * s26 +
        12 * s15 * pow<2>(s16) * s26 + 4 * pow<3>(s16) * s26 +
        32 * pow<2>(mm2) * s25 * s26 - 8 * mm2 * s15 * s25 * s26 -
        8 * pow<2>(s15) * s25 * s26 - 8 * mm2 * s16 * s25 * s26 -
        16 * s15 * s16 * s25 * s26 - 8 * pow<2>(s16) * s25 * s26 +
        16 * pow<2>(mm2) * pow<2>(s26) - 4 * mm2 * s15 * pow<2>(s26) -
        4 * pow<2>(s15) * pow<2>(s26) - 4 * mm2 * s16 * pow<2>(s26) -
        8 * s15 * s16 * pow<2>(s26) - 4 * pow<2>(s16) * pow<2>(s26) +
        32 * me2 * pow<2>(mm2) * s56 - 24 * pow<2>(mm2) * s12 * s56 -
        16 * me2 * mm2 * s15 * s56 + 6 * me2 * s12 * s15 * s56 +
        12 * mm2 * s12 * s15 * s56 - 4 * pow<2>(s12) * s15 * s56 -
        8 * me2 * pow<2>(s15) * s56 + 2 * s12 * pow<2>(s15) * s56 -
        16 * me2 * mm2 * s16 * s56 + 6 * me2 * s12 * s16 * s56 +
        12 * mm2 * s12 * s16 * s56 - 4 * pow<2>(s12) * s16 * s56 -
        16 * me2 * s15 * s16 * s56 + 4 * s12 * s15 * s16 * s56 -
        8 * me2 * pow<2>(s16) * s56 + 2 * s12 * pow<2>(s16) * s56 +
        24 * pow<2>(mm2) * s25 * s56 + 6 * me2 * s15 * s25 * s56 -
        8 * mm2 * s15 * s25 * s56 - 8 * pow<2>(s15) * s25 * s56 +
        6 * me2 * s16 * s25 * s56 - 8 * mm2 * s16 * s25 * s56 -
        16 * s15 * s16 * s25 * s56 - 8 * pow<2>(s16) * s25 * s56 +
        4 * s15 * pow<2>(s25) * s56 + 4 * s16 * pow<2>(s25) * s56 +
        24 * pow<2>(mm2) * s26 * s56 + 6 * me2 * s15 * s26 * s56 -
        8 * mm2 * s15 * s26 * s56 - 8 * pow<2>(s15) * s26 * s56 +
        6 * me2 * s16 * s26 * s56 - 8 * mm2 * s16 * s26 * s56 -
        16 * s15 * s16 * s26 * s56 - 8 * pow<2>(s16) * s26 * s56 +
        8 * s15 * s25 * s26 * s56 + 8 * s16 * s25 * s26 * s56 +
        4 * s15 * pow<2>(s26) * s56 + 4 * s16 * pow<2>(s26) * s56 +
        8 * me2 * mm2 * pow<2>(s56) - 6 * me2 * s12 * pow<2>(s56) -
        6 * mm2 * s12 * pow<2>(s56) + 4 * pow<2>(s12) * pow<2>(s56) +
        4 * me2 * s15 * pow<2>(s56) + 2 * s12 * s15 * pow<2>(s56) +
        4 * me2 * s16 * pow<2>(s56) + 2 * s12 * s16 * pow<2>(s56) +
        4 * mm2 * s25 * pow<2>(s56) - 4 * s12 * s25 * pow<2>(s56) +
        4 * s15 * s25 * pow<2>(s56) + 4 * s16 * s25 * pow<2>(s56) +
        4 * mm2 * s26 * pow<2>(s56) - 4 * s12 * s26 * pow<2>(s56) +
        4 * s15 * s26 * pow<2>(s56) + 4 * s16 * s26 * pow<2>(s56) -
        2 * s12 * pow<3>(s56)};
    const auto if45{
        16 * me2 * pow<2>(mm2) * s12 - 12 * me2 * mm2 * pow<2>(s12) -
        12 * pow<2>(mm2) * pow<2>(s12) + 8 * mm2 * pow<3>(s12) -
        16 * me2 * mm2 * s12 * s15 + 12 * mm2 * pow<2>(s12) * s15 -
        4 * me2 * s12 * pow<2>(s15) + 2 * pow<2>(s12) * pow<2>(s15) +
        8 * me2 * pow<2>(mm2) * s16 - 20 * me2 * mm2 * s12 * s16 -
        12 * pow<2>(mm2) * s12 * s16 + 18 * mm2 * pow<2>(s12) * s16 +
        6 * pow<2>(me2) * s15 * s16 - 6 * me2 * mm2 * s15 * s16 -
        8 * me2 * s12 * s15 * s16 + 12 * mm2 * s12 * s15 * s16 +
        2 * pow<2>(s12) * s15 * s16 - 4 * me2 * pow<2>(s15) * s16 +
        2 * s12 * pow<2>(s15) * s16 + 6 * pow<2>(me2) * pow<2>(s16) -
        6 * me2 * mm2 * pow<2>(s16) - 4 * me2 * s12 * pow<2>(s16) +
        10 * mm2 * s12 * pow<2>(s16) - 6 * me2 * s15 * pow<2>(s16) +
        2 * s12 * s15 * pow<2>(s16) - 2 * me2 * pow<3>(s16) +
        12 * me2 * mm2 * s12 * s25 + 12 * pow<2>(mm2) * s12 * s25 -
        16 * mm2 * pow<2>(s12) * s25 + 6 * me2 * s12 * s15 * s25 -
        10 * mm2 * s12 * s15 * s25 - 4 * pow<2>(s12) * s15 * s25 -
        4 * s12 * pow<2>(s15) * s25 + 10 * me2 * mm2 * s16 * s25 +
        6 * pow<2>(mm2) * s16 * s25 - 24 * mm2 * s12 * s16 * s25 +
        5 * me2 * s15 * s16 * s25 - 5 * mm2 * s15 * s16 * s25 -
        6 * s12 * s15 * s16 * s25 - 2 * pow<2>(s15) * s16 * s25 +
        3 * me2 * pow<2>(s16) * s25 - 6 * mm2 * pow<2>(s16) * s25 -
        2 * s15 * pow<2>(s16) * s25 + 8 * mm2 * s12 * pow<2>(s25) +
        4 * s12 * s15 * pow<2>(s25) + 3 * me2 * s16 * pow<2>(s25) +
        7 * mm2 * s16 * pow<2>(s25) - 2 * s12 * s16 * pow<2>(s25) -
        2 * pow<2>(s16) * pow<2>(s25) + 2 * s16 * pow<3>(s25) -
        4 * me2 * pow<2>(mm2) * s26 + 12 * pow<3>(mm2) * s26 +
        12 * me2 * mm2 * s12 * s26 + 8 * pow<2>(mm2) * s12 * s26 -
        16 * mm2 * pow<2>(s12) * s26 + 6 * me2 * mm2 * s15 * s26 -
        18 * pow<2>(mm2) * s15 * s26 - 12 * mm2 * s12 * s15 * s26 +
        me2 * pow<2>(s15) * s26 + 3 * mm2 * pow<2>(s15) * s26 -
        2 * s12 * pow<2>(s15) * s26 + 2 * pow<3>(s15) * s26 +
        8 * me2 * mm2 * s16 * s26 - 12 * pow<2>(mm2) * s16 * s26 -
        6 * me2 * s12 * s16 * s26 - 18 * mm2 * s12 * s16 * s26 +
        4 * pow<2>(s12) * s16 * s26 + 7 * me2 * s15 * s16 * s26 +
        6 * mm2 * s15 * s16 * s26 - 2 * s12 * s15 * s16 * s26 +
        2 * pow<2>(s15) * s16 * s26 + 4 * me2 * pow<2>(s16) * s26 +
        2 * mm2 * pow<2>(s16) * s26 + 2 * s12 * pow<2>(s16) * s26 -
        6 * me2 * mm2 * s25 * s26 + 2 * pow<2>(mm2) * s25 * s26 +
        20 * mm2 * s12 * s25 * s26 - 3 * me2 * s15 * s25 * s26 +
        3 * mm2 * s15 * s25 * s26 + 6 * s12 * s15 * s25 * s26 +
        3 * me2 * s16 * s25 * s26 + 10 * mm2 * s16 * s25 * s26 -
        6 * s12 * s16 * s25 * s26 + 2 * s15 * s16 * s25 * s26 -
        2 * pow<2>(s16) * s25 * s26 - 4 * mm2 * pow<2>(s25) * s26 -
        2 * s15 * pow<2>(s25) * s26 + 4 * s16 * pow<2>(s25) * s26 +
        8 * mm2 * s12 * pow<2>(s26) - 3 * me2 * s15 * pow<2>(s26) +
        2 * mm2 * s15 * pow<2>(s26) + 2 * s12 * s15 * pow<2>(s26) +
        2 * mm2 * s16 * pow<2>(s26) - 4 * s12 * s16 * pow<2>(s26) +
        2 * s15 * s16 * pow<2>(s26) - 4 * mm2 * s25 * pow<2>(s26) -
        4 * s15 * s25 * pow<2>(s26) + 2 * s16 * s25 * pow<2>(s26) -
        2 * s15 * pow<3>(s26) + 14 * me2 * mm2 * s12 * s56 +
        6 * pow<2>(mm2) * s12 * s56 - 14 * mm2 * pow<2>(s12) * s56 +
        7 * me2 * s12 * s15 * s56 - 5 * mm2 * s12 * s15 * s56 -
        4 * pow<2>(s12) * s15 * s56 - 2 * s12 * pow<2>(s15) * s56 -
        6 * pow<2>(me2) * s16 * s56 + 8 * me2 * mm2 * s16 * s56 +
        5 * me2 * s12 * s16 * s56 - 16 * mm2 * s12 * s16 * s56 +
        4 * me2 * s15 * s16 * s56 - 4 * s12 * s15 * s16 * s56 +
        4 * me2 * pow<2>(s16) * s56 - 3 * me2 * s12 * s25 * s56 +
        13 * mm2 * s12 * s25 * s56 + 2 * pow<2>(s12) * s25 * s56 +
        8 * s12 * s15 * s25 * s56 - 2 * me2 * s16 * s25 * s56 +
        7 * mm2 * s16 * s25 * s56 + 2 * s12 * s16 * s25 * s56 +
        2 * s15 * s16 * s25 * s56 - 2 * s12 * pow<2>(s25) * s56 +
        2 * s16 * pow<2>(s25) * s56 - 8 * me2 * mm2 * s26 * s56 +
        10 * pow<2>(mm2) * s26 * s56 + 3 * me2 * s12 * s26 * s56 +
        16 * mm2 * s12 * s26 * s56 - 2 * pow<2>(s12) * s26 * s56 -
        2 * me2 * s15 * s26 * s56 - 3 * mm2 * s15 * s26 * s56 +
        6 * s12 * s15 * s26 * s56 - 2 * pow<2>(s15) * s26 * s56 -
        4 * me2 * s16 * s26 * s56 - 2 * mm2 * s16 * s26 * s56 -
        2 * s12 * s16 * s26 * s56 - 8 * mm2 * s25 * s26 * s56 -
        2 * s15 * s25 * s26 * s56 + 2 * s16 * s25 * s26 * s56 -
        4 * mm2 * pow<2>(s26) * s56 + 2 * s12 * pow<2>(s26) * s56 -
        2 * s15 * pow<2>(s26) * s56 + 2 * me2 * mm2 * pow<2>(s56) -
        2 * me2 * s12 * pow<2>(s56) + 5 * mm2 * s12 * pow<2>(s56) +
        2 * s12 * s15 * pow<2>(s56) - 2 * me2 * s16 * pow<2>(s56) -
        2 * s12 * s25 * pow<2>(s56)};
    const auto if55{
        32 * me2 * pow<3>(mm2) - 24 * me2 * pow<2>(mm2) * s12 -
        24 * pow<3>(mm2) * s12 + 16 * pow<2>(mm2) * pow<2>(s12) -
        32 * me2 * pow<2>(mm2) * s15 + 24 * pow<2>(mm2) * s12 * s15 -
        8 * me2 * mm2 * pow<2>(s15) + 4 * mm2 * s12 * pow<2>(s15) -
        48 * me2 * pow<2>(mm2) * s16 + 12 * me2 * mm2 * s12 * s16 +
        36 * pow<2>(mm2) * s12 * s16 - 8 * mm2 * pow<2>(s12) * s16 +
        8 * me2 * mm2 * s15 * s16 - 8 * mm2 * s12 * s15 * s16 +
        24 * me2 * mm2 * pow<2>(s16) - 6 * me2 * s12 * pow<2>(s16) -
        18 * mm2 * s12 * pow<2>(s16) + 4 * pow<2>(s12) * pow<2>(s16) -
        4 * me2 * s15 * pow<2>(s16) + 4 * s12 * s15 * pow<2>(s16) -
        4 * me2 * pow<3>(s16) + 4 * s12 * pow<3>(s16) +
        24 * me2 * pow<2>(mm2) * s25 + 24 * pow<3>(mm2) * s25 -
        32 * pow<2>(mm2) * s12 * s25 + 12 * me2 * mm2 * s15 * s25 -
        20 * pow<2>(mm2) * s15 * s25 - 8 * mm2 * s12 * s15 * s25 -
        8 * mm2 * pow<2>(s15) * s25 - 32 * pow<2>(mm2) * s16 * s25 +
        8 * mm2 * s12 * s16 * s25 + 6 * me2 * pow<2>(s16) * s25 +
        14 * mm2 * pow<2>(s16) * s25 - 8 * s12 * pow<2>(s16) * s25 -
        4 * s15 * pow<2>(s16) * s25 - 4 * pow<3>(s16) * s25 +
        16 * pow<2>(mm2) * pow<2>(s25) + 8 * mm2 * s15 * pow<2>(s25) +
        4 * pow<2>(s16) * pow<2>(s25) + 24 * me2 * pow<2>(mm2) * s26 +
        24 * pow<3>(mm2) * s26 - 32 * pow<2>(mm2) * s12 * s26 -
        24 * pow<2>(mm2) * s15 * s26 - 4 * mm2 * pow<2>(s15) * s26 -
        12 * me2 * mm2 * s16 * s26 - 36 * pow<2>(mm2) * s16 * s26 +
        16 * mm2 * s12 * s16 * s26 - 6 * me2 * s15 * s16 * s26 +
        6 * mm2 * s15 * s16 * s26 + 4 * s12 * s15 * s16 * s26 +
        2 * pow<2>(s15) * s16 * s26 + 16 * mm2 * pow<2>(s16) * s26 -
        4 * s12 * pow<2>(s16) * s26 - 2 * pow<3>(s16) * s26 +
        32 * pow<2>(mm2) * s25 * s26 + 8 * mm2 * s15 * s25 * s26 -
        8 * mm2 * s16 * s25 * s26 - 4 * s15 * s16 * s25 * s26 +
        4 * pow<2>(s16) * s25 * s26 + 16 * pow<2>(mm2) * pow<2>(s26) -
        8 * mm2 * s16 * pow<2>(s26) - 4 * s15 * s16 * pow<2>(s26) +
        32 * me2 * pow<2>(mm2) * s56 - 24 * pow<2>(mm2) * s12 * s56 +
        16 * me2 * mm2 * s15 * s56 - 8 * mm2 * s12 * s15 * s56 -
        16 * me2 * mm2 * s16 * s56 + 6 * me2 * s12 * s16 * s56 +
        14 * mm2 * s12 * s16 * s56 - 4 * pow<2>(s12) * s16 * s56 +
        4 * me2 * s15 * s16 * s56 - 4 * s12 * s15 * s16 * s56 +
        8 * me2 * pow<2>(s16) * s56 - 8 * s12 * pow<2>(s16) * s56 -
        12 * me2 * mm2 * s25 * s56 + 20 * pow<2>(mm2) * s25 * s56 +
        8 * mm2 * s12 * s25 * s56 + 16 * mm2 * s15 * s25 * s56 -
        6 * me2 * s16 * s25 * s56 - 6 * mm2 * s16 * s25 * s56 +
        8 * s12 * s16 * s25 * s56 + 4 * s15 * s16 * s25 * s56 +
        8 * pow<2>(s16) * s25 * s56 - 8 * mm2 * pow<2>(s25) * s56 -
        4 * s16 * pow<2>(s25) * s56 + 24 * pow<2>(mm2) * s26 * s56 +
        8 * mm2 * s15 * s26 * s56 - 12 * mm2 * s16 * s26 * s56 +
        4 * s12 * s16 * s26 * s56 + 4 * pow<2>(s16) * s26 * s56 -
        8 * mm2 * s25 * s26 * s56 - 4 * s16 * s25 * s26 * s56 -
        8 * me2 * mm2 * pow<2>(s56) + 4 * mm2 * s12 * pow<2>(s56) -
        4 * me2 * s16 * pow<2>(s56) + 4 * s12 * s16 * pow<2>(s56) -
        8 * mm2 * s25 * pow<2>(s56) - 4 * s16 * s25 * pow<2>(s56) -
        4 * mm2 * s26 * pow<2>(s56) - 2 * s16 * s26 * pow<2>(s56)};
    const auto if16{
        -16 * pow<2>(me2) * mm2 * s12 + 12 * pow<2>(me2) * pow<2>(s12) +
        12 * me2 * mm2 * pow<2>(s12) - 8 * me2 * pow<3>(s12) -
        16 * pow<2>(me2) * mm2 * s15 + 24 * pow<2>(me2) * s12 * s15 +
        24 * me2 * mm2 * s12 * s15 - 24 * me2 * pow<2>(s12) * s15 +
        12 * pow<2>(me2) * pow<2>(s15) + 12 * me2 * mm2 * pow<2>(s15) -
        24 * me2 * s12 * pow<2>(s15) - 8 * me2 * pow<3>(s15) +
        12 * pow<3>(me2) * s16 - 4 * pow<2>(me2) * mm2 * s16 +
        8 * pow<2>(me2) * s12 * s16 + 12 * me2 * mm2 * s12 * s16 -
        16 * me2 * pow<2>(s12) * s16 + 8 * pow<2>(me2) * s15 * s16 +
        12 * me2 * mm2 * s15 * s16 - 32 * me2 * s12 * s15 * s16 -
        16 * me2 * pow<2>(s15) * s16 - 8 * me2 * s12 * pow<2>(s16) -
        8 * me2 * s15 * pow<2>(s16) - 4 * pow<2>(me2) * mm2 * s25 -
        12 * me2 * pow<2>(mm2) * s25 - 4 * me2 * mm2 * s12 * s25 +
        10 * me2 * pow<2>(s12) * s25 + 16 * me2 * s12 * s15 * s25 -
        6 * mm2 * s12 * s15 * s25 + 4 * pow<2>(s12) * s15 * s25 +
        6 * me2 * pow<2>(s15) * s25 - 6 * mm2 * pow<2>(s15) * s25 +
        8 * s12 * pow<2>(s15) * s25 + 4 * pow<3>(s15) * s25 +
        20 * pow<2>(me2) * s16 * s25 + 8 * me2 * mm2 * s16 * s25 +
        6 * me2 * s12 * s16 * s25 + 4 * me2 * s15 * s16 * s25 -
        6 * mm2 * s15 * s16 * s25 + 8 * s12 * s15 * s16 * s25 +
        8 * pow<2>(s15) * s16 * s25 + 4 * s15 * pow<2>(s16) * s25 -
        2 * me2 * mm2 * pow<2>(s25) + 6 * pow<2>(mm2) * pow<2>(s25) -
        4 * mm2 * s12 * pow<2>(s25) - 4 * s12 * s15 * pow<2>(s25) -
        4 * pow<2>(s15) * pow<2>(s25) + 7 * me2 * s16 * pow<2>(s25) -
        5 * mm2 * s16 * pow<2>(s25) - 2 * s15 * s16 * pow<2>(s25) +
        2 * mm2 * pow<3>(s25) - s16 * pow<3>(s25) +
        4 * pow<2>(me2) * mm2 * s26 - 12 * me2 * pow<2>(mm2) * s26 -
        12 * pow<2>(me2) * s12 * s26 - 8 * me2 * mm2 * s12 * s26 +
        16 * me2 * pow<2>(s12) * s26 - 12 * pow<2>(me2) * s15 * s26 -
        8 * me2 * mm2 * s15 * s26 + 32 * me2 * s12 * s15 * s26 +
        16 * me2 * pow<2>(s15) * s26 + 8 * pow<2>(me2) * s16 * s26 +
        8 * me2 * mm2 * s16 * s26 + 12 * me2 * s12 * s16 * s26 +
        12 * me2 * s15 * s16 * s26 - 4 * me2 * mm2 * s25 * s26 -
        10 * me2 * s12 * s25 * s26 - 7 * me2 * s15 * s25 * s26 +
        7 * mm2 * s15 * s25 * s26 - 8 * s12 * s15 * s25 * s26 -
        8 * pow<2>(s15) * s25 * s26 + 6 * me2 * s16 * s25 * s26 -
        6 * s15 * s16 * s25 * s26 + 4 * mm2 * pow<2>(s25) * s26 +
        3 * s15 * pow<2>(s25) * s26 - s16 * pow<2>(s25) * s26 -
        8 * me2 * s12 * pow<2>(s26) - 8 * me2 * s15 * pow<2>(s26) +
        3 * s15 * s25 * pow<2>(s26) + 4 * pow<2>(me2) * mm2 * s56 -
        12 * me2 * pow<2>(mm2) * s56 - 12 * pow<2>(me2) * s12 * s56 -
        8 * me2 * mm2 * s12 * s56 + 16 * me2 * pow<2>(s12) * s56 -
        12 * pow<2>(me2) * s15 * s56 - 8 * me2 * mm2 * s15 * s56 +
        32 * me2 * s12 * s15 * s56 + 16 * me2 * pow<2>(s15) * s56 +
        8 * pow<2>(me2) * s16 * s56 + 8 * me2 * mm2 * s16 * s56 +
        12 * me2 * s12 * s16 * s56 + 12 * me2 * s15 * s16 * s56 -
        6 * me2 * mm2 * s25 * s56 + 6 * pow<2>(mm2) * s25 * s56 -
        9 * me2 * s12 * s25 * s56 - 3 * mm2 * s12 * s25 * s56 -
        6 * me2 * s15 * s25 * s56 + 4 * mm2 * s15 * s25 * s56 -
        8 * s12 * s15 * s25 * s56 - 8 * pow<2>(s15) * s25 * s56 +
        6 * me2 * s16 * s25 * s56 - 4 * mm2 * s16 * s25 * s56 -
        6 * s15 * s16 * s25 * s56 + 2 * mm2 * pow<2>(s25) * s56 +
        s12 * pow<2>(s25) * s56 + 4 * s15 * pow<2>(s25) * s56 -
        s16 * pow<2>(s25) * s56 - 16 * me2 * s12 * s26 * s56 -
        16 * me2 * s15 * s26 * s56 + s12 * s25 * s26 * s56 +
        7 * s15 * s25 * s26 * s56 - 8 * me2 * s12 * pow<2>(s56) -
        8 * me2 * s15 * pow<2>(s56) + s12 * s25 * pow<2>(s56) +
        4 * s15 * s25 * pow<2>(s56)};
    const auto if26{
        -16 * pow<2>(me2) * mm2 * s12 + 12 * pow<2>(me2) * pow<2>(s12) +
        12 * me2 * mm2 * pow<2>(s12) - 8 * me2 * pow<3>(s12) +
        12 * pow<2>(me2) * s12 * s15 + 12 * me2 * mm2 * s12 * s15 -
        16 * me2 * pow<2>(s12) * s15 - 8 * me2 * s12 * pow<2>(s15) +
        12 * pow<3>(me2) * s16 - 4 * pow<2>(me2) * mm2 * s16 +
        8 * pow<2>(me2) * s12 * s16 + 12 * me2 * mm2 * s12 * s16 -
        16 * me2 * pow<2>(s12) * s16 - 2 * pow<2>(me2) * s15 * s16 +
        6 * me2 * mm2 * s15 * s16 - 20 * me2 * s12 * s15 * s16 -
        4 * me2 * pow<2>(s15) * s16 - 8 * me2 * s12 * pow<2>(s16) -
        4 * me2 * s15 * pow<2>(s16) - 16 * me2 * mm2 * s12 * s25 +
        12 * me2 * pow<2>(s12) * s25 + 10 * me2 * s12 * s15 * s25 -
        6 * mm2 * s12 * s15 * s25 + 4 * pow<2>(s12) * s15 * s25 +
        4 * s12 * pow<2>(s15) * s25 + 18 * pow<2>(me2) * s16 * s25 -
        6 * me2 * mm2 * s16 * s25 + 12 * me2 * s12 * s16 * s25 +
        3 * me2 * s15 * s16 * s25 - 3 * mm2 * s15 * s16 * s25 +
        6 * s12 * s15 * s16 * s25 + 2 * pow<2>(s15) * s16 * s25 +
        2 * me2 * pow<2>(s16) * s25 - 3 * mm2 * pow<2>(s16) * s25 +
        2 * s12 * pow<2>(s16) * s25 + 4 * s15 * pow<2>(s16) * s25 +
        2 * pow<3>(s16) * s25 + 4 * mm2 * s12 * pow<2>(s25) -
        2 * pow<2>(s12) * pow<2>(s25) - 4 * s12 * s15 * pow<2>(s25) +
        3 * me2 * s16 * pow<2>(s25) + mm2 * s16 * pow<2>(s25) -
        2 * s12 * s16 * pow<2>(s25) - 2 * s16 * pow<3>(s25) +
        8 * pow<2>(me2) * mm2 * s26 - 12 * pow<2>(me2) * s12 * s26 -
        20 * me2 * mm2 * s12 * s26 + 18 * me2 * pow<2>(s12) * s26 -
        6 * pow<2>(me2) * s15 * s26 - 10 * me2 * mm2 * s15 * s26 +
        24 * me2 * s12 * s15 * s26 + 7 * me2 * pow<2>(s15) * s26 +
        3 * mm2 * pow<2>(s15) * s26 - 2 * s12 * pow<2>(s15) * s26 -
        2 * pow<3>(s15) * s26 + 12 * pow<2>(me2) * s16 * s26 -
        8 * me2 * mm2 * s16 * s26 + 18 * me2 * s12 * s16 * s26 +
        6 * mm2 * s12 * s16 * s26 - 4 * pow<2>(s12) * s16 * s26 +
        10 * me2 * s15 * s16 * s26 + 3 * mm2 * s15 * s16 * s26 -
        6 * s12 * s15 * s16 * s26 - 4 * pow<2>(s15) * s16 * s26 +
        2 * me2 * pow<2>(s16) * s26 - 4 * s12 * pow<2>(s16) * s26 -
        2 * s15 * pow<2>(s16) * s26 + 6 * me2 * mm2 * s25 * s26 -
        6 * pow<2>(mm2) * s25 * s26 - 12 * me2 * s12 * s25 * s26 +
        8 * mm2 * s12 * s25 * s26 - 2 * pow<2>(s12) * s25 * s26 -
        5 * me2 * s15 * s25 * s26 + 5 * mm2 * s15 * s25 * s26 -
        6 * s12 * s15 * s25 * s26 + 6 * me2 * s16 * s25 * s26 +
        7 * mm2 * s16 * s25 * s26 - 2 * s12 * s16 * s25 * s26 -
        2 * s15 * s16 * s25 * s26 - 2 * pow<2>(s16) * s25 * s26 -
        4 * mm2 * pow<2>(s25) * s26 + 2 * s12 * pow<2>(s25) * s26 +
        2 * s15 * pow<2>(s25) * s26 - 2 * s16 * pow<2>(s25) * s26 +
        6 * me2 * mm2 * pow<2>(s26) - 6 * pow<2>(mm2) * pow<2>(s26) -
        10 * me2 * s12 * pow<2>(s26) + 4 * mm2 * s12 * pow<2>(s26) -
        6 * me2 * s15 * pow<2>(s26) + 3 * mm2 * s15 * pow<2>(s26) +
        2 * pow<2>(s15) * pow<2>(s26) + 2 * me2 * s16 * pow<2>(s26) +
        4 * mm2 * s16 * pow<2>(s26) + 2 * s12 * s16 * pow<2>(s26) +
        2 * s15 * s16 * pow<2>(s26) - 6 * mm2 * s25 * pow<2>(s26) +
        2 * s12 * s25 * pow<2>(s26) + 2 * s15 * s25 * pow<2>(s26) -
        2 * mm2 * pow<3>(s26) - 6 * pow<2>(me2) * s12 * s56 -
        14 * me2 * mm2 * s12 * s56 + 14 * me2 * pow<2>(s12) * s56 +
        13 * me2 * s12 * s15 * s56 - 3 * mm2 * s12 * s15 * s56 +
        2 * pow<2>(s12) * s15 * s56 + 2 * s12 * pow<2>(s15) * s56 +
        10 * pow<2>(me2) * s16 * s56 - 8 * me2 * mm2 * s16 * s56 +
        16 * me2 * s12 * s16 * s56 + 3 * mm2 * s12 * s16 * s56 -
        2 * pow<2>(s12) * s16 * s56 + 8 * me2 * s15 * s16 * s56 +
        4 * me2 * pow<2>(s16) * s56 - 2 * s12 * pow<2>(s16) * s56 -
        5 * me2 * s12 * s25 * s56 + 7 * mm2 * s12 * s25 * s56 -
        4 * pow<2>(s12) * s25 * s56 - 8 * s12 * s15 * s25 * s56 +
        3 * me2 * s16 * s25 * s56 + 2 * mm2 * s16 * s25 * s56 -
        6 * s12 * s16 * s25 * s56 - 2 * s15 * s16 * s25 * s56 -
        2 * pow<2>(s16) * s25 * s56 + 2 * s12 * pow<2>(s25) * s56 -
        2 * s16 * pow<2>(s25) * s56 + 8 * me2 * mm2 * s26 * s56 -
        6 * pow<2>(mm2) * s26 * s56 - 16 * me2 * s12 * s26 * s56 +
        5 * mm2 * s12 * s26 * s56 - 7 * me2 * s15 * s26 * s56 +
        2 * mm2 * s15 * s26 * s56 - 2 * s12 * s15 * s26 * s56 +
        2 * pow<2>(s15) * s26 * s56 + 2 * me2 * s16 * s26 * s56 +
        4 * mm2 * s16 * s26 * s56 + 2 * s12 * s16 * s26 * s56 +
        2 * s15 * s16 * s26 * s56 - 4 * mm2 * s25 * s26 * s56 +
        4 * s12 * s25 * s26 * s56 + 2 * s15 * s25 * s26 * s56 -
        4 * mm2 * pow<2>(s26) * s56 - 2 * me2 * mm2 * pow<2>(s56) -
        5 * me2 * s12 * pow<2>(s56) + 2 * mm2 * s12 * pow<2>(s56) -
        2 * s12 * s15 * pow<2>(s56) + 2 * s12 * s25 * pow<2>(s56) -
        2 * mm2 * s26 * pow<2>(s56)};
    const auto if36{
        16 * me2 * pow<2>(mm2) * s12 - 12 * me2 * mm2 * pow<2>(s12) -
        12 * pow<2>(mm2) * pow<2>(s12) + 8 * mm2 * pow<3>(s12) +
        8 * me2 * pow<2>(mm2) * s15 - 20 * me2 * mm2 * s12 * s15 -
        12 * pow<2>(mm2) * s12 * s15 + 18 * mm2 * pow<2>(s12) * s15 +
        6 * pow<2>(me2) * pow<2>(s15) - 6 * me2 * mm2 * pow<2>(s15) -
        4 * me2 * s12 * pow<2>(s15) + 10 * mm2 * s12 * pow<2>(s15) -
        2 * me2 * pow<3>(s15) - 16 * me2 * mm2 * s12 * s16 +
        12 * mm2 * pow<2>(s12) * s16 + 6 * pow<2>(me2) * s15 * s16 -
        6 * me2 * mm2 * s15 * s16 - 8 * me2 * s12 * s15 * s16 +
        12 * mm2 * s12 * s15 * s16 + 2 * pow<2>(s12) * s15 * s16 -
        6 * me2 * pow<2>(s15) * s16 + 2 * s12 * pow<2>(s15) * s16 -
        4 * me2 * s12 * pow<2>(s16) + 2 * pow<2>(s12) * pow<2>(s16) -
        4 * me2 * s15 * pow<2>(s16) + 2 * s12 * s15 * pow<2>(s16) -
        4 * me2 * pow<2>(mm2) * s25 + 12 * pow<3>(mm2) * s25 +
        12 * me2 * mm2 * s12 * s25 + 8 * pow<2>(mm2) * s12 * s25 -
        16 * mm2 * pow<2>(s12) * s25 + 8 * me2 * mm2 * s15 * s25 -
        12 * pow<2>(mm2) * s15 * s25 - 6 * me2 * s12 * s15 * s25 -
        18 * mm2 * s12 * s15 * s25 + 4 * pow<2>(s12) * s15 * s25 +
        4 * me2 * pow<2>(s15) * s25 + 2 * mm2 * pow<2>(s15) * s25 +
        2 * s12 * pow<2>(s15) * s25 + 6 * me2 * mm2 * s16 * s25 -
        18 * pow<2>(mm2) * s16 * s25 - 12 * mm2 * s12 * s16 * s25 +
        7 * me2 * s15 * s16 * s25 + 6 * mm2 * s15 * s16 * s25 -
        2 * s12 * s15 * s16 * s25 + me2 * pow<2>(s16) * s25 +
        3 * mm2 * pow<2>(s16) * s25 - 2 * s12 * pow<2>(s16) * s25 +
        2 * s15 * pow<2>(s16) * s25 + 2 * pow<3>(s16) * s25 +
        8 * mm2 * s12 * pow<2>(s25) + 2 * mm2 * s15 * pow<2>(s25) -
        4 * s12 * s15 * pow<2>(s25) - 3 * me2 * s16 * pow<2>(s25) +
        2 * mm2 * s16 * pow<2>(s25) + 2 * s12 * s16 * pow<2>(s25) +
        2 * s15 * s16 * pow<2>(s25) - 2 * s16 * pow<3>(s25) +
        12 * me2 * mm2 * s12 * s26 + 12 * pow<2>(mm2) * s12 * s26 -
        16 * mm2 * pow<2>(s12) * s26 + 10 * me2 * mm2 * s15 * s26 +
        6 * pow<2>(mm2) * s15 * s26 - 24 * mm2 * s12 * s15 * s26 +
        3 * me2 * pow<2>(s15) * s26 - 6 * mm2 * pow<2>(s15) * s26 +
        6 * me2 * s12 * s16 * s26 - 10 * mm2 * s12 * s16 * s26 -
        4 * pow<2>(s12) * s16 * s26 + 5 * me2 * s15 * s16 * s26 -
        5 * mm2 * s15 * s16 * s26 - 6 * s12 * s15 * s16 * s26 -
        2 * pow<2>(s15) * s16 * s26 - 4 * s12 * pow<2>(s16) * s26 -
        2 * s15 * pow<2>(s16) * s26 - 6 * me2 * mm2 * s25 * s26 +
        2 * pow<2>(mm2) * s25 * s26 + 20 * mm2 * s12 * s25 * s26 +
        3 * me2 * s15 * s25 * s26 + 10 * mm2 * s15 * s25 * s26 -
        6 * s12 * s15 * s25 * s26 - 2 * pow<2>(s15) * s25 * s26 -
        3 * me2 * s16 * s25 * s26 + 3 * mm2 * s16 * s25 * s26 +
        6 * s12 * s16 * s25 * s26 + 2 * s15 * s16 * s25 * s26 -
        4 * mm2 * pow<2>(s25) * s26 + 2 * s15 * pow<2>(s25) * s26 -
        4 * s16 * pow<2>(s25) * s26 + 8 * mm2 * s12 * pow<2>(s26) +
        3 * me2 * s15 * pow<2>(s26) + 7 * mm2 * s15 * pow<2>(s26) -
        2 * s12 * s15 * pow<2>(s26) - 2 * pow<2>(s15) * pow<2>(s26) +
        4 * s12 * s16 * pow<2>(s26) - 4 * mm2 * s25 * pow<2>(s26) +
        4 * s15 * s25 * pow<2>(s26) - 2 * s16 * s25 * pow<2>(s26) +
        2 * s15 * pow<3>(s26) + 14 * me2 * mm2 * s12 * s56 +
        6 * pow<2>(mm2) * s12 * s56 - 14 * mm2 * pow<2>(s12) * s56 -
        6 * pow<2>(me2) * s15 * s56 + 8 * me2 * mm2 * s15 * s56 +
        5 * me2 * s12 * s15 * s56 - 16 * mm2 * s12 * s15 * s56 +
        4 * me2 * pow<2>(s15) * s56 + 7 * me2 * s12 * s16 * s56 -
        5 * mm2 * s12 * s16 * s56 - 4 * pow<2>(s12) * s16 * s56 +
        4 * me2 * s15 * s16 * s56 - 4 * s12 * s15 * s16 * s56 -
        2 * s12 * pow<2>(s16) * s56 - 8 * me2 * mm2 * s25 * s56 +
        10 * pow<2>(mm2) * s25 * s56 + 3 * me2 * s12 * s25 * s56 +
        16 * mm2 * s12 * s25 * s56 - 2 * pow<2>(s12) * s25 * s56 -
        4 * me2 * s15 * s25 * s56 - 2 * mm2 * s15 * s25 * s56 -
        2 * s12 * s15 * s25 * s56 - 2 * me2 * s16 * s25 * s56 -
        3 * mm2 * s16 * s25 * s56 + 6 * s12 * s16 * s25 * s56 -
        2 * pow<2>(s16) * s25 * s56 - 4 * mm2 * pow<2>(s25) * s56 +
        2 * s12 * pow<2>(s25) * s56 - 2 * s16 * pow<2>(s25) * s56 -
        3 * me2 * s12 * s26 * s56 + 13 * mm2 * s12 * s26 * s56 +
        2 * pow<2>(s12) * s26 * s56 - 2 * me2 * s15 * s26 * s56 +
        7 * mm2 * s15 * s26 * s56 + 2 * s12 * s15 * s26 * s56 +
        8 * s12 * s16 * s26 * s56 + 2 * s15 * s16 * s26 * s56 -
        8 * mm2 * s25 * s26 * s56 + 2 * s15 * s25 * s26 * s56 -
        2 * s16 * s25 * s26 * s56 - 2 * s12 * pow<2>(s26) * s56 +
        2 * s15 * pow<2>(s26) * s56 + 2 * me2 * mm2 * pow<2>(s56) -
        2 * me2 * s12 * pow<2>(s56) + 5 * mm2 * s12 * pow<2>(s56) -
        2 * me2 * s15 * pow<2>(s56) + 2 * s12 * s16 * pow<2>(s56) -
        2 * s12 * s26 * pow<2>(s56)};
    const auto if46{
        8 * me2 * mm2 * pow<2>(s12) - 6 * me2 * pow<3>(s12) -
        6 * mm2 * pow<3>(s12) + 4 * pow<4>(s12) - 6 * pow<2>(me2) * s12 * s15 +
        2 * me2 * mm2 * s12 * s15 - 4 * me2 * pow<2>(s12) * s15 -
        6 * mm2 * pow<2>(s12) * s15 + 8 * pow<3>(s12) * s15 +
        4 * pow<2>(s12) * pow<2>(s15) - 6 * pow<2>(me2) * s12 * s16 +
        2 * me2 * mm2 * s12 * s16 - 4 * me2 * pow<2>(s12) * s16 -
        6 * mm2 * pow<2>(s12) * s16 + 8 * pow<3>(s12) * s16 -
        12 * pow<2>(me2) * s15 * s16 - 4 * me2 * mm2 * s15 * s16 +
        8 * me2 * s12 * s15 * s16 - 6 * mm2 * s12 * s15 * s16 +
        12 * pow<2>(s12) * s15 * s16 + 4 * me2 * pow<2>(s15) * s16 +
        4 * s12 * pow<2>(s15) * s16 + 4 * pow<2>(s12) * pow<2>(s16) +
        4 * me2 * s15 * pow<2>(s16) + 4 * s12 * s15 * pow<2>(s16) -
        2 * me2 * mm2 * s12 * s25 + 6 * pow<2>(mm2) * s12 * s25 +
        6 * me2 * pow<2>(s12) * s25 + 4 * mm2 * pow<2>(s12) * s25 -
        8 * pow<3>(s12) * s25 - 4 * me2 * s12 * s15 * s25 -
        4 * mm2 * s12 * s15 * s25 - 6 * pow<2>(s12) * s15 * s25 +
        3 * pow<2>(me2) * s16 * s25 - 2 * me2 * mm2 * s16 * s25 +
        3 * pow<2>(mm2) * s16 * s25 - 12 * pow<2>(s12) * s16 * s25 -
        9 * me2 * s15 * s16 * s25 - 2 * mm2 * s15 * s16 * s25 -
        4 * s12 * s15 * s16 * s25 + me2 * pow<2>(s16) * s25 -
        2 * mm2 * pow<2>(s16) * s25 - 4 * s12 * pow<2>(s16) * s25 +
        4 * pow<2>(s12) * pow<2>(s25) + 2 * me2 * s16 * pow<2>(s25) -
        mm2 * s16 * pow<2>(s25) + 4 * s12 * s16 * pow<2>(s25) -
        2 * me2 * mm2 * s12 * s26 + 6 * pow<2>(mm2) * s12 * s26 +
        6 * me2 * pow<2>(s12) * s26 + 4 * mm2 * pow<2>(s12) * s26 -
        8 * pow<3>(s12) * s26 + 3 * pow<2>(me2) * s15 * s26 -
        2 * me2 * mm2 * s15 * s26 + 3 * pow<2>(mm2) * s15 * s26 -
        12 * pow<2>(s12) * s15 * s26 + me2 * pow<2>(s15) * s26 -
        2 * mm2 * pow<2>(s15) * s26 - 4 * s12 * pow<2>(s15) * s26 -
        4 * me2 * s12 * s16 * s26 - 4 * mm2 * s12 * s16 * s26 -
        6 * pow<2>(s12) * s16 * s26 - 9 * me2 * s15 * s16 * s26 -
        2 * mm2 * s15 * s16 * s26 - 4 * s12 * s15 * s16 * s26 -
        4 * me2 * mm2 * s25 * s26 - 12 * pow<2>(mm2) * s25 * s26 -
        6 * me2 * s12 * s25 * s26 + 8 * mm2 * s12 * s25 * s26 +
        12 * pow<2>(s12) * s25 * s26 + 2 * me2 * s15 * s25 * s26 +
        9 * mm2 * s15 * s25 * s26 + 4 * s12 * s15 * s25 * s26 +
        2 * me2 * s16 * s25 * s26 + 9 * mm2 * s16 * s25 * s26 +
        4 * s12 * s16 * s25 * s26 - 4 * mm2 * pow<2>(s25) * s26 -
        4 * s12 * pow<2>(s25) * s26 + 4 * pow<2>(s12) * pow<2>(s26) +
        2 * me2 * s15 * pow<2>(s26) - mm2 * s15 * pow<2>(s26) +
        4 * s12 * s15 * pow<2>(s26) - 4 * mm2 * s25 * pow<2>(s26) -
        4 * s12 * s25 * pow<2>(s26) + 8 * pow<2>(me2) * mm2 * s56 +
        8 * me2 * pow<2>(mm2) * s56 + 3 * pow<2>(me2) * s12 * s56 -
        6 * me2 * mm2 * s12 * s56 + 3 * pow<2>(mm2) * s12 * s56 -
        4 * pow<3>(s12) * s56 - 6 * me2 * mm2 * s15 * s56 -
        3 * me2 * s12 * s15 * s56 - 2 * mm2 * s12 * s15 * s56 -
        4 * pow<2>(s12) * s15 * s56 - 6 * me2 * mm2 * s16 * s56 -
        3 * me2 * s12 * s16 * s56 - 2 * mm2 * s12 * s16 * s56 -
        4 * pow<2>(s12) * s16 * s56 - 8 * me2 * s15 * s16 * s56 -
        2 * s12 * s15 * s16 * s56 + 6 * me2 * mm2 * s25 * s56 +
        2 * me2 * s12 * s25 * s56 + 3 * mm2 * s12 * s25 * s56 +
        4 * pow<2>(s12) * s25 * s56 + me2 * s16 * s25 * s56 +
        mm2 * s16 * s25 * s56 + 4 * s12 * s16 * s25 * s56 +
        6 * me2 * mm2 * s26 * s56 + 2 * me2 * s12 * s26 * s56 +
        3 * mm2 * s12 * s26 * s56 + 4 * pow<2>(s12) * s26 * s56 +
        me2 * s15 * s26 * s56 + mm2 * s15 * s26 * s56 +
        4 * s12 * s15 * s26 * s56 - 8 * mm2 * s25 * s26 * s56 -
        2 * s12 * s25 * s26 * s56 + 8 * me2 * mm2 * pow<2>(s56) +
        me2 * s12 * pow<2>(s56) + mm2 * s12 * pow<2>(s56)};
    const auto if56{
        16 * me2 * pow<2>(mm2) * s12 - 12 * me2 * mm2 * pow<2>(s12) -
        12 * pow<2>(mm2) * pow<2>(s12) + 8 * mm2 * pow<3>(s12) -
        12 * pow<2>(me2) * mm2 * s15 + 4 * me2 * pow<2>(mm2) * s15 -
        8 * me2 * mm2 * s12 * s15 - 12 * pow<2>(mm2) * s12 * s15 +
        16 * mm2 * pow<2>(s12) * s15 + 8 * mm2 * s12 * pow<2>(s15) -
        12 * pow<2>(me2) * mm2 * s16 - 4 * me2 * pow<2>(mm2) * s16 -
        4 * me2 * mm2 * s12 * s16 + 10 * mm2 * pow<2>(s12) * s16 +
        4 * me2 * mm2 * s15 * s16 + 10 * mm2 * s12 * s15 * s16 -
        6 * pow<2>(me2) * pow<2>(s16) + 2 * me2 * mm2 * pow<2>(s16) +
        4 * me2 * s12 * pow<2>(s16) + 4 * me2 * s15 * pow<2>(s16) +
        2 * me2 * pow<3>(s16) - 4 * me2 * pow<2>(mm2) * s25 +
        12 * pow<3>(mm2) * s25 + 12 * me2 * mm2 * s12 * s25 +
        8 * pow<2>(mm2) * s12 * s25 - 16 * mm2 * pow<2>(s12) * s25 -
        8 * me2 * mm2 * s15 * s25 - 8 * pow<2>(mm2) * s15 * s25 -
        12 * mm2 * s12 * s15 * s25 - 8 * me2 * mm2 * s16 * s25 -
        20 * pow<2>(mm2) * s16 * s25 - 6 * mm2 * s12 * s16 * s25 +
        6 * mm2 * s15 * s16 * s25 - 5 * me2 * pow<2>(s16) * s25 +
        7 * mm2 * pow<2>(s16) * s25 + s15 * pow<2>(s16) * s25 +
        pow<3>(s16) * s25 + 8 * mm2 * s12 * pow<2>(s25) -
        16 * me2 * pow<2>(mm2) * s26 + 24 * me2 * mm2 * s12 * s26 +
        24 * pow<2>(mm2) * s12 * s26 - 24 * mm2 * pow<2>(s12) * s26 +
        8 * me2 * mm2 * s15 * s26 + 12 * pow<2>(mm2) * s15 * s26 -
        32 * mm2 * s12 * s15 * s26 - 8 * mm2 * pow<2>(s15) * s26 +
        6 * me2 * s12 * s16 * s26 - 16 * mm2 * s12 * s16 * s26 -
        4 * pow<2>(s12) * s16 * s26 + 7 * me2 * s15 * s16 * s26 -
        7 * mm2 * s15 * s16 * s26 - 8 * s12 * s15 * s16 * s26 -
        3 * pow<2>(s15) * s16 * s26 - 4 * s12 * pow<2>(s16) * s26 -
        3 * s15 * pow<2>(s16) * s26 - 12 * me2 * mm2 * s25 * s26 -
        8 * pow<2>(mm2) * s25 * s26 + 32 * mm2 * s12 * s25 * s26 +
        12 * mm2 * s15 * s25 * s26 - 6 * me2 * s16 * s25 * s26 +
        4 * mm2 * s16 * s25 * s26 + 8 * s12 * s16 * s25 * s26 +
        6 * s15 * s16 * s25 * s26 + 2 * pow<2>(s16) * s25 * s26 -
        8 * mm2 * pow<2>(s25) * s26 - 4 * s16 * pow<2>(s25) * s26 -
        12 * me2 * mm2 * pow<2>(s26) - 12 * pow<2>(mm2) * pow<2>(s26) +
        24 * mm2 * s12 * pow<2>(s26) + 16 * mm2 * s15 * pow<2>(s26) -
        6 * me2 * s16 * pow<2>(s26) + 6 * mm2 * s16 * pow<2>(s26) +
        8 * s12 * s16 * pow<2>(s26) + 8 * s15 * s16 * pow<2>(s26) +
        4 * pow<2>(s16) * pow<2>(s26) - 16 * mm2 * s25 * pow<2>(s26) -
        8 * s16 * s25 * pow<2>(s26) - 8 * mm2 * pow<3>(s26) -
        4 * s16 * pow<3>(s26) + 12 * pow<2>(me2) * mm2 * s56 -
        4 * me2 * pow<2>(mm2) * s56 + 8 * me2 * mm2 * s12 * s56 +
        12 * pow<2>(mm2) * s12 * s56 - 16 * mm2 * pow<2>(s12) * s56 -
        16 * mm2 * s12 * s15 * s56 + 6 * pow<2>(me2) * s16 * s56 -
        6 * me2 * mm2 * s16 * s56 - 3 * me2 * s12 * s16 * s56 -
        9 * mm2 * s12 * s16 * s56 - s12 * s15 * s16 * s56 -
        2 * me2 * pow<2>(s16) * s56 - s12 * pow<2>(s16) * s56 +
        8 * me2 * mm2 * s25 * s56 + 8 * pow<2>(mm2) * s25 * s56 +
        12 * mm2 * s12 * s25 * s56 + 4 * me2 * s16 * s25 * s56 -
        6 * mm2 * s16 * s25 * s56 - pow<2>(s16) * s25 * s56 -
        8 * me2 * mm2 * s26 * s56 - 12 * pow<2>(mm2) * s26 * s56 +
        32 * mm2 * s12 * s26 * s56 + 16 * mm2 * s15 * s26 * s56 -
        4 * me2 * s16 * s26 * s56 + 6 * mm2 * s16 * s26 * s56 +
        8 * s12 * s16 * s26 * s56 + 7 * s15 * s16 * s26 * s56 +
        4 * pow<2>(s16) * s26 * s56 - 12 * mm2 * s25 * s26 * s56 -
        6 * s16 * s25 * s26 * s56 - 16 * mm2 * pow<2>(s26) * s56 -
        8 * s16 * pow<2>(s26) * s56 + 8 * mm2 * s12 * pow<2>(s56) +
        s12 * s16 * pow<2>(s56) - 8 * mm2 * s26 * pow<2>(s56) -
        4 * s16 * s26 * pow<2>(s56)};
    const auto if66{
        32 * pow<2>(me2) * pow<2>(mm2) - 24 * pow<2>(me2) * mm2 * s12 -
        24 * me2 * pow<2>(mm2) * s12 + 16 * me2 * mm2 * pow<2>(s12) -
        24 * pow<2>(me2) * mm2 * s15 - 24 * me2 * pow<2>(mm2) * s15 +
        32 * me2 * mm2 * s12 * s15 + 16 * me2 * mm2 * pow<2>(s15) -
        32 * pow<2>(me2) * mm2 * s16 + 24 * me2 * mm2 * s12 * s16 +
        24 * me2 * mm2 * s15 * s16 - 8 * pow<2>(me2) * pow<2>(s16) +
        4 * me2 * s12 * pow<2>(s16) + 4 * me2 * s15 * pow<2>(s16) +
        32 * me2 * pow<2>(mm2) * s25 - 24 * me2 * mm2 * s12 * s25 -
        20 * me2 * mm2 * s15 * s25 + 12 * pow<2>(mm2) * s15 * s25 -
        8 * mm2 * s12 * s15 * s25 - 8 * mm2 * pow<2>(s15) * s25 -
        32 * me2 * mm2 * s16 * s25 - 12 * mm2 * s15 * s16 * s25 -
        8 * me2 * pow<2>(s16) * s25 - 2 * s15 * pow<2>(s16) * s25 -
        8 * pow<2>(mm2) * pow<2>(s25) + 4 * mm2 * s12 * pow<2>(s25) +
        8 * mm2 * s15 * pow<2>(s25) + 8 * mm2 * s16 * pow<2>(s25) +
        2 * pow<2>(s16) * pow<2>(s25) + 24 * pow<2>(me2) * mm2 * s26 +
        24 * me2 * pow<2>(mm2) * s26 - 32 * me2 * mm2 * s12 * s26 -
        32 * me2 * mm2 * s15 * s26 + 12 * pow<2>(me2) * s16 * s26 -
        20 * me2 * mm2 * s16 * s26 - 8 * me2 * s12 * s16 * s26 -
        8 * me2 * s15 * s16 * s26 - 8 * me2 * pow<2>(s16) * s26 +
        24 * me2 * mm2 * s25 * s26 + 8 * mm2 * s15 * s25 * s26 +
        12 * me2 * s16 * s25 * s26 + 2 * s15 * s16 * s25 * s26 -
        4 * mm2 * pow<2>(s25) * s26 - 2 * s16 * pow<2>(s25) * s26 +
        16 * me2 * mm2 * pow<2>(s26) + 8 * me2 * s16 * pow<2>(s26) +
        24 * pow<2>(me2) * mm2 * s56 + 24 * me2 * pow<2>(mm2) * s56 -
        32 * me2 * mm2 * s12 * s56 - 32 * me2 * mm2 * s15 * s56 +
        12 * pow<2>(me2) * s16 * s56 - 20 * me2 * mm2 * s16 * s56 -
        8 * me2 * s12 * s16 * s56 - 8 * me2 * s15 * s16 * s56 -
        8 * me2 * pow<2>(s16) * s56 + 20 * me2 * mm2 * s25 * s56 -
        12 * pow<2>(mm2) * s25 * s56 + 8 * mm2 * s12 * s25 * s56 +
        16 * mm2 * s15 * s25 * s56 + 10 * me2 * s16 * s25 * s56 +
        10 * mm2 * s16 * s25 * s56 + 2 * s12 * s16 * s25 * s56 +
        4 * s15 * s16 * s25 * s56 + 4 * pow<2>(s16) * s25 * s56 -
        8 * mm2 * pow<2>(s25) * s56 - 4 * s16 * pow<2>(s25) * s56 +
        32 * me2 * mm2 * s26 * s56 + 16 * me2 * s16 * s26 * s56 -
        8 * mm2 * s25 * s26 * s56 - 4 * s16 * s25 * s26 * s56 +
        16 * me2 * mm2 * pow<2>(s56) + 8 * me2 * s16 * pow<2>(s56) -
        8 * mm2 * s25 * pow<2>(s56) - 4 * s16 * s25 * pow<2>(s56)};

    return if11 / pow<2>(den1) + if22 / pow<2>(den2) + if33 / pow<2>(den3) +
           if44 / pow<2>(den4) + if55 / pow<2>(den5) + if66 / pow<2>(den6) +
           2 * (if12 / (den1 * den2) + if13 / (den1 * den3) +
                if14 / (den1 * den4) + if15 / (den1 * den5) +
                if16 / (den1 * den6) + if23 / (den2 * den3) +
                if24 / (den2 * den4) + if25 / (den2 * den5) +
                if26 / (den2 * den6) + if34 / (den3 * den4) +
                if35 / (den3 * den5) + if36 / (den3 * den6) +
                if45 / (den4 * den5) + if46 / (den4 * den6) +
                if56 / (den5 * den6));
}

auto DoubleRadiativeMuonDecay::MSqPolarizedS2n(double mm2, double me2, double s12, double s15, double s16, double s25, double s26, double s56,
                                               double den1, double den2, double den3, double den4, double den5, double den6) -> double {
    using muc::pow;

    // Adapt from McMule v0.5.0, mudec/mudec_pm2ennggav.f95
    //
    // Copyright 2020-2024  Yannick Ulrich and others (The McMule development team)
    //

    const auto if11{
        24 * pow<3>(me2) + 8 * pow<2>(me2) * mm2 - 16 * pow<2>(me2) * s12 -
        16 * pow<2>(me2) * s15 - 16 * pow<2>(me2) * s16 +
        36 * pow<2>(me2) * s25 + 4 * me2 * mm2 * s25 - 8 * me2 * s12 * s25 -
        8 * me2 * s15 * s25 - 4 * me2 * s16 * s25 + 18 * me2 * pow<2>(s25) +
        2 * mm2 * pow<2>(s25) - 4 * s12 * pow<2>(s25) - 2 * s15 * pow<2>(s25) -
        4 * s16 * pow<2>(s25) + 4 * pow<3>(s25) + 24 * pow<2>(me2) * s26 +
        4 * me2 * s16 * s26 + 8 * me2 * s25 * s26 + 2 * s15 * s25 * s26 +
        4 * pow<2>(s25) * s26 - 4 * me2 * pow<2>(s26) + 24 * pow<2>(me2) * s56 +
        4 * me2 * s16 * s56 + 14 * me2 * s25 * s56 + 2 * mm2 * s25 * s56 -
        4 * s12 * s25 * s56 - 2 * s15 * s25 * s56 - 4 * s16 * s25 * s56 +
        8 * pow<2>(s25) * s56 - 8 * me2 * s26 * s56 + 4 * s25 * s26 * s56 -
        4 * me2 * pow<2>(s56) + 4 * s25 * pow<2>(s56)};
    const auto if12{
        24 * pow<3>(me2) + 8 * pow<2>(me2) * mm2 - 16 * pow<2>(me2) * s12 -
        16 * pow<2>(me2) * s15 - 16 * pow<2>(me2) * s16 +
        30 * pow<2>(me2) * s25 + 2 * me2 * mm2 * s25 - 4 * me2 * s12 * s25 -
        4 * me2 * s15 * s25 - 4 * me2 * s16 * s25 + 6 * me2 * pow<2>(s25) +
        2 * s15 * pow<2>(s25) + 2 * s16 * pow<2>(s25) - 2 * pow<3>(s25) +
        30 * pow<2>(me2) * s26 + 2 * me2 * mm2 * s26 - 4 * me2 * s12 * s26 -
        4 * me2 * s15 * s26 - 4 * me2 * s16 * s26 + 12 * me2 * s25 * s26 +
        4 * s15 * s25 * s26 + 4 * s16 * s25 * s26 - 6 * pow<2>(s25) * s26 +
        6 * me2 * pow<2>(s26) + 2 * s15 * pow<2>(s26) + 2 * s16 * pow<2>(s26) -
        6 * s25 * pow<2>(s26) - 2 * pow<3>(s26) + 24 * pow<2>(me2) * s56 +
        12 * me2 * s25 * s56 + 2 * mm2 * s25 * s56 - 4 * s12 * s25 * s56 -
        2 * pow<2>(s25) * s56 + 12 * me2 * s26 * s56 + 2 * mm2 * s26 * s56 -
        4 * s12 * s26 * s56 - 4 * s25 * s26 * s56 - 2 * pow<2>(s26) * s56 +
        6 * me2 * pow<2>(s56) + 2 * mm2 * pow<2>(s56) - 4 * s12 * pow<2>(s56) -
        2 * s15 * pow<2>(s56) - 2 * s16 * pow<2>(s56) + 2 * s25 * pow<2>(s56) +
        2 * s26 * pow<2>(s56) + 2 * pow<3>(s56)};
    const auto if22{
        24 * pow<3>(me2) + 8 * pow<2>(me2) * mm2 - 16 * pow<2>(me2) * s12 -
        16 * pow<2>(me2) * s15 - 16 * pow<2>(me2) * s16 +
        24 * pow<2>(me2) * s25 + 4 * me2 * s15 * s25 - 4 * me2 * pow<2>(s25) +
        36 * pow<2>(me2) * s26 + 4 * me2 * mm2 * s26 - 8 * me2 * s12 * s26 -
        4 * me2 * s15 * s26 - 8 * me2 * s16 * s26 + 8 * me2 * s25 * s26 +
        2 * s16 * s25 * s26 + 18 * me2 * pow<2>(s26) + 2 * mm2 * pow<2>(s26) -
        4 * s12 * pow<2>(s26) - 4 * s15 * pow<2>(s26) - 2 * s16 * pow<2>(s26) +
        4 * s25 * pow<2>(s26) + 4 * pow<3>(s26) + 24 * pow<2>(me2) * s56 +
        4 * me2 * s15 * s56 - 8 * me2 * s25 * s56 + 14 * me2 * s26 * s56 +
        2 * mm2 * s26 * s56 - 4 * s12 * s26 * s56 - 4 * s15 * s26 * s56 -
        2 * s16 * s26 * s56 + 4 * s25 * s26 * s56 + 8 * pow<2>(s26) * s56 -
        4 * me2 * pow<2>(s56) + 4 * s26 * pow<2>(s56)};
    const auto if13{
        -6 * me2 * pow<2>(s12) - 2 * mm2 * pow<2>(s12) + 4 * pow<3>(s12) -
        6 * pow<2>(me2) * s15 - 2 * me2 * mm2 * s15 - 7 * me2 * s12 * s15 -
        3 * mm2 * s12 * s15 + 10 * pow<2>(s12) * s15 - me2 * pow<2>(s15) -
        mm2 * pow<2>(s15) + 7 * s12 * pow<2>(s15) + pow<3>(s15) -
        5 * me2 * s12 * s16 - mm2 * s12 * s16 + 6 * pow<2>(s12) * s16 -
        me2 * s15 * s16 - mm2 * s15 * s16 + 8 * s12 * s15 * s16 +
        2 * pow<2>(s15) * s16 + s12 * pow<2>(s16) + s15 * pow<2>(s16) +
        6 * me2 * mm2 * s25 + 2 * pow<2>(mm2) * s25 + 12 * me2 * s12 * s25 -
        2 * mm2 * s12 * s25 - 12 * pow<2>(s12) * s25 + 5 * me2 * s15 * s25 +
        mm2 * s15 * s25 - 19 * s12 * s15 * s25 - 7 * pow<2>(s15) * s25 +
        4 * me2 * s16 * s25 - 2 * mm2 * s16 * s25 - 10 * s12 * s16 * s25 -
        9 * s15 * s16 * s25 - pow<2>(s16) * s25 - 6 * me2 * pow<2>(s25) +
        4 * mm2 * pow<2>(s25) + 12 * s12 * pow<2>(s25) + 9 * s15 * pow<2>(s25) +
        4 * s16 * pow<2>(s25) - 4 * pow<3>(s25) + 6 * me2 * s12 * s26 -
        8 * pow<2>(s12) * s26 - 2 * me2 * s15 * s26 + 2 * mm2 * s15 * s26 -
        11 * s12 * s15 * s26 - 4 * pow<2>(s15) * s26 - 4 * s12 * s16 * s26 -
        5 * s15 * s16 * s26 - 6 * me2 * s25 * s26 + 4 * mm2 * s25 * s26 +
        16 * s12 * s25 * s26 + 12 * s15 * s25 * s26 + 4 * s16 * s25 * s26 -
        8 * pow<2>(s25) * s26 + 4 * s12 * pow<2>(s26) + 3 * s15 * pow<2>(s26) -
        4 * s25 * pow<2>(s26) + 3 * pow<2>(me2) * s56 + 4 * me2 * mm2 * s56 +
        pow<2>(mm2) * s56 + 10 * me2 * s12 * s56 + 2 * mm2 * s12 * s56 -
        12 * pow<2>(s12) * s56 + 2 * me2 * s15 * s56 + 3 * mm2 * s15 * s56 -
        16 * s12 * s15 * s56 - 5 * pow<2>(s15) * s56 - mm2 * s16 * s56 -
        7 * s12 * s16 * s56 - 6 * s15 * s16 * s56 - 9 * me2 * s25 * s56 +
        2 * mm2 * s25 * s56 + 22 * s12 * s25 * s56 + 16 * s15 * s25 * s56 +
        7 * s16 * s25 * s56 - 10 * pow<2>(s25) * s56 + 2 * me2 * s26 * s56 +
        3 * mm2 * s26 * s56 + 10 * s12 * s26 * s56 + 7 * s15 * s26 * s56 -
        10 * s25 * s26 * s56 + 2 * mm2 * pow<2>(s56) + 6 * s12 * pow<2>(s56) +
        4 * s15 * pow<2>(s56) - 6 * s25 * pow<2>(s56)};
    const auto if23{
        -6 * me2 * pow<2>(s12) - 2 * mm2 * pow<2>(s12) + 4 * pow<3>(s12) -
        6 * pow<2>(me2) * s15 - 2 * me2 * mm2 * s15 - me2 * s12 * s15 -
        mm2 * s12 * s15 + 6 * pow<2>(s12) * s15 + 2 * me2 * pow<2>(s15) +
        2 * s12 * pow<2>(s15) - 11 * me2 * s12 * s16 - 3 * mm2 * s12 * s16 +
        10 * pow<2>(s12) * s16 - 3 * me2 * s15 * s16 - mm2 * s15 * s16 +
        9 * s12 * s15 * s16 + pow<2>(s15) * s16 - 5 * me2 * pow<2>(s16) -
        mm2 * pow<2>(s16) + 7 * s12 * pow<2>(s16) + 2 * s15 * pow<2>(s16) +
        pow<3>(s16) + 12 * me2 * s12 * s25 + 2 * mm2 * s12 * s25 -
        12 * pow<2>(s12) * s25 - me2 * s15 * s25 - 3 * mm2 * s15 * s25 -
        8 * s12 * s15 * s25 + 2 * pow<2>(s15) * s25 + 10 * me2 * s16 * s25 -
        18 * s12 * s16 * s25 - 2 * s15 * s16 * s25 - 4 * pow<2>(s16) * s25 -
        6 * me2 * pow<2>(s25) + 12 * s12 * pow<2>(s25) + 2 * s15 * pow<2>(s25) +
        8 * s16 * pow<2>(s25) - 4 * pow<3>(s25) + 6 * me2 * mm2 * s26 +
        2 * pow<2>(mm2) * s26 + 6 * me2 * s12 * s26 - 4 * mm2 * s12 * s26 -
        8 * pow<2>(s12) * s26 - 5 * me2 * s15 * s26 - 7 * mm2 * s15 * s26 -
        2 * s12 * s15 * s26 + 4 * pow<2>(s15) * s26 + 3 * me2 * s16 * s26 -
        5 * mm2 * s16 * s26 - 10 * s12 * s16 * s26 + 3 * s15 * s16 * s26 -
        pow<2>(s16) * s26 - 6 * me2 * s25 * s26 + 6 * mm2 * s25 * s26 +
        16 * s12 * s25 * s26 - 2 * s15 * s25 * s26 + 8 * s16 * s25 * s26 -
        8 * pow<2>(s25) * s26 + 6 * mm2 * pow<2>(s26) + 4 * s12 * pow<2>(s26) -
        4 * s15 * pow<2>(s26) - 4 * s25 * pow<2>(s26) + 3 * pow<2>(me2) * s56 +
        4 * me2 * mm2 * s56 + pow<2>(mm2) * s56 + 10 * me2 * s12 * s56 +
        2 * mm2 * s12 * s56 - 12 * pow<2>(s12) * s56 - me2 * s15 * s56 -
        3 * mm2 * s15 * s56 - 8 * s12 * s15 * s56 + 2 * pow<2>(s15) * s56 +
        9 * me2 * s16 * s56 + mm2 * s16 * s56 - 17 * s12 * s16 * s56 -
        s15 * s16 * s56 - 3 * pow<2>(s16) * s56 - 9 * me2 * s25 * s56 +
        mm2 * s25 * s56 + 22 * s12 * s25 * s56 + s15 * s25 * s56 +
        11 * s16 * s25 * s56 - 10 * pow<2>(s25) * s56 - me2 * s26 * s56 +
        9 * mm2 * s26 * s56 + 12 * s12 * s26 * s56 - 7 * s15 * s26 * s56 +
        2 * s16 * s26 * s56 - 10 * s25 * s26 * s56 - 4 * me2 * pow<2>(s56) +
        10 * s12 * pow<2>(s56) - s15 * pow<2>(s56) + 3 * s16 * pow<2>(s56) -
        7 * s25 * pow<2>(s56) - s26 * pow<2>(s56) - pow<3>(s56)};
    const auto if33{
        24 * me2 * pow<2>(mm2) + 8 * pow<3>(mm2) - 16 * pow<2>(mm2) * s12 -
        24 * me2 * mm2 * s15 - 16 * pow<2>(mm2) * s15 + 16 * mm2 * s12 * s15 +
        6 * me2 * pow<2>(s15) + 10 * mm2 * pow<2>(s15) - 4 * s12 * pow<2>(s15) -
        2 * pow<3>(s15) - 12 * me2 * mm2 * s16 - 12 * pow<2>(mm2) * s16 +
        8 * mm2 * s12 * s16 + 6 * me2 * s15 * s16 + 14 * mm2 * s15 * s16 -
        4 * s12 * s15 * s16 - 4 * pow<2>(s15) * s16 + 4 * mm2 * pow<2>(s16) -
        2 * s15 * pow<2>(s16) + 16 * pow<2>(mm2) * s25 - 16 * mm2 * s15 * s25 +
        4 * pow<2>(s15) * s25 - 8 * mm2 * s16 * s25 + 4 * s15 * s16 * s25 +
        16 * pow<2>(mm2) * s26 - 16 * mm2 * s15 * s26 + 4 * pow<2>(s15) * s26 -
        8 * mm2 * s16 * s26 + 4 * s15 * s16 * s26 + 12 * me2 * mm2 * s56 +
        12 * pow<2>(mm2) * s56 - 8 * mm2 * s12 * s56 - 6 * me2 * s15 * s56 -
        14 * mm2 * s15 * s56 + 4 * s12 * s15 * s56 + 4 * pow<2>(s15) * s56 -
        8 * mm2 * s16 * s56 + 4 * s15 * s16 * s56 + 8 * mm2 * s25 * s56 -
        4 * s15 * s25 * s56 + 8 * mm2 * s26 * s56 - 4 * s15 * s26 * s56 +
        4 * mm2 * pow<2>(s56) - 2 * s15 * pow<2>(s56)};
    const auto if14{
        -12 * pow<2>(me2) * s12 - 4 * me2 * mm2 * s12 + 8 * me2 * pow<2>(s12) -
        10 * pow<2>(me2) * s15 - 2 * me2 * mm2 * s15 + 12 * me2 * s12 * s15 +
        2 * me2 * pow<2>(s15) + 8 * me2 * s12 * s16 + 6 * me2 * s15 * s16 +
        12 * pow<2>(me2) * s25 + 4 * me2 * mm2 * s25 - 18 * me2 * s12 * s25 -
        15 * me2 * s15 * s25 - mm2 * s15 * s25 + 3 * s12 * s15 * s25 +
        pow<2>(s15) * s25 - 11 * me2 * s16 * s25 - mm2 * s16 * s25 +
        s12 * s16 * s25 + 3 * s15 * s16 * s25 + 2 * pow<2>(s16) * s25 +
        10 * me2 * pow<2>(s25) - 2 * mm2 * pow<2>(s25) - s15 * pow<2>(s25) -
        s16 * pow<2>(s25) - 12 * me2 * s12 * s26 - 10 * me2 * s15 * s26 -
        2 * pow<2>(s15) * s26 - 2 * s12 * s16 * s26 - 2 * s15 * s16 * s26 +
        12 * me2 * s25 * s26 - 2 * mm2 * s25 * s26 + 2 * s12 * s25 * s26 +
        2 * s15 * s25 * s26 - 2 * pow<2>(s25) * s26 + 2 * s12 * pow<2>(s26) +
        2 * s15 * pow<2>(s26) - 2 * s25 * pow<2>(s26) + 6 * pow<2>(me2) * s56 +
        2 * me2 * mm2 * s56 - 14 * me2 * s12 * s56 - 12 * me2 * s15 * s56 -
        mm2 * s15 * s56 + 3 * s12 * s15 * s56 + pow<2>(s15) * s56 -
        4 * me2 * s16 * s56 - s12 * s16 * s56 + s15 * s16 * s56 +
        16 * me2 * s25 * s56 - 3 * mm2 * s25 * s56 - 2 * s15 * s25 * s56 -
        s16 * s25 * s56 + 5 * me2 * s26 * s56 - mm2 * s26 * s56 +
        4 * s12 * s26 * s56 + 3 * s15 * s26 * s56 + 2 * s16 * s26 * s56 -
        4 * s25 * s26 * s56 - 2 * pow<2>(s26) * s56 + 5 * me2 * pow<2>(s56) -
        s15 * pow<2>(s56) - 2 * s26 * pow<2>(s56)};
    const auto if24{
        -12 * pow<2>(me2) * s12 - 4 * me2 * mm2 * s12 + 8 * me2 * pow<2>(s12) -
        10 * pow<2>(me2) * s15 - 2 * me2 * mm2 * s15 + 12 * me2 * s12 * s15 +
        2 * me2 * pow<2>(s15) - 12 * pow<2>(me2) * s16 - 4 * me2 * mm2 * s16 +
        16 * me2 * s12 * s16 + 12 * me2 * s15 * s16 + 8 * me2 * pow<2>(s16) +
        12 * pow<2>(me2) * s25 - 16 * me2 * s12 * s25 - 8 * me2 * s15 * s25 -
        16 * me2 * s16 * s25 + 8 * me2 * pow<2>(s25) - 4 * me2 * mm2 * s26 -
        10 * me2 * s12 * s26 - 6 * me2 * s15 * s26 - 7 * me2 * s16 * s26 +
        mm2 * s16 * s26 - 3 * s12 * s16 * s26 - 4 * s15 * s16 * s26 -
        3 * pow<2>(s16) * s26 + 10 * me2 * s25 * s26 + 4 * s16 * s25 * s26 +
        2 * mm2 * pow<2>(s26) + 3 * s16 * pow<2>(s26) + 12 * pow<2>(me2) * s56 -
        16 * me2 * s12 * s56 - 8 * me2 * s15 * s56 - 16 * me2 * s16 * s56 +
        16 * me2 * s25 * s56 + 9 * me2 * s26 * s56 + 3 * mm2 * s26 * s56 +
        4 * s16 * s26 * s56 - s25 * s26 * s56 - pow<2>(s26) * s56 +
        8 * me2 * pow<2>(s56) - s26 * pow<2>(s56)};
    const auto if34{
        12 * me2 * mm2 * s12 + 4 * pow<2>(mm2) * s12 - 8 * mm2 * pow<2>(s12) +
        4 * me2 * mm2 * s15 - 6 * me2 * s12 * s15 - 8 * mm2 * s12 * s15 +
        4 * pow<2>(s12) * s15 - 4 * me2 * pow<2>(s15) + 4 * s12 * pow<2>(s15) +
        10 * me2 * mm2 * s16 + 2 * pow<2>(mm2) * s16 - 12 * mm2 * s12 * s16 -
        7 * me2 * s15 * s16 - 5 * mm2 * s15 * s16 + 8 * s12 * s15 * s16 +
        3 * pow<2>(s15) * s16 - 2 * mm2 * pow<2>(s16) + 3 * s15 * pow<2>(s16) -
        12 * me2 * mm2 * s25 - 4 * pow<2>(mm2) * s25 + 16 * mm2 * s12 * s25 +
        6 * me2 * s15 * s25 + 8 * mm2 * s15 * s25 - 8 * s12 * s15 * s25 -
        4 * pow<2>(s15) * s25 + 12 * mm2 * s16 * s25 - 8 * s15 * s16 * s25 -
        8 * mm2 * pow<2>(s25) + 4 * s15 * pow<2>(s25) - 12 * me2 * mm2 * s26 +
        16 * mm2 * s12 * s26 + 6 * me2 * s15 * s26 + 2 * mm2 * s15 * s26 -
        8 * s12 * s15 * s26 - 2 * pow<2>(s15) * s26 + 8 * mm2 * s16 * s26 -
        6 * s15 * s16 * s26 - 16 * mm2 * s25 * s26 + 8 * s15 * s25 * s26 -
        8 * mm2 * pow<2>(s26) + 4 * s15 * pow<2>(s26) - 10 * me2 * mm2 * s56 -
        2 * pow<2>(mm2) * s56 + 12 * mm2 * s12 * s56 + 7 * me2 * s15 * s56 +
        5 * mm2 * s15 * s56 - 8 * s12 * s15 * s56 - 3 * pow<2>(s15) * s56 +
        4 * mm2 * s16 * s56 - 6 * s15 * s16 * s56 - 12 * mm2 * s25 * s56 +
        8 * s15 * s25 * s56 - 8 * mm2 * s26 * s56 + 6 * s15 * s26 * s56 -
        2 * mm2 * pow<2>(s56) + 3 * s15 * pow<2>(s56)};
    const auto if44{
        24 * pow<2>(me2) * mm2 + 8 * me2 * pow<2>(mm2) - 16 * me2 * mm2 * s12 -
        12 * pow<2>(me2) * s15 - 12 * me2 * mm2 * s15 + 8 * me2 * s12 * s15 +
        4 * me2 * pow<2>(s15) - 16 * me2 * mm2 * s16 + 8 * me2 * s15 * s16 +
        16 * me2 * mm2 * s25 - 8 * me2 * s15 * s25 + 24 * me2 * mm2 * s26 -
        12 * me2 * s15 * s26 + 4 * mm2 * s16 * s26 - 2 * s15 * s16 * s26 -
        4 * mm2 * pow<2>(s26) + 2 * s15 * pow<2>(s26) + 16 * me2 * mm2 * s56 -
        8 * me2 * s15 * s56 - 4 * mm2 * s26 * s56 + 2 * s15 * s26 * s56};
    const auto if15{
        -6 * me2 * pow<2>(s12) - 2 * mm2 * pow<2>(s12) + 4 * pow<3>(s12) -
        11 * me2 * s12 * s15 - 3 * mm2 * s12 * s15 + 10 * pow<2>(s12) * s15 -
        5 * me2 * pow<2>(s15) - mm2 * pow<2>(s15) + 7 * s12 * pow<2>(s15) +
        pow<3>(s15) - 6 * pow<2>(me2) * s16 - 2 * me2 * mm2 * s16 -
        me2 * s12 * s16 - mm2 * s12 * s16 + 6 * pow<2>(s12) * s16 -
        3 * me2 * s15 * s16 - mm2 * s15 * s16 + 9 * s12 * s15 * s16 +
        2 * pow<2>(s15) * s16 + 2 * me2 * pow<2>(s16) + 2 * s12 * pow<2>(s16) +
        s15 * pow<2>(s16) + 6 * me2 * mm2 * s25 + 2 * pow<2>(mm2) * s25 +
        6 * me2 * s12 * s25 - 4 * mm2 * s12 * s25 - 8 * pow<2>(s12) * s25 +
        3 * me2 * s15 * s25 - 5 * mm2 * s15 * s25 - 10 * s12 * s15 * s25 -
        pow<2>(s15) * s25 - 5 * me2 * s16 * s25 - 7 * mm2 * s16 * s25 -
        2 * s12 * s16 * s25 + 3 * s15 * s16 * s25 + 4 * pow<2>(s16) * s25 +
        6 * mm2 * pow<2>(s25) + 4 * s12 * pow<2>(s25) - 4 * s16 * pow<2>(s25) +
        12 * me2 * s12 * s26 + 2 * mm2 * s12 * s26 - 12 * pow<2>(s12) * s26 +
        10 * me2 * s15 * s26 - 18 * s12 * s15 * s26 - 4 * pow<2>(s15) * s26 -
        me2 * s16 * s26 - 3 * mm2 * s16 * s26 - 8 * s12 * s16 * s26 -
        2 * s15 * s16 * s26 + 2 * pow<2>(s16) * s26 - 6 * me2 * s25 * s26 +
        6 * mm2 * s25 * s26 + 16 * s12 * s25 * s26 + 8 * s15 * s25 * s26 -
        2 * s16 * s25 * s26 - 4 * pow<2>(s25) * s26 - 6 * me2 * pow<2>(s26) +
        12 * s12 * pow<2>(s26) + 8 * s15 * pow<2>(s26) + 2 * s16 * pow<2>(s26) -
        8 * s25 * pow<2>(s26) - 4 * pow<3>(s26) + 3 * pow<2>(me2) * s56 +
        4 * me2 * mm2 * s56 + pow<2>(mm2) * s56 + 10 * me2 * s12 * s56 +
        2 * mm2 * s12 * s56 - 12 * pow<2>(s12) * s56 + 9 * me2 * s15 * s56 +
        mm2 * s15 * s56 - 17 * s12 * s15 * s56 - 3 * pow<2>(s15) * s56 -
        me2 * s16 * s56 - 3 * mm2 * s16 * s56 - 8 * s12 * s16 * s56 -
        s15 * s16 * s56 + 2 * pow<2>(s16) * s56 - me2 * s25 * s56 +
        9 * mm2 * s25 * s56 + 12 * s12 * s25 * s56 + 2 * s15 * s25 * s56 -
        7 * s16 * s25 * s56 - 9 * me2 * s26 * s56 + mm2 * s26 * s56 +
        22 * s12 * s26 * s56 + 11 * s15 * s26 * s56 + s16 * s26 * s56 -
        10 * s25 * s26 * s56 - 10 * pow<2>(s26) * s56 - 4 * me2 * pow<2>(s56) +
        10 * s12 * pow<2>(s56) + 3 * s15 * pow<2>(s56) - s16 * pow<2>(s56) -
        s25 * pow<2>(s56) - 7 * s26 * pow<2>(s56) - pow<3>(s56)};
    const auto if25{
        -6 * me2 * pow<2>(s12) - 2 * mm2 * pow<2>(s12) + 4 * pow<3>(s12) -
        5 * me2 * s12 * s15 - mm2 * s12 * s15 + 6 * pow<2>(s12) * s15 +
        s12 * pow<2>(s15) - 6 * pow<2>(me2) * s16 - 2 * me2 * mm2 * s16 -
        7 * me2 * s12 * s16 - 3 * mm2 * s12 * s16 + 10 * pow<2>(s12) * s16 -
        me2 * s15 * s16 - mm2 * s15 * s16 + 8 * s12 * s15 * s16 +
        pow<2>(s15) * s16 - me2 * pow<2>(s16) - mm2 * pow<2>(s16) +
        7 * s12 * pow<2>(s16) + 2 * s15 * pow<2>(s16) + pow<3>(s16) +
        6 * me2 * s12 * s25 - 8 * pow<2>(s12) * s25 - 4 * s12 * s15 * s25 -
        2 * me2 * s16 * s25 + 2 * mm2 * s16 * s25 - 11 * s12 * s16 * s25 -
        5 * s15 * s16 * s25 - 4 * pow<2>(s16) * s25 + 4 * s12 * pow<2>(s25) +
        3 * s16 * pow<2>(s25) + 6 * me2 * mm2 * s26 + 2 * pow<2>(mm2) * s26 +
        12 * me2 * s12 * s26 - 2 * mm2 * s12 * s26 - 12 * pow<2>(s12) * s26 +
        4 * me2 * s15 * s26 - 2 * mm2 * s15 * s26 - 10 * s12 * s15 * s26 -
        pow<2>(s15) * s26 + 5 * me2 * s16 * s26 + mm2 * s16 * s26 -
        19 * s12 * s16 * s26 - 9 * s15 * s16 * s26 - 7 * pow<2>(s16) * s26 -
        6 * me2 * s25 * s26 + 4 * mm2 * s25 * s26 + 16 * s12 * s25 * s26 +
        4 * s15 * s25 * s26 + 12 * s16 * s25 * s26 - 4 * pow<2>(s25) * s26 -
        6 * me2 * pow<2>(s26) + 4 * mm2 * pow<2>(s26) + 12 * s12 * pow<2>(s26) +
        4 * s15 * pow<2>(s26) + 9 * s16 * pow<2>(s26) - 8 * s25 * pow<2>(s26) -
        4 * pow<3>(s26) + 3 * pow<2>(me2) * s56 + 4 * me2 * mm2 * s56 +
        pow<2>(mm2) * s56 + 10 * me2 * s12 * s56 + 2 * mm2 * s12 * s56 -
        12 * pow<2>(s12) * s56 - mm2 * s15 * s56 - 7 * s12 * s15 * s56 +
        2 * me2 * s16 * s56 + 3 * mm2 * s16 * s56 - 16 * s12 * s16 * s56 -
        6 * s15 * s16 * s56 - 5 * pow<2>(s16) * s56 + 2 * me2 * s25 * s56 +
        3 * mm2 * s25 * s56 + 10 * s12 * s25 * s56 + 7 * s16 * s25 * s56 -
        9 * me2 * s26 * s56 + 2 * mm2 * s26 * s56 + 22 * s12 * s26 * s56 +
        7 * s15 * s26 * s56 + 16 * s16 * s26 * s56 - 10 * s25 * s26 * s56 -
        10 * pow<2>(s26) * s56 + 2 * mm2 * pow<2>(s56) + 6 * s12 * pow<2>(s56) +
        4 * s16 * pow<2>(s56) - 6 * s26 * pow<2>(s56)};
    const auto if35{
        24 * me2 * pow<2>(mm2) + 8 * pow<3>(mm2) - 16 * pow<2>(mm2) * s12 -
        18 * me2 * mm2 * s15 - 14 * pow<2>(mm2) * s15 + 12 * mm2 * s12 * s15 +
        3 * me2 * pow<2>(s15) + 7 * mm2 * pow<2>(s15) - 2 * s12 * pow<2>(s15) -
        pow<3>(s15) - 18 * me2 * mm2 * s16 - 14 * pow<2>(mm2) * s16 +
        12 * mm2 * s12 * s16 + 12 * me2 * s15 * s16 + 16 * mm2 * s15 * s16 -
        8 * s12 * s15 * s16 - 5 * pow<2>(s15) * s16 + 3 * me2 * pow<2>(s16) +
        7 * mm2 * pow<2>(s16) - 2 * s12 * pow<2>(s16) - 5 * s15 * pow<2>(s16) -
        pow<3>(s16) + 16 * pow<2>(mm2) * s25 - 12 * mm2 * s15 * s25 +
        2 * pow<2>(s15) * s25 - 12 * mm2 * s16 * s25 + 8 * s15 * s16 * s25 +
        2 * pow<2>(s16) * s25 + 16 * pow<2>(mm2) * s26 - 12 * mm2 * s15 * s26 +
        2 * pow<2>(s15) * s26 - 12 * mm2 * s16 * s26 + 8 * s15 * s16 * s26 +
        2 * pow<2>(s16) * s26 + 12 * me2 * mm2 * s56 + 12 * pow<2>(mm2) * s56 -
        8 * mm2 * s12 * s56 - 9 * me2 * s15 * s56 - 13 * mm2 * s15 * s56 +
        6 * s12 * s15 * s56 + 4 * pow<2>(s15) * s56 - 9 * me2 * s16 * s56 -
        13 * mm2 * s16 * s56 + 6 * s12 * s16 * s56 + 10 * s15 * s16 * s56 +
        4 * pow<2>(s16) * s56 + 8 * mm2 * s25 * s56 - 6 * s15 * s25 * s56 -
        6 * s16 * s25 * s56 + 8 * mm2 * s26 * s56 - 6 * s15 * s26 * s56 -
        6 * s16 * s26 * s56 + 6 * me2 * pow<2>(s56) + 6 * mm2 * pow<2>(s56) -
        4 * s12 * pow<2>(s56) - 5 * s15 * pow<2>(s56) - 5 * s16 * pow<2>(s56) +
        4 * s25 * pow<2>(s56) + 4 * s26 * pow<2>(s56) + 2 * pow<3>(s56)};
    const auto if45{
        12 * me2 * mm2 * s12 + 4 * pow<2>(mm2) * s12 - 8 * mm2 * pow<2>(s12) -
        6 * me2 * s12 * s15 - 6 * mm2 * s12 * s15 + 4 * pow<2>(s12) * s15 +
        2 * s12 * pow<2>(s15) + 6 * me2 * mm2 * s16 + 2 * pow<2>(mm2) * s16 -
        10 * mm2 * s12 * s16 - me2 * s15 * s16 - 3 * mm2 * s15 * s16 +
        4 * s12 * s15 * s16 + pow<2>(s15) * s16 - s12 * pow<2>(s16) -
        2 * s15 * pow<2>(s16) - 2 * pow<3>(s16) + 8 * mm2 * s12 * s25 -
        4 * s12 * s15 * s25 - 3 * me2 * s16 * s25 + 3 * mm2 * s16 * s25 +
        2 * s12 * s16 * s25 + 3 * pow<2>(s16) * s25 - 2 * s16 * pow<2>(s25) -
        12 * me2 * mm2 * s26 + 16 * mm2 * s12 * s26 + 6 * me2 * s15 * s26 +
        2 * mm2 * s15 * s26 - 8 * s12 * s15 * s26 - pow<2>(s15) * s26 +
        6 * me2 * s16 * s26 + 8 * mm2 * s16 * s26 - 4 * s12 * s16 * s26 -
        4 * s15 * s16 * s26 - pow<2>(s16) * s26 - 8 * mm2 * s25 * s26 +
        4 * s15 * s25 * s26 + 2 * s16 * s25 * s26 - 8 * mm2 * pow<2>(s26) +
        4 * s15 * pow<2>(s26) + 4 * s16 * pow<2>(s26) + 6 * mm2 * s12 * s56 -
        2 * me2 * s15 * s56 - 2 * s12 * s15 * s56 - 2 * me2 * s16 * s56 +
        mm2 * s16 * s56 + s12 * s16 * s56 + 2 * s15 * s16 * s56 +
        4 * pow<2>(s16) * s56 + 3 * me2 * s25 * s56 + mm2 * s25 * s56 -
        2 * s12 * s25 * s56 - 2 * s15 * s25 * s56 - 5 * s16 * s25 * s56 +
        2 * pow<2>(s25) * s56 - 3 * me2 * s26 * s56 - 4 * mm2 * s26 * s56 +
        2 * s12 * s26 * s56 + s15 * s26 * s56 + s16 * s26 * s56 -
        2 * pow<2>(s26) * s56 + 2 * me2 * pow<2>(s56) - mm2 * pow<2>(s56) -
        2 * s16 * pow<2>(s56) + 2 * s25 * pow<2>(s56)};
    const auto if55{
        24 * me2 * pow<2>(mm2) + 8 * pow<3>(mm2) - 16 * pow<2>(mm2) * s12 -
        12 * me2 * mm2 * s15 - 12 * pow<2>(mm2) * s15 + 8 * mm2 * s12 * s15 +
        4 * mm2 * pow<2>(s15) - 24 * me2 * mm2 * s16 - 16 * pow<2>(mm2) * s16 +
        16 * mm2 * s12 * s16 + 6 * me2 * s15 * s16 + 14 * mm2 * s15 * s16 -
        4 * s12 * s15 * s16 - 2 * pow<2>(s15) * s16 + 6 * me2 * pow<2>(s16) +
        10 * mm2 * pow<2>(s16) - 4 * s12 * pow<2>(s16) - 4 * s15 * pow<2>(s16) -
        2 * pow<3>(s16) + 16 * pow<2>(mm2) * s25 - 8 * mm2 * s15 * s25 -
        16 * mm2 * s16 * s25 + 4 * s15 * s16 * s25 + 4 * pow<2>(s16) * s25 +
        16 * pow<2>(mm2) * s26 - 8 * mm2 * s15 * s26 - 16 * mm2 * s16 * s26 +
        4 * s15 * s16 * s26 + 4 * pow<2>(s16) * s26 + 12 * me2 * mm2 * s56 +
        12 * pow<2>(mm2) * s56 - 8 * mm2 * s12 * s56 - 8 * mm2 * s15 * s56 -
        6 * me2 * s16 * s56 - 14 * mm2 * s16 * s56 + 4 * s12 * s16 * s56 +
        4 * s15 * s16 * s56 + 4 * pow<2>(s16) * s56 + 8 * mm2 * s25 * s56 -
        4 * s16 * s25 * s56 + 8 * mm2 * s26 * s56 - 4 * s16 * s26 * s56 +
        4 * mm2 * pow<2>(s56) - 2 * s16 * pow<2>(s56)};
    const auto if16{
        -12 * pow<2>(me2) * s12 - 4 * me2 * mm2 * s12 + 8 * me2 * pow<2>(s12) -
        12 * pow<2>(me2) * s15 - 4 * me2 * mm2 * s15 + 16 * me2 * s12 * s15 +
        8 * me2 * pow<2>(s15) - 10 * pow<2>(me2) * s16 - 2 * me2 * mm2 * s16 +
        12 * me2 * s12 * s16 + 12 * me2 * s15 * s16 + 2 * me2 * pow<2>(s16) -
        4 * me2 * mm2 * s25 - 10 * me2 * s12 * s25 - 7 * me2 * s15 * s25 +
        mm2 * s15 * s25 - 3 * s12 * s15 * s25 - 3 * pow<2>(s15) * s25 -
        6 * me2 * s16 * s25 - 4 * s15 * s16 * s25 + 2 * mm2 * pow<2>(s25) +
        3 * s15 * pow<2>(s25) + 12 * pow<2>(me2) * s26 - 16 * me2 * s12 * s26 -
        16 * me2 * s15 * s26 - 8 * me2 * s16 * s26 + 10 * me2 * s25 * s26 +
        4 * s15 * s25 * s26 + 8 * me2 * pow<2>(s26) + 12 * pow<2>(me2) * s56 -
        16 * me2 * s12 * s56 - 16 * me2 * s15 * s56 - 8 * me2 * s16 * s56 +
        9 * me2 * s25 * s56 + 3 * mm2 * s25 * s56 + 4 * s15 * s25 * s56 -
        pow<2>(s25) * s56 + 16 * me2 * s26 * s56 - s25 * s26 * s56 +
        8 * me2 * pow<2>(s56) - s25 * pow<2>(s56)};
    const auto if26{
        -12 * pow<2>(me2) * s12 - 4 * me2 * mm2 * s12 + 8 * me2 * pow<2>(s12) +
        8 * me2 * s12 * s15 - 10 * pow<2>(me2) * s16 - 2 * me2 * mm2 * s16 +
        12 * me2 * s12 * s16 + 6 * me2 * s15 * s16 + 2 * me2 * pow<2>(s16) -
        12 * me2 * s12 * s25 - 2 * s12 * s15 * s25 - 10 * me2 * s16 * s25 -
        2 * s15 * s16 * s25 - 2 * pow<2>(s16) * s25 + 2 * s12 * pow<2>(s25) +
        2 * s16 * pow<2>(s25) + 12 * pow<2>(me2) * s26 + 4 * me2 * mm2 * s26 -
        18 * me2 * s12 * s26 - 11 * me2 * s15 * s26 - mm2 * s15 * s26 +
        s12 * s15 * s26 + 2 * pow<2>(s15) * s26 - 15 * me2 * s16 * s26 -
        mm2 * s16 * s26 + 3 * s12 * s16 * s26 + 3 * s15 * s16 * s26 +
        pow<2>(s16) * s26 + 12 * me2 * s25 * s26 - 2 * mm2 * s25 * s26 +
        2 * s12 * s25 * s26 + 2 * s16 * s25 * s26 - 2 * pow<2>(s25) * s26 +
        10 * me2 * pow<2>(s26) - 2 * mm2 * pow<2>(s26) - s15 * pow<2>(s26) -
        s16 * pow<2>(s26) - 2 * s25 * pow<2>(s26) + 6 * pow<2>(me2) * s56 +
        2 * me2 * mm2 * s56 - 14 * me2 * s12 * s56 - 4 * me2 * s15 * s56 -
        s12 * s15 * s56 - 12 * me2 * s16 * s56 - mm2 * s16 * s56 +
        3 * s12 * s16 * s56 + s15 * s16 * s56 + pow<2>(s16) * s56 +
        5 * me2 * s25 * s56 - mm2 * s25 * s56 + 4 * s12 * s25 * s56 +
        2 * s15 * s25 * s56 + 3 * s16 * s25 * s56 - 2 * pow<2>(s25) * s56 +
        16 * me2 * s26 * s56 - 3 * mm2 * s26 * s56 - s15 * s26 * s56 -
        2 * s16 * s26 * s56 - 4 * s25 * s26 * s56 + 5 * me2 * pow<2>(s56) -
        s16 * pow<2>(s56) - 2 * s25 * pow<2>(s56)};
    const auto if36{
        12 * me2 * mm2 * s12 + 4 * pow<2>(mm2) * s12 - 8 * mm2 * pow<2>(s12) +
        6 * me2 * mm2 * s15 + 2 * pow<2>(mm2) * s15 - 10 * mm2 * s12 * s15 -
        s12 * pow<2>(s15) - 2 * pow<3>(s15) - 6 * me2 * s12 * s16 -
        6 * mm2 * s12 * s16 + 4 * pow<2>(s12) * s16 - me2 * s15 * s16 -
        3 * mm2 * s15 * s16 + 4 * s12 * s15 * s16 - 2 * pow<2>(s15) * s16 +
        2 * s12 * pow<2>(s16) + s15 * pow<2>(s16) - 12 * me2 * mm2 * s25 +
        16 * mm2 * s12 * s25 + 6 * me2 * s15 * s25 + 8 * mm2 * s15 * s25 -
        4 * s12 * s15 * s25 - pow<2>(s15) * s25 + 6 * me2 * s16 * s25 +
        2 * mm2 * s16 * s25 - 8 * s12 * s16 * s25 - 4 * s15 * s16 * s25 -
        pow<2>(s16) * s25 - 8 * mm2 * pow<2>(s25) + 4 * s15 * pow<2>(s25) +
        4 * s16 * pow<2>(s25) + 8 * mm2 * s12 * s26 - 3 * me2 * s15 * s26 +
        3 * mm2 * s15 * s26 + 2 * s12 * s15 * s26 + 3 * pow<2>(s15) * s26 -
        4 * s12 * s16 * s26 - 8 * mm2 * s25 * s26 + 2 * s15 * s25 * s26 +
        4 * s16 * s25 * s26 - 2 * s15 * pow<2>(s26) + 6 * mm2 * s12 * s56 -
        2 * me2 * s15 * s56 + mm2 * s15 * s56 + s12 * s15 * s56 +
        4 * pow<2>(s15) * s56 - 2 * me2 * s16 * s56 - 2 * s12 * s16 * s56 +
        2 * s15 * s16 * s56 - 3 * me2 * s25 * s56 - 4 * mm2 * s25 * s56 +
        2 * s12 * s25 * s56 + s15 * s25 * s56 + s16 * s25 * s56 -
        2 * pow<2>(s25) * s56 + 3 * me2 * s26 * s56 + mm2 * s26 * s56 -
        2 * s12 * s26 * s56 - 5 * s15 * s26 * s56 - 2 * s16 * s26 * s56 +
        2 * pow<2>(s26) * s56 + 2 * me2 * pow<2>(s56) - mm2 * pow<2>(s56) -
        2 * s15 * pow<2>(s56) + 2 * s26 * pow<2>(s56)};
    const auto if46{
        6 * me2 * pow<2>(s12) + 2 * mm2 * pow<2>(s12) - 4 * pow<3>(s12) +
        5 * me2 * s12 * s15 + mm2 * s12 * s15 - 6 * pow<2>(s12) * s15 -
        s12 * pow<2>(s15) + 5 * me2 * s12 * s16 + mm2 * s12 * s16 -
        6 * pow<2>(s12) * s16 + 2 * me2 * s15 * s16 - 6 * s12 * s15 * s16 -
        s12 * pow<2>(s16) - 6 * me2 * s12 * s25 + 8 * pow<2>(s12) * s25 +
        4 * s12 * s15 * s25 - 5 * me2 * s16 * s25 + mm2 * s16 * s25 +
        9 * s12 * s16 * s25 + s15 * s16 * s25 + pow<2>(s16) * s25 -
        4 * s12 * pow<2>(s25) - 3 * s16 * pow<2>(s25) - 6 * me2 * s12 * s26 +
        8 * pow<2>(s12) * s26 - 5 * me2 * s15 * s26 + mm2 * s15 * s26 +
        9 * s12 * s15 * s26 + pow<2>(s15) * s26 + 4 * s12 * s16 * s26 +
        s15 * s16 * s26 + 6 * me2 * s25 * s26 - 4 * mm2 * s25 * s26 -
        12 * s12 * s25 * s26 - 3 * s15 * s25 * s26 - 3 * s16 * s25 * s26 +
        4 * pow<2>(s25) * s26 - 4 * s12 * pow<2>(s26) - 3 * s15 * pow<2>(s26) +
        4 * s25 * pow<2>(s26) - 3 * pow<2>(me2) * s56 + 2 * me2 * mm2 * s56 +
        pow<2>(mm2) * s56 + 4 * pow<2>(s12) * s56 - mm2 * s15 * s56 +
        2 * s12 * s15 * s56 - mm2 * s16 * s56 + 2 * s12 * s16 * s56 -
        2 * me2 * s25 * s56 + mm2 * s25 * s56 - 4 * s12 * s25 * s56 -
        2 * s16 * s25 * s56 - 2 * me2 * s26 * s56 + mm2 * s26 * s56 -
        4 * s12 * s26 * s56 - 2 * s15 * s26 * s56 + 2 * s25 * s26 * s56 -
        me2 * pow<2>(s56) + mm2 * pow<2>(s56)};
    const auto if56{
        12 * me2 * mm2 * s12 + 4 * pow<2>(mm2) * s12 - 8 * mm2 * pow<2>(s12) +
        10 * me2 * mm2 * s15 + 2 * pow<2>(mm2) * s15 - 12 * mm2 * s12 * s15 -
        2 * mm2 * pow<2>(s15) + 4 * me2 * mm2 * s16 - 6 * me2 * s12 * s16 -
        8 * mm2 * s12 * s16 + 4 * pow<2>(s12) * s16 - 7 * me2 * s15 * s16 -
        5 * mm2 * s15 * s16 + 8 * s12 * s15 * s16 + 3 * pow<2>(s15) * s16 -
        4 * me2 * pow<2>(s16) + 4 * s12 * pow<2>(s16) + 3 * s15 * pow<2>(s16) -
        12 * me2 * mm2 * s25 + 16 * mm2 * s12 * s25 + 8 * mm2 * s15 * s25 +
        6 * me2 * s16 * s25 + 2 * mm2 * s16 * s25 - 8 * s12 * s16 * s25 -
        6 * s15 * s16 * s25 - 2 * pow<2>(s16) * s25 - 8 * mm2 * pow<2>(s25) +
        4 * s16 * pow<2>(s25) - 12 * me2 * mm2 * s26 - 4 * pow<2>(mm2) * s26 +
        16 * mm2 * s12 * s26 + 12 * mm2 * s15 * s26 + 6 * me2 * s16 * s26 +
        8 * mm2 * s16 * s26 - 8 * s12 * s16 * s26 - 8 * s15 * s16 * s26 -
        4 * pow<2>(s16) * s26 - 16 * mm2 * s25 * s26 + 8 * s16 * s25 * s26 -
        8 * mm2 * pow<2>(s26) + 4 * s16 * pow<2>(s26) - 10 * me2 * mm2 * s56 -
        2 * pow<2>(mm2) * s56 + 12 * mm2 * s12 * s56 + 4 * mm2 * s15 * s56 +
        7 * me2 * s16 * s56 + 5 * mm2 * s16 * s56 - 8 * s12 * s16 * s56 -
        6 * s15 * s16 * s56 - 3 * pow<2>(s16) * s56 - 8 * mm2 * s25 * s56 +
        6 * s16 * s25 * s56 - 12 * mm2 * s26 * s56 + 8 * s16 * s26 * s56 -
        2 * mm2 * pow<2>(s56) + 3 * s16 * pow<2>(s56)};
    const auto if66{
        24 * pow<2>(me2) * mm2 + 8 * me2 * pow<2>(mm2) - 16 * me2 * mm2 * s12 -
        16 * me2 * mm2 * s15 - 12 * pow<2>(me2) * s16 - 12 * me2 * mm2 * s16 +
        8 * me2 * s12 * s16 + 8 * me2 * s15 * s16 + 4 * me2 * pow<2>(s16) +
        24 * me2 * mm2 * s25 + 4 * mm2 * s15 * s25 - 12 * me2 * s16 * s25 -
        2 * s15 * s16 * s25 - 4 * mm2 * pow<2>(s25) + 2 * s16 * pow<2>(s25) +
        16 * me2 * mm2 * s26 - 8 * me2 * s16 * s26 + 16 * me2 * mm2 * s56 -
        8 * me2 * s16 * s56 - 4 * mm2 * s25 * s56 + 2 * s16 * s25 * s56};

    return if11 / pow<2>(den1) + if22 / pow<2>(den2) + if33 / pow<2>(den3) +
           if44 / pow<2>(den4) + if55 / pow<2>(den5) + if66 / pow<2>(den6) +
           2 * (if12 / (den1 * den2) + if13 / (den1 * den3) +
                if14 / (den1 * den4) + if15 / (den1 * den5) +
                if16 / (den1 * den6) + if23 / (den2 * den3) +
                if24 / (den2 * den4) + if25 / (den2 * den5) +
                if26 / (den2 * den6) + if34 / (den3 * den4) +
                if35 / (den3 * den5) + if36 / (den3 * den6) +
                if45 / (den4 * den5) + if46 / (den4 * den6) +
                if56 / (den5 * den6));
}

auto DoubleRadiativeMuonDecay::MSqPolarizedS5n(double mm2, double me2, double s12, double s15, double s16, double s25, double s26, double s56,
                                               double den1, double den2, double den3, double den4, double den5, double den6) -> double {
    using muc::pow;

    // Adapt from McMule v0.5.0, mudec/mudec_pm2ennggav.f95
    //
    // Copyright 2020-2024  Yannick Ulrich and others (The McMule development team)
    //

    const auto if11{
        24 * pow<3>(me2) + 8 * pow<2>(me2) * mm2 - 16 * pow<2>(me2) * s12 -
        16 * pow<2>(me2) * s15 - 16 * pow<2>(me2) * s16 +
        36 * pow<2>(me2) * s25 + 4 * me2 * mm2 * s25 - 8 * me2 * s12 * s25 -
        8 * me2 * s15 * s25 - 4 * me2 * s16 * s25 + 16 * me2 * pow<2>(s25) -
        2 * s12 * pow<2>(s25) - 2 * s16 * pow<2>(s25) + 2 * pow<3>(s25) +
        24 * pow<2>(me2) * s26 + 4 * me2 * s16 * s26 + 6 * me2 * s25 * s26 -
        2 * mm2 * s25 * s26 + 2 * s12 * s25 * s26 + 4 * s15 * s25 * s26 +
        2 * s16 * s25 * s26 - 4 * me2 * pow<2>(s26) - 2 * s25 * pow<2>(s26) +
        24 * pow<2>(me2) * s56 + 4 * me2 * s16 * s56 + 12 * me2 * s25 * s56 -
        2 * s12 * s25 * s56 - 2 * s16 * s25 * s56 + 4 * pow<2>(s25) * s56 -
        8 * me2 * s26 * s56 - 4 * me2 * pow<2>(s56) + 2 * s25 * pow<2>(s56)};
    const auto if12{
        24 * pow<3>(me2) + 8 * pow<2>(me2) * mm2 - 16 * pow<2>(me2) * s12 -
        16 * pow<2>(me2) * s15 - 16 * pow<2>(me2) * s16 +
        30 * pow<2>(me2) * s25 + 2 * me2 * mm2 * s25 - 4 * me2 * s12 * s25 -
        4 * me2 * s15 * s25 - 4 * me2 * s16 * s25 + 4 * me2 * pow<2>(s25) -
        2 * mm2 * pow<2>(s25) + 2 * s12 * pow<2>(s25) + 4 * s15 * pow<2>(s25) +
        4 * s16 * pow<2>(s25) - 4 * pow<3>(s25) + 30 * pow<2>(me2) * s26 +
        2 * me2 * mm2 * s26 - 4 * me2 * s12 * s26 - 4 * me2 * s15 * s26 -
        4 * me2 * s16 * s26 + 8 * me2 * s25 * s26 - 4 * mm2 * s25 * s26 +
        4 * s12 * s25 * s26 + 8 * s15 * s25 * s26 + 8 * s16 * s25 * s26 -
        12 * pow<2>(s25) * s26 + 4 * me2 * pow<2>(s26) - 2 * mm2 * pow<2>(s26) +
        2 * s12 * pow<2>(s26) + 4 * s15 * pow<2>(s26) + 4 * s16 * pow<2>(s26) -
        12 * s25 * pow<2>(s26) - 4 * pow<3>(s26) + 24 * pow<2>(me2) * s56 +
        8 * me2 * s25 * s56 - 2 * mm2 * s25 * s56 + 4 * s15 * s25 * s56 +
        4 * s16 * s25 * s56 - 8 * pow<2>(s25) * s56 + 8 * me2 * s26 * s56 -
        2 * mm2 * s26 * s56 + 4 * s15 * s26 * s56 + 4 * s16 * s26 * s56 -
        16 * s25 * s26 * s56 - 8 * pow<2>(s26) * s56 + 4 * me2 * pow<2>(s56) -
        2 * s12 * pow<2>(s56) - 4 * s25 * pow<2>(s56) - 4 * s26 * pow<2>(s56)};
    const auto if22{
        24 * pow<3>(me2) + 8 * pow<2>(me2) * mm2 - 16 * pow<2>(me2) * s12 -
        16 * pow<2>(me2) * s15 - 16 * pow<2>(me2) * s16 +
        20 * pow<2>(me2) * s25 - 4 * me2 * mm2 * s25 + 4 * me2 * s12 * s25 +
        8 * me2 * s15 * s25 + 4 * me2 * s16 * s25 - 8 * me2 * pow<2>(s25) +
        32 * pow<2>(me2) * s26 - 4 * me2 * s12 * s26 - 4 * me2 * s16 * s26 +
        2 * s16 * s25 * s26 + 14 * me2 * pow<2>(s26) + 2 * mm2 * pow<2>(s26) -
        4 * s12 * pow<2>(s26) - 4 * s15 * pow<2>(s26) - 2 * s16 * pow<2>(s26) +
        4 * s25 * pow<2>(s26) + 4 * pow<3>(s26) + 20 * pow<2>(me2) * s56 -
        4 * me2 * mm2 * s56 + 4 * me2 * s12 * s56 + 8 * me2 * s15 * s56 +
        4 * me2 * s16 * s56 - 16 * me2 * s25 * s56 + 6 * me2 * s26 * s56 +
        2 * mm2 * s26 * s56 - 4 * s12 * s26 * s56 - 4 * s15 * s26 * s56 -
        2 * s16 * s26 * s56 + 4 * s25 * s26 * s56 + 8 * pow<2>(s26) * s56 -
        8 * me2 * pow<2>(s56) + 4 * s26 * pow<2>(s56)};
    const auto if13{
        -6 * pow<2>(me2) * s12 - 2 * me2 * mm2 * s12 + me2 * pow<2>(s12) -
        mm2 * pow<2>(s12) + 2 * pow<3>(s12) - 6 * pow<2>(me2) * s15 +
        2 * me2 * mm2 * s15 - me2 * s12 * s15 - mm2 * s12 * s15 +
        5 * pow<2>(s12) * s15 - 2 * me2 * pow<2>(s15) + 3 * s12 * pow<2>(s15) -
        4 * pow<2>(me2) * s16 + 2 * me2 * s12 * s16 + 3 * pow<2>(s12) * s16 -
        3 * me2 * s15 * s16 + 3 * s12 * s15 * s16 - me2 * pow<2>(s16) +
        6 * pow<2>(me2) * s25 + 2 * me2 * mm2 * s25 - 5 * me2 * s12 * s25 -
        3 * mm2 * s12 * s25 - 3 * pow<2>(s12) * s25 - 4 * me2 * s15 * s25 -
        2 * mm2 * s15 * s25 - 3 * s12 * s15 * s25 - 3 * me2 * s16 * s25 -
        2 * mm2 * s16 * s25 + 4 * me2 * pow<2>(s25) + 2 * mm2 * pow<2>(s25) +
        s12 * pow<2>(s25) - s16 * pow<2>(s25) + 3 * pow<2>(me2) * s26 -
        2 * me2 * mm2 * s26 - pow<2>(mm2) * s26 - 4 * me2 * s12 * s26 -
        3 * pow<2>(s12) * s26 - 4 * me2 * s15 * s26 + mm2 * s15 * s26 -
        2 * s12 * s15 * s26 - 3 * me2 * s16 * s26 + mm2 * s16 * s26 +
        s12 * s16 * s26 + 5 * me2 * s25 * s26 + 2 * s12 * s25 * s26 -
        s16 * s25 * s26 + 2 * me2 * pow<2>(s26) - 3 * mm2 * pow<2>(s26) +
        s12 * pow<2>(s26) + 8 * pow<2>(me2) * s56 + 4 * me2 * mm2 * s56 -
        2 * me2 * s12 * s56 - 6 * pow<2>(s12) * s56 - 2 * me2 * s15 * s56 -
        5 * s12 * s15 * s56 - 2 * me2 * s16 * s56 - s12 * s16 * s56 +
        6 * me2 * s25 * s56 + 4 * mm2 * s25 * s56 + 4 * s12 * s25 * s56 -
        s16 * s25 * s56 + 6 * me2 * s26 * s56 - 2 * mm2 * s26 * s56 +
        3 * s12 * s26 * s56 + 2 * me2 * pow<2>(s56) + 2 * s12 * pow<2>(s56)};
    const auto if23{
        -6 * pow<2>(me2) * s12 - 2 * me2 * mm2 * s12 + me2 * pow<2>(s12) -
        mm2 * pow<2>(s12) + 2 * pow<3>(s12) - 6 * pow<2>(me2) * s15 +
        2 * me2 * mm2 * s15 + 4 * me2 * s12 * s15 + 2 * pow<2>(s12) * s15 -
        4 * pow<2>(me2) * s16 + me2 * s12 * s16 - mm2 * s12 * s16 +
        4 * pow<2>(s12) * s16 - 2 * me2 * s15 * s16 + 2 * s12 * s15 * s16 -
        2 * me2 * pow<2>(s16) + 2 * s12 * pow<2>(s16) + 6 * pow<2>(me2) * s25 +
        2 * me2 * mm2 * s25 - 5 * me2 * s12 * s25 - mm2 * s12 * s25 -
        4 * pow<2>(s12) * s25 - 4 * me2 * s15 * s25 - 2 * s12 * s15 * s25 -
        3 * me2 * s16 * s25 - mm2 * s16 * s25 - 4 * s12 * s16 * s25 +
        4 * me2 * pow<2>(s25) + 2 * mm2 * pow<2>(s25) + 2 * s12 * pow<2>(s25) +
        3 * pow<2>(me2) * s26 + 4 * me2 * mm2 * s26 + pow<2>(mm2) * s26 -
        7 * me2 * s12 * s26 - 3 * mm2 * s12 * s26 - 2 * pow<2>(s12) * s26 -
        3 * me2 * s15 * s26 - mm2 * s15 * s26 - 5 * me2 * s16 * s26 -
        3 * mm2 * s16 * s26 + s15 * s16 * s26 + pow<2>(s16) * s26 +
        9 * me2 * s25 * s26 + 5 * mm2 * s25 * s26 - s15 * s25 * s26 -
        3 * s16 * s25 * s26 + 2 * pow<2>(s25) * s26 + 3 * me2 * pow<2>(s26) +
        mm2 * pow<2>(s26) + s15 * pow<2>(s26) - s16 * pow<2>(s26) +
        2 * s25 * pow<2>(s26) + 8 * pow<2>(me2) * s56 + 4 * me2 * mm2 * s56 -
        6 * me2 * s12 * s56 - 4 * pow<2>(s12) * s56 - 2 * me2 * s15 * s56 -
        2 * s12 * s15 * s56 - 4 * s12 * s16 * s56 + 8 * me2 * s25 * s56 +
        2 * mm2 * s25 * s56 + 4 * s12 * s25 * s56 + 8 * me2 * s26 * s56 +
        4 * mm2 * s26 * s56 - s15 * s26 * s56 - 2 * s16 * s26 * s56 +
        3 * s25 * s26 * s56 + pow<2>(s26) * s56 + 2 * me2 * pow<2>(s56) +
        2 * s12 * pow<2>(s56) + s26 * pow<2>(s56)};
    const auto if33{
        12 * me2 * mm2 * s12 + 4 * pow<2>(mm2) * s12 - 8 * mm2 * pow<2>(s12) -
        4 * mm2 * s12 * s15 + 4 * me2 * pow<2>(s15) - 2 * s12 * pow<2>(s15) -
        4 * mm2 * s12 * s16 + 8 * me2 * s15 * s16 - 4 * s12 * s15 * s16 +
        4 * me2 * pow<2>(s16) - 2 * s12 * pow<2>(s16) - 12 * me2 * mm2 * s25 -
        4 * pow<2>(mm2) * s25 + 16 * mm2 * s12 * s25 + 4 * mm2 * s15 * s25 +
        2 * pow<2>(s15) * s25 + 4 * mm2 * s16 * s25 + 4 * s15 * s16 * s25 +
        2 * pow<2>(s16) * s25 - 8 * mm2 * pow<2>(s25) + 8 * mm2 * s12 * s26 -
        6 * me2 * s15 * s26 - 2 * mm2 * s15 * s26 + 4 * s12 * s15 * s26 +
        4 * pow<2>(s15) * s26 - 6 * me2 * s16 * s26 - 2 * mm2 * s16 * s26 +
        4 * s12 * s16 * s26 + 8 * s15 * s16 * s26 + 4 * pow<2>(s16) * s26 -
        8 * mm2 * s25 * s26 - 4 * s15 * s25 * s26 - 4 * s16 * s25 * s26 -
        4 * s15 * pow<2>(s26) - 4 * s16 * pow<2>(s26) + 4 * mm2 * s12 * s56 -
        8 * me2 * s15 * s56 + 4 * s12 * s15 * s56 - 8 * me2 * s16 * s56 +
        4 * s12 * s16 * s56 - 4 * mm2 * s25 * s56 - 4 * s15 * s25 * s56 -
        4 * s16 * s25 * s56 + 6 * me2 * s26 * s56 + 2 * mm2 * s26 * s56 -
        4 * s12 * s26 * s56 - 8 * s15 * s26 * s56 - 8 * s16 * s26 * s56 +
        4 * s25 * s26 * s56 + 4 * pow<2>(s26) * s56 + 4 * me2 * pow<2>(s56) -
        2 * s12 * pow<2>(s56) + 2 * s25 * pow<2>(s56) + 4 * s26 * pow<2>(s56)};
    const auto if14{
        -12 * pow<3>(me2) - 4 * pow<2>(me2) * mm2 + 2 * pow<2>(me2) * s12 -
        2 * me2 * mm2 * s12 + 4 * me2 * pow<2>(s12) + 6 * me2 * s12 * s15 +
        8 * pow<2>(me2) * s16 + 4 * me2 * s12 * s16 + me2 * s15 * s16 -
        12 * pow<2>(me2) * s25 - 4 * me2 * mm2 * s25 - 3 * me2 * s12 * s25 -
        mm2 * s12 * s25 + pow<2>(s12) * s25 - 2 * me2 * s15 * s25 +
        3 * s12 * s15 * s25 - mm2 * s16 * s25 + 3 * s12 * s16 * s25 +
        2 * s15 * s16 * s25 + 2 * pow<2>(s16) * s25 - 2 * me2 * pow<2>(s25) -
        2 * mm2 * pow<2>(s25) - s12 * pow<2>(s25) - 2 * s16 * pow<2>(s25) -
        18 * pow<2>(me2) * s26 - 2 * me2 * mm2 * s26 - 2 * me2 * s12 * s26 -
        2 * me2 * s15 * s26 + mm2 * s15 * s26 - 2 * pow<2>(s15) * s26 +
        2 * me2 * s16 * s26 - s12 * s16 * s26 - 2 * s15 * s16 * s26 -
        6 * me2 * s25 * s26 - 3 * mm2 * s25 * s26 + 2 * s15 * s25 * s26 -
        2 * s16 * s25 * s26 - 3 * me2 * pow<2>(s26) + mm2 * pow<2>(s26) -
        2 * s16 * pow<2>(s26) + 2 * s25 * pow<2>(s26) + 2 * pow<3>(s26) -
        10 * pow<2>(me2) * s56 + 2 * me2 * mm2 * s56 - 4 * me2 * s12 * s56 -
        pow<2>(s12) * s56 - 4 * me2 * s15 * s56 + s12 * s15 * s56 -
        me2 * s16 * s56 - s12 * s16 * s56 - 2 * me2 * s25 * s56 -
        2 * mm2 * s25 * s56 - 2 * s16 * s25 * s56 - 3 * me2 * s26 * s56 +
        3 * s12 * s26 * s56 + 2 * s15 * s26 * s56 + 2 * pow<2>(s26) * s56 +
        s12 * pow<2>(s56)};
    const auto if24{
        -12 * pow<3>(me2) - 4 * pow<2>(me2) * mm2 + 2 * pow<2>(me2) * s12 -
        2 * me2 * mm2 * s12 + 4 * me2 * pow<2>(s12) + 6 * me2 * s12 * s15 +
        2 * pow<2>(me2) * s16 - 2 * me2 * mm2 * s16 + 8 * me2 * s12 * s16 +
        6 * me2 * s15 * s16 + 4 * me2 * pow<2>(s16) - 8 * pow<2>(me2) * s25 -
        4 * me2 * mm2 * s25 - 4 * me2 * s12 * s25 - 4 * me2 * s16 * s25 -
        20 * pow<2>(me2) * s26 - 4 * me2 * mm2 * s26 + me2 * s16 * s26 +
        mm2 * s16 * s26 - 2 * s12 * s16 * s26 - 3 * s15 * s16 * s26 -
        2 * pow<2>(s16) * s26 - 6 * me2 * s25 * s26 + 2 * s16 * s25 * s26 -
        7 * me2 * pow<2>(s26) + mm2 * pow<2>(s26) + s16 * pow<2>(s26) +
        s25 * pow<2>(s26) + pow<3>(s26) - 8 * pow<2>(me2) * s56 -
        4 * me2 * mm2 * s56 - 4 * me2 * s12 * s56 - 4 * me2 * s16 * s56 -
        6 * me2 * s26 * s56 + 2 * mm2 * s26 * s56 + 2 * s16 * s26 * s56 +
        pow<2>(s26) * s56};
    const auto if34{
        6 * me2 * pow<2>(s12) + 2 * mm2 * pow<2>(s12) - 4 * pow<3>(s12) +
        6 * pow<2>(me2) * s15 + 2 * me2 * mm2 * s15 - 4 * pow<2>(s12) * s15 -
        2 * me2 * pow<2>(s15) + 6 * pow<2>(me2) * s16 + 2 * me2 * mm2 * s16 +
        4 * me2 * s12 * s16 + 2 * mm2 * s12 * s16 - 8 * pow<2>(s12) * s16 -
        2 * me2 * s15 * s16 - 4 * s12 * s15 * s16 - 4 * s12 * pow<2>(s16) -
        12 * me2 * s12 * s25 - 4 * mm2 * s12 * s25 + 12 * pow<2>(s12) * s25 +
        8 * s12 * s15 * s25 - 4 * me2 * s16 * s25 - 2 * mm2 * s16 * s25 +
        16 * s12 * s16 * s25 + 4 * s15 * s16 * s25 + 4 * pow<2>(s16) * s25 +
        6 * me2 * pow<2>(s25) + 2 * mm2 * pow<2>(s25) - 12 * s12 * pow<2>(s25) -
        4 * s15 * pow<2>(s25) - 8 * s16 * pow<2>(s25) + 4 * pow<3>(s25) -
        6 * me2 * s12 * s26 + 8 * pow<2>(s12) * s26 + 5 * me2 * s15 * s26 +
        mm2 * s15 * s26 + 2 * s12 * s15 * s26 - pow<2>(s15) * s26 +
        4 * me2 * s16 * s26 + 2 * mm2 * s16 * s26 + 6 * s12 * s16 * s26 -
        s15 * s16 * s26 + 6 * me2 * s25 * s26 - 16 * s12 * s25 * s26 -
        2 * s15 * s25 * s26 - 6 * s16 * s25 * s26 + 8 * pow<2>(s25) * s26 -
        4 * s12 * pow<2>(s26) + 4 * s25 * pow<2>(s26) - 6 * pow<2>(me2) * s56 -
        2 * me2 * mm2 * s56 - 4 * me2 * s12 * s56 - 2 * mm2 * s12 * s56 +
        8 * pow<2>(s12) * s56 + 2 * me2 * s15 * s56 + 4 * s12 * s15 * s56 +
        8 * s12 * s16 * s56 + 4 * me2 * s25 * s56 + 2 * mm2 * s25 * s56 -
        16 * s12 * s25 * s56 - 4 * s15 * s25 * s56 - 8 * s16 * s25 * s56 +
        8 * pow<2>(s25) * s56 - 4 * me2 * s26 * s56 - 2 * mm2 * s26 * s56 -
        6 * s12 * s26 * s56 + s15 * s26 * s56 + 6 * s25 * s26 * s56 -
        4 * s12 * pow<2>(s56) + 4 * s25 * pow<2>(s56)};
    const auto if44{
        12 * pow<2>(me2) * s12 + 4 * me2 * mm2 * s12 - 8 * me2 * pow<2>(s12) +
        8 * pow<2>(me2) * s15 - 8 * me2 * s12 * s15 + 12 * pow<2>(me2) * s16 +
        4 * me2 * mm2 * s16 - 16 * me2 * s12 * s16 - 8 * me2 * s15 * s16 -
        8 * me2 * pow<2>(s16) - 12 * pow<2>(me2) * s25 - 4 * me2 * mm2 * s25 +
        16 * me2 * s12 * s25 + 8 * me2 * s15 * s25 + 16 * me2 * s16 * s25 -
        8 * me2 * pow<2>(s25) + 12 * me2 * s12 * s26 + 8 * me2 * s15 * s26 +
        10 * me2 * s16 * s26 - 2 * mm2 * s16 * s26 + 4 * s12 * s16 * s26 +
        4 * s15 * s16 * s26 + 4 * pow<2>(s16) * s26 - 12 * me2 * s25 * s26 -
        4 * s16 * s25 * s26 - 2 * s12 * pow<2>(s26) - 2 * s15 * pow<2>(s26) -
        4 * s16 * pow<2>(s26) + 2 * s25 * pow<2>(s26) - 12 * pow<2>(me2) * s56 -
        4 * me2 * mm2 * s56 + 16 * me2 * s12 * s56 + 8 * me2 * s15 * s56 +
        16 * me2 * s16 * s56 - 16 * me2 * s25 * s56 - 10 * me2 * s26 * s56 +
        2 * mm2 * s26 * s56 - 4 * s12 * s26 * s56 - 4 * s15 * s26 * s56 -
        8 * s16 * s26 * s56 + 4 * s25 * s26 * s56 + 4 * pow<2>(s26) * s56 -
        8 * me2 * pow<2>(s56) + 4 * s26 * pow<2>(s56)};
    const auto if15{
        -6 * pow<2>(me2) * s12 - 2 * me2 * mm2 * s12 + me2 * pow<2>(s12) -
        mm2 * pow<2>(s12) + 2 * pow<3>(s12) - 6 * pow<2>(me2) * s15 -
        2 * me2 * mm2 * s15 + me2 * s12 * s15 - mm2 * s12 * s15 +
        5 * pow<2>(s12) * s15 + 3 * s12 * pow<2>(s15) - 10 * pow<2>(me2) * s16 -
        6 * me2 * mm2 * s16 + 8 * me2 * s12 * s16 + 3 * pow<2>(s12) * s16 +
        6 * me2 * s15 * s16 + 4 * s12 * s15 * s16 + 6 * me2 * pow<2>(s16) +
        s12 * pow<2>(s16) - 3 * me2 * s12 * s25 - 3 * mm2 * s12 * s25 -
        2 * pow<2>(s12) * s25 - 2 * me2 * s15 * s25 - 2 * mm2 * s15 * s25 -
        3 * s12 * s15 * s25 - 4 * me2 * s16 * s25 - 2 * mm2 * s16 * s25 -
        s12 * s16 * s25 + s15 * s16 * s25 + pow<2>(s16) * s25 +
        2 * mm2 * pow<2>(s25) + 3 * pow<2>(me2) * s26 - 2 * me2 * mm2 * s26 -
        pow<2>(mm2) * s26 + 2 * mm2 * s12 * s26 - 6 * pow<2>(s12) * s26 +
        me2 * s15 * s26 + mm2 * s15 * s26 - 10 * s12 * s15 * s26 -
        2 * pow<2>(s15) * s26 - 4 * me2 * s16 * s26 - 6 * s12 * s16 * s26 -
        2 * s15 * s16 * s26 + me2 * s25 * s26 + 3 * mm2 * s25 * s26 +
        4 * s12 * s25 * s26 + 3 * s15 * s25 * s26 + s16 * s25 * s26 -
        me2 * pow<2>(s26) - mm2 * pow<2>(s26) + 6 * s12 * pow<2>(s26) +
        5 * s15 * pow<2>(s26) + 3 * s16 * pow<2>(s26) - 2 * s25 * pow<2>(s26) -
        2 * pow<3>(s26) + 6 * pow<2>(me2) * s56 + 2 * me2 * mm2 * s56 -
        2 * me2 * s12 * s56 - 5 * pow<2>(s12) * s56 - 7 * s12 * s15 * s56 -
        4 * me2 * s16 * s56 - 5 * s12 * s16 * s56 + 2 * me2 * s25 * s56 +
        4 * mm2 * s25 * s56 + 3 * s12 * s25 * s56 - s16 * s25 * s56 +
        10 * s12 * s26 * s56 + 5 * s15 * s26 * s56 + 3 * s16 * s26 * s56 -
        3 * s25 * s26 * s56 - 5 * pow<2>(s26) * s56 + 4 * s12 * pow<2>(s56) -
        3 * s26 * pow<2>(s56)};
    const auto if25{
        -6 * pow<2>(me2) * s12 - 2 * me2 * mm2 * s12 + me2 * pow<2>(s12) -
        mm2 * pow<2>(s12) + 2 * pow<3>(s12) + 3 * pow<2>(s12) * s15 -
        4 * pow<2>(me2) * s16 - 4 * me2 * mm2 * s16 + me2 * s12 * s16 -
        mm2 * s12 * s16 + 5 * pow<2>(s12) * s16 + 3 * me2 * s15 * s16 +
        3 * s12 * s15 * s16 + 3 * me2 * pow<2>(s16) + 3 * s12 * pow<2>(s16) -
        4 * me2 * s12 * s25 - 2 * mm2 * s12 * s25 - 2 * pow<2>(s12) * s25 -
        4 * me2 * s16 * s25 - mm2 * s16 * s25 - 2 * s12 * s16 * s25 +
        3 * pow<2>(me2) * s26 + 4 * me2 * mm2 * s26 + pow<2>(mm2) * s26 -
        4 * me2 * s12 * s26 - 2 * mm2 * s12 * s26 - 4 * pow<2>(s12) * s26 -
        me2 * s15 * s26 - 2 * mm2 * s15 * s26 - 3 * s12 * s15 * s26 -
        5 * me2 * s16 * s26 - 3 * mm2 * s16 * s26 - 5 * s12 * s16 * s26 +
        2 * me2 * s25 * s26 + 3 * mm2 * s25 * s26 + 2 * s12 * s25 * s26 +
        s16 * s25 * s26 + 3 * me2 * pow<2>(s26) + 2 * mm2 * pow<2>(s26) +
        2 * s12 * pow<2>(s26) + s16 * pow<2>(s26) - me2 * s12 * s56 -
        mm2 * s12 * s56 - 3 * pow<2>(s12) * s56 - 4 * me2 * s16 * s56 -
        3 * s12 * s16 * s56 + 2 * me2 * s26 * s56 + 2 * mm2 * s26 * s56 +
        3 * s12 * s26 * s56 + s16 * s26 * s56};
    const auto if35{
        12 * me2 * mm2 * s12 + 4 * pow<2>(mm2) * s12 - 8 * mm2 * pow<2>(s12) +
        8 * me2 * mm2 * s15 - 3 * me2 * s12 * s15 - 9 * mm2 * s12 * s15 +
        2 * pow<2>(s12) * s15 - 4 * me2 * pow<2>(s15) + 3 * s12 * pow<2>(s15) +
        8 * me2 * mm2 * s16 - 6 * me2 * s12 * s16 - 10 * mm2 * s12 * s16 +
        4 * pow<2>(s12) * s16 - 8 * me2 * s15 * s16 + 7 * s12 * s15 * s16 -
        4 * me2 * pow<2>(s16) + 4 * s12 * pow<2>(s16) - 12 * me2 * mm2 * s25 -
        4 * pow<2>(mm2) * s25 + 16 * mm2 * s12 * s25 + 6 * me2 * s15 * s25 +
        10 * mm2 * s15 * s25 - 6 * s12 * s15 * s25 - 4 * pow<2>(s15) * s25 +
        9 * me2 * s16 * s25 + 11 * mm2 * s16 * s25 - 10 * s12 * s16 * s25 -
        9 * s15 * s16 * s25 - 5 * pow<2>(s16) * s25 - 8 * mm2 * pow<2>(s25) +
        4 * s15 * pow<2>(s25) + 6 * s16 * pow<2>(s25) - 6 * me2 * mm2 * s26 -
        2 * pow<2>(mm2) * s26 + 12 * mm2 * s12 * s26 + 6 * me2 * s15 * s26 +
        8 * mm2 * s15 * s26 - 6 * s12 * s15 * s26 - 4 * pow<2>(s15) * s26 +
        3 * me2 * s16 * s26 + 7 * mm2 * s16 * s26 - 6 * s12 * s16 * s26 -
        7 * s15 * s16 * s26 - 3 * pow<2>(s16) * s26 - 12 * mm2 * s25 * s26 +
        8 * s15 * s25 * s26 + 8 * s16 * s25 * s26 - 4 * mm2 * pow<2>(s26) +
        4 * s15 * pow<2>(s26) + 2 * s16 * pow<2>(s26) - 8 * me2 * mm2 * s56 +
        3 * me2 * s12 * s56 + 9 * mm2 * s12 * s56 - 2 * pow<2>(s12) * s56 +
        8 * me2 * s15 * s56 - 6 * s12 * s15 * s56 + 8 * me2 * s16 * s56 -
        7 * s12 * s16 * s56 - 6 * me2 * s25 * s56 - 10 * mm2 * s25 * s56 +
        6 * s12 * s25 * s56 + 8 * s15 * s25 * s56 + 9 * s16 * s25 * s56 -
        4 * pow<2>(s25) * s56 - 6 * me2 * s26 * s56 - 8 * mm2 * s26 * s56 +
        6 * s12 * s26 * s56 + 8 * s15 * s26 * s56 + 7 * s16 * s26 * s56 -
        8 * s25 * s26 * s56 - 4 * pow<2>(s26) * s56 - 4 * me2 * pow<2>(s56) +
        3 * s12 * pow<2>(s56) - 4 * s25 * pow<2>(s56) - 4 * s26 * pow<2>(s56)};
    const auto if45{
        6 * me2 * pow<2>(s12) + 2 * mm2 * pow<2>(s12) - 4 * pow<3>(s12) +
        4 * me2 * s12 * s15 - 4 * pow<2>(s12) * s15 + 8 * me2 * s12 * s16 +
        2 * mm2 * s12 * s16 - 8 * pow<2>(s12) * s16 + 2 * me2 * s15 * s16 -
        4 * s12 * s15 * s16 + me2 * pow<2>(s16) - 4 * s12 * pow<2>(s16) -
        6 * me2 * s12 * s25 - 2 * mm2 * s12 * s25 + 8 * pow<2>(s12) * s25 +
        4 * s12 * s15 * s25 - 3 * me2 * s16 * s25 - mm2 * s16 * s25 +
        10 * s12 * s16 * s25 + 2 * s15 * s16 * s25 + 2 * pow<2>(s16) * s25 -
        4 * s12 * pow<2>(s25) - 2 * s16 * pow<2>(s25) - 2 * me2 * mm2 * s26 -
        2 * pow<2>(mm2) * s26 - 6 * me2 * s12 * s26 + 2 * mm2 * s12 * s26 +
        8 * pow<2>(s12) * s26 - me2 * s15 * s26 + 5 * mm2 * s15 * s26 +
        3 * s12 * s15 * s26 - 2 * pow<2>(s15) * s26 - 2 * me2 * s16 * s26 +
        6 * mm2 * s16 * s26 + 7 * s12 * s16 * s26 - 5 * s15 * s16 * s26 -
        2 * pow<2>(s16) * s26 + 3 * me2 * s25 * s26 - 3 * mm2 * s25 * s26 -
        10 * s12 * s25 * s26 - 2 * s16 * s25 * s26 + 2 * pow<2>(s25) * s26 +
        3 * me2 * pow<2>(s26) - 2 * mm2 * pow<2>(s26) - 6 * s12 * pow<2>(s26) -
        s16 * pow<2>(s26) + 4 * s25 * pow<2>(s26) + 2 * pow<3>(s26) -
        5 * me2 * s12 * s56 - mm2 * s12 * s56 + 6 * pow<2>(s12) * s56 +
        2 * s12 * s15 * s56 - me2 * s16 * s56 + 6 * s12 * s16 * s56 -
        6 * s12 * s25 * s56 - 2 * s16 * s25 * s56 + 2 * me2 * s26 * s56 -
        3 * mm2 * s26 * s56 - 7 * s12 * s26 * s56 + 2 * s15 * s26 * s56 +
        2 * s16 * s26 * s56 + 2 * s25 * s26 * s56 + 2 * pow<2>(s26) * s56 -
        2 * s12 * pow<2>(s56)};
    const auto if55{
        12 * me2 * mm2 * s12 + 4 * pow<2>(mm2) * s12 - 8 * mm2 * pow<2>(s12) +
        8 * me2 * mm2 * s15 - 8 * mm2 * s12 * s15 + 8 * me2 * mm2 * s16 -
        6 * me2 * s12 * s16 - 10 * mm2 * s12 * s16 + 4 * pow<2>(s12) * s16 -
        4 * me2 * s15 * s16 + 4 * s12 * s15 * s16 - 4 * me2 * pow<2>(s16) +
        4 * s12 * pow<2>(s16) - 12 * me2 * mm2 * s25 - 4 * pow<2>(mm2) * s25 +
        16 * mm2 * s12 * s25 + 8 * mm2 * s15 * s25 + 6 * me2 * s16 * s25 +
        10 * mm2 * s16 * s25 - 8 * s12 * s16 * s25 - 4 * s15 * s16 * s25 -
        4 * pow<2>(s16) * s25 - 8 * mm2 * pow<2>(s25) + 4 * s16 * pow<2>(s25) -
        12 * me2 * mm2 * s26 - 4 * pow<2>(mm2) * s26 + 16 * mm2 * s12 * s26 +
        8 * mm2 * s15 * s26 + 6 * me2 * s16 * s26 + 10 * mm2 * s16 * s26 -
        8 * s12 * s16 * s26 - 4 * s15 * s16 * s26 - 4 * pow<2>(s16) * s26 -
        16 * mm2 * s25 * s26 + 8 * s16 * s25 * s26 - 8 * mm2 * pow<2>(s26) +
        4 * s16 * pow<2>(s26) - 8 * me2 * mm2 * s56 + 8 * mm2 * s12 * s56 +
        4 * me2 * s16 * s56 - 4 * s12 * s16 * s56 - 8 * mm2 * s25 * s56 +
        4 * s16 * s25 * s56 - 8 * mm2 * s26 * s56 + 4 * s16 * s26 * s56};
    const auto if16{
        -12 * pow<2>(me2) * s12 - 4 * me2 * mm2 * s12 + 8 * me2 * pow<2>(s12) -
        12 * pow<2>(me2) * s15 - 4 * me2 * mm2 * s15 + 16 * me2 * s12 * s15 +
        8 * me2 * pow<2>(s15) - 10 * pow<2>(me2) * s16 - 2 * me2 * mm2 * s16 +
        12 * me2 * s12 * s16 + 12 * me2 * s15 * s16 + 2 * me2 * pow<2>(s16) -
        4 * me2 * mm2 * s25 - 9 * me2 * s12 * s25 + mm2 * s12 * s25 -
        pow<2>(s12) * s25 - 6 * me2 * s15 * s25 + 2 * mm2 * s15 * s25 -
        5 * s12 * s15 * s25 - 4 * pow<2>(s15) * s25 - 5 * me2 * s16 * s25 +
        mm2 * s16 * s25 - 2 * s12 * s16 * s25 - 6 * s15 * s16 * s25 -
        pow<2>(s16) * s25 + 2 * mm2 * pow<2>(s25) + s12 * pow<2>(s25) +
        4 * s15 * pow<2>(s25) + s16 * pow<2>(s25) + 12 * pow<2>(me2) * s26 -
        16 * me2 * s12 * s26 - 16 * me2 * s15 * s26 - 8 * me2 * s16 * s26 +
        7 * me2 * s25 * s26 - 3 * mm2 * s25 * s26 + 4 * s12 * s25 * s26 +
        8 * s15 * s25 * s26 + 4 * s16 * s25 * s26 - 3 * pow<2>(s25) * s26 +
        8 * me2 * pow<2>(s26) - 3 * s25 * pow<2>(s26) + 12 * pow<2>(me2) * s56 -
        16 * me2 * s12 * s56 - 16 * me2 * s15 * s56 - 8 * me2 * s16 * s56 +
        6 * me2 * s25 * s56 + 4 * s12 * s25 * s56 + 8 * s15 * s25 * s56 +
        4 * s16 * s25 * s56 - 4 * pow<2>(s25) * s56 + 16 * me2 * s26 * s56 -
        7 * s25 * s26 * s56 + 8 * me2 * pow<2>(s56) - 4 * s25 * pow<2>(s56)};
    const auto if26{
        -12 * pow<2>(me2) * s12 - 4 * me2 * mm2 * s12 + 8 * me2 * pow<2>(s12) +
        8 * me2 * s12 * s15 - 6 * pow<2>(me2) * s16 - 2 * me2 * mm2 * s16 +
        10 * me2 * s12 * s16 + 4 * me2 * s15 * s16 + 3 * me2 * pow<2>(s16) -
        10 * me2 * s12 * s25 + 2 * mm2 * s12 * s25 - 2 * pow<2>(s12) * s25 -
        4 * s12 * s15 * s25 - 5 * me2 * s16 * s25 + mm2 * s16 * s25 -
        3 * s12 * s16 * s25 - 2 * s15 * s16 * s25 - 2 * pow<2>(s16) * s25 +
        4 * s12 * pow<2>(s25) + 2 * s16 * pow<2>(s25) + 6 * pow<2>(me2) * s26 +
        2 * me2 * mm2 * s26 - 13 * me2 * s12 * s26 + mm2 * s12 * s26 -
        pow<2>(s12) * s26 - 7 * me2 * s15 * s26 - mm2 * s15 * s26 +
        2 * pow<2>(s15) * s26 - 10 * me2 * s16 * s26 + 2 * s15 * s16 * s26 +
        5 * me2 * s25 * s26 - 3 * mm2 * s25 * s26 + 6 * s12 * s25 * s26 +
        4 * s16 * s25 * s26 - 2 * pow<2>(s25) * s26 + 6 * me2 * pow<2>(s26) -
        mm2 * pow<2>(s26) + s12 * pow<2>(s26) - 2 * s15 * pow<2>(s26) -
        2 * s25 * pow<2>(s26) - 9 * me2 * s12 * s56 + mm2 * s12 * s56 -
        pow<2>(s12) * s56 - 2 * s12 * s15 * s56 - 7 * me2 * s16 * s56 +
        6 * s12 * s25 * s56 + 2 * s16 * s25 * s56 + 7 * me2 * s26 * s56 -
        2 * mm2 * s26 * s56 + 3 * s12 * s26 * s56 - 2 * s15 * s26 * s56 -
        2 * s25 * s26 * s56 + 2 * s12 * pow<2>(s56)};
    const auto if36{
        12 * pow<2>(me2) * mm2 + 4 * me2 * pow<2>(mm2) - 2 * me2 * mm2 * s12 +
        2 * pow<2>(mm2) * s12 - 4 * mm2 * pow<2>(s12) - 6 * pow<2>(me2) * s15 -
        6 * me2 * mm2 * s15 + 4 * me2 * s12 * s15 - 6 * mm2 * s12 * s15 +
        pow<2>(s12) * s15 + 2 * me2 * pow<2>(s15) + 2 * s12 * pow<2>(s15) -
        6 * pow<2>(me2) * s16 - 6 * me2 * mm2 * s16 + me2 * s12 * s16 -
        3 * mm2 * s12 * s16 + 2 * pow<2>(s12) * s16 + 5 * me2 * s15 * s16 +
        4 * s12 * s15 * s16 + 2 * me2 * pow<2>(s16) + s12 * pow<2>(s16) +
        8 * me2 * mm2 * s25 + 4 * pow<2>(mm2) * s25 + 4 * mm2 * s12 * s25 -
        4 * me2 * s15 * s25 - 2 * mm2 * s15 * s25 - s12 * s15 * s25 -
        5 * me2 * s16 * s25 - 6 * mm2 * s16 * s25 - s12 * s16 * s25 +
        2 * s15 * s16 * s25 + 3 * pow<2>(s16) * s25 - s16 * pow<2>(s25) +
        8 * me2 * mm2 * s26 + 3 * me2 * s12 * s26 + 5 * mm2 * s12 * s26 -
        2 * pow<2>(s12) * s26 - 3 * me2 * s15 * s26 - 3 * s12 * s15 * s26 -
        2 * me2 * s16 * s26 - 4 * s12 * s16 * s26 - 3 * me2 * s25 * s26 +
        4 * s12 * s25 * s26 + 2 * s15 * s25 * s26 - 2 * pow<2>(s25) * s26 -
        3 * me2 * pow<2>(s26) - mm2 * pow<2>(s26) + 4 * s12 * pow<2>(s26) +
        2 * s15 * pow<2>(s26) + 2 * s16 * pow<2>(s26) - 4 * s25 * pow<2>(s26) -
        2 * pow<3>(s26) + 6 * pow<2>(me2) * s56 + 6 * me2 * mm2 * s56 -
        3 * me2 * s12 * s56 + 5 * mm2 * s12 * s56 - pow<2>(s12) * s56 -
        4 * me2 * s15 * s56 - 4 * s12 * s15 * s56 - 3 * me2 * s16 * s56 -
        4 * s12 * s16 * s56 + 4 * me2 * s25 * s56 + 2 * mm2 * s25 * s56 +
        s12 * s25 * s56 - 2 * s16 * s25 * s56 + 2 * me2 * s26 * s56 +
        mm2 * s26 * s56 + 3 * s12 * s26 * s56 - 2 * s25 * s26 * s56 -
        2 * pow<2>(s26) * s56 + 2 * me2 * pow<2>(s56) + 2 * s12 * pow<2>(s56)};
    const auto if46{
        6 * pow<2>(me2) * s12 + 2 * me2 * mm2 * s12 - me2 * pow<2>(s12) +
        mm2 * pow<2>(s12) - 2 * pow<3>(s12) - 3 * pow<2>(s12) * s15 +
        6 * pow<2>(me2) * s16 + 2 * me2 * mm2 * s16 - 5 * me2 * s12 * s16 +
        mm2 * s12 * s16 - 3 * pow<2>(s12) * s16 - me2 * s15 * s16 -
        3 * s12 * s15 * s16 - 3 * me2 * pow<2>(s16) - s12 * pow<2>(s16) +
        4 * me2 * s12 * s25 + 2 * mm2 * s12 * s25 + 2 * pow<2>(s12) * s25 +
        4 * me2 * s16 * s25 + mm2 * s16 * s25 + s12 * s16 * s25 -
        3 * pow<2>(me2) * s26 - pow<2>(mm2) * s26 + 5 * me2 * s12 * s26 +
        mm2 * s12 * s26 + 3 * pow<2>(s12) * s26 - me2 * s15 * s26 +
        2 * mm2 * s15 * s26 + 3 * s12 * s15 * s26 + 5 * me2 * s16 * s26 +
        mm2 * s16 * s26 + 2 * s12 * s16 * s26 - 2 * me2 * s25 * s26 -
        3 * mm2 * s25 * s26 - s12 * s25 * s26 - 2 * me2 * pow<2>(s26) -
        mm2 * pow<2>(s26) - s12 * pow<2>(s26) + 3 * me2 * s12 * s56 +
        mm2 * s12 * s56 + 2 * pow<2>(s12) * s56 + 4 * me2 * s16 * s56 +
        s12 * s16 * s56 - me2 * s26 * s56 - mm2 * s26 * s56 -
        2 * s12 * s26 * s56};
    const auto if56{
        12 * pow<2>(me2) * mm2 + 4 * me2 * pow<2>(mm2) - 2 * me2 * mm2 * s12 +
        2 * pow<2>(mm2) * s12 - 4 * mm2 * pow<2>(s12) - 6 * mm2 * s12 * s15 -
        6 * pow<2>(me2) * s16 - 2 * me2 * mm2 * s16 + 3 * me2 * s12 * s16 -
        3 * mm2 * s12 * s16 + s12 * s15 * s16 - 2 * me2 * pow<2>(s16) +
        s12 * pow<2>(s16) + 8 * me2 * mm2 * s25 + 4 * pow<2>(mm2) * s25 +
        4 * mm2 * s12 * s25 - 4 * me2 * s16 * s25 - 4 * mm2 * s16 * s25 +
        2 * me2 * mm2 * s26 - 2 * pow<2>(mm2) * s26 + 8 * mm2 * s12 * s26 +
        6 * mm2 * s15 * s26 - 3 * me2 * s16 * s26 + 3 * mm2 * s16 * s26 -
        s15 * s16 * s26 - pow<2>(s16) * s26 - 4 * mm2 * s25 * s26 -
        4 * mm2 * pow<2>(s26) + 6 * mm2 * s12 * s56 - s12 * s16 * s56 -
        6 * mm2 * s26 * s56 + s16 * s26 * s56};
    const auto if66{
        24 * pow<2>(me2) * mm2 + 8 * me2 * pow<2>(mm2) - 16 * me2 * mm2 * s12 -
        16 * me2 * mm2 * s15 - 12 * pow<2>(me2) * s16 - 12 * me2 * mm2 * s16 +
        8 * me2 * s12 * s16 + 8 * me2 * s15 * s16 + 4 * me2 * pow<2>(s16) +
        20 * me2 * mm2 * s25 - 4 * pow<2>(mm2) * s25 + 4 * mm2 * s12 * s25 +
        8 * mm2 * s15 * s25 - 10 * me2 * s16 * s25 + 6 * mm2 * s16 * s25 -
        2 * s12 * s16 * s25 - 4 * s15 * s16 * s25 - 2 * pow<2>(s16) * s25 -
        8 * mm2 * pow<2>(s25) + 4 * s16 * pow<2>(s25) + 16 * me2 * mm2 * s26 -
        8 * me2 * s16 * s26 - 4 * mm2 * s25 * s26 + 2 * s16 * s25 * s26 +
        16 * me2 * mm2 * s56 - 8 * me2 * s16 * s56 - 8 * mm2 * s25 * s56 +
        4 * s16 * s25 * s56};

    return if11 / pow<2>(den1) + if22 / pow<2>(den2) + if33 / pow<2>(den3) +
           if44 / pow<2>(den4) + if55 / pow<2>(den5) + if66 / pow<2>(den6) +
           2 * (if12 / (den1 * den2) + if13 / (den1 * den3) +
                if14 / (den1 * den4) + if15 / (den1 * den5) +
                if16 / (den1 * den6) + if23 / (den2 * den3) +
                if24 / (den2 * den4) + if25 / (den2 * den5) +
                if26 / (den2 * den6) + if34 / (den3 * den4) +
                if35 / (den3 * den5) + if36 / (den3 * den6) +
                if45 / (den4 * den5) + if46 / (den4 * den6) +
                if56 / (den5 * den6));
}

auto DoubleRadiativeMuonDecay::MSqPolarizedS6n(double mm2, double me2, double s12, double s15, double s16, double s25, double s26, double s56,
                                               double den1, double den2, double den3, double den4, double den5, double den6) -> double {
    using muc::pow;

    // Adapt from McMule v0.5.0, mudec/mudec_pm2ennggav.f95
    //
    // Copyright 2020-2024  Yannick Ulrich and others (The McMule development team)
    //

    const auto if11{
        24 * pow<3>(me2) + 8 * pow<2>(me2) * mm2 - 16 * pow<2>(me2) * s12 -
        16 * pow<2>(me2) * s15 - 16 * pow<2>(me2) * s16 +
        32 * pow<2>(me2) * s25 - 4 * me2 * s12 * s25 - 4 * me2 * s15 * s25 +
        14 * me2 * pow<2>(s25) + 2 * mm2 * pow<2>(s25) - 4 * s12 * pow<2>(s25) -
        2 * s15 * pow<2>(s25) - 4 * s16 * pow<2>(s25) + 4 * pow<3>(s25) +
        20 * pow<2>(me2) * s26 - 4 * me2 * mm2 * s26 + 4 * me2 * s12 * s26 +
        4 * me2 * s15 * s26 + 8 * me2 * s16 * s26 + 2 * s15 * s25 * s26 +
        4 * pow<2>(s25) * s26 - 8 * me2 * pow<2>(s26) + 20 * pow<2>(me2) * s56 -
        4 * me2 * mm2 * s56 + 4 * me2 * s12 * s56 + 4 * me2 * s15 * s56 +
        8 * me2 * s16 * s56 + 6 * me2 * s25 * s56 + 2 * mm2 * s25 * s56 -
        4 * s12 * s25 * s56 - 2 * s15 * s25 * s56 - 4 * s16 * s25 * s56 +
        8 * pow<2>(s25) * s56 - 16 * me2 * s26 * s56 + 4 * s25 * s26 * s56 -
        8 * me2 * pow<2>(s56) + 4 * s25 * pow<2>(s56)};
    const auto if12{
        24 * pow<3>(me2) + 8 * pow<2>(me2) * mm2 - 16 * pow<2>(me2) * s12 -
        16 * pow<2>(me2) * s15 - 16 * pow<2>(me2) * s16 +
        30 * pow<2>(me2) * s25 + 2 * me2 * mm2 * s25 - 4 * me2 * s12 * s25 -
        4 * me2 * s15 * s25 - 4 * me2 * s16 * s25 + 4 * me2 * pow<2>(s25) -
        2 * mm2 * pow<2>(s25) + 2 * s12 * pow<2>(s25) + 4 * s15 * pow<2>(s25) +
        4 * s16 * pow<2>(s25) - 4 * pow<3>(s25) + 30 * pow<2>(me2) * s26 +
        2 * me2 * mm2 * s26 - 4 * me2 * s12 * s26 - 4 * me2 * s15 * s26 -
        4 * me2 * s16 * s26 + 8 * me2 * s25 * s26 - 4 * mm2 * s25 * s26 +
        4 * s12 * s25 * s26 + 8 * s15 * s25 * s26 + 8 * s16 * s25 * s26 -
        12 * pow<2>(s25) * s26 + 4 * me2 * pow<2>(s26) - 2 * mm2 * pow<2>(s26) +
        2 * s12 * pow<2>(s26) + 4 * s15 * pow<2>(s26) + 4 * s16 * pow<2>(s26) -
        12 * s25 * pow<2>(s26) - 4 * pow<3>(s26) + 24 * pow<2>(me2) * s56 +
        8 * me2 * s25 * s56 - 2 * mm2 * s25 * s56 + 4 * s15 * s25 * s56 +
        4 * s16 * s25 * s56 - 8 * pow<2>(s25) * s56 + 8 * me2 * s26 * s56 -
        2 * mm2 * s26 * s56 + 4 * s15 * s26 * s56 + 4 * s16 * s26 * s56 -
        16 * s25 * s26 * s56 - 8 * pow<2>(s26) * s56 + 4 * me2 * pow<2>(s56) -
        2 * s12 * pow<2>(s56) - 4 * s25 * pow<2>(s56) - 4 * s26 * pow<2>(s56)};
    const auto if22{
        24 * pow<3>(me2) + 8 * pow<2>(me2) * mm2 - 16 * pow<2>(me2) * s12 -
        16 * pow<2>(me2) * s15 - 16 * pow<2>(me2) * s16 +
        24 * pow<2>(me2) * s25 + 4 * me2 * s15 * s25 - 4 * me2 * pow<2>(s25) +
        36 * pow<2>(me2) * s26 + 4 * me2 * mm2 * s26 - 8 * me2 * s12 * s26 -
        4 * me2 * s15 * s26 - 8 * me2 * s16 * s26 + 6 * me2 * s25 * s26 -
        2 * mm2 * s25 * s26 + 2 * s12 * s25 * s26 + 2 * s15 * s25 * s26 +
        4 * s16 * s25 * s26 - 2 * pow<2>(s25) * s26 + 16 * me2 * pow<2>(s26) -
        2 * s12 * pow<2>(s26) - 2 * s15 * pow<2>(s26) + 2 * pow<3>(s26) +
        24 * pow<2>(me2) * s56 + 4 * me2 * s15 * s56 - 8 * me2 * s25 * s56 +
        12 * me2 * s26 * s56 - 2 * s12 * s26 * s56 - 2 * s15 * s26 * s56 +
        4 * pow<2>(s26) * s56 - 4 * me2 * pow<2>(s56) + 2 * s26 * pow<2>(s56)};
    const auto if13{
        -6 * pow<2>(me2) * s12 - 2 * me2 * mm2 * s12 + me2 * pow<2>(s12) -
        mm2 * pow<2>(s12) + 2 * pow<3>(s12) - 4 * pow<2>(me2) * s15 -
        4 * me2 * mm2 * s15 + me2 * s12 * s15 - mm2 * s12 * s15 +
        5 * pow<2>(s12) * s15 + 3 * me2 * pow<2>(s15) + 3 * s12 * pow<2>(s15) +
        3 * pow<2>(s12) * s16 + 3 * me2 * s15 * s16 + 3 * s12 * s15 * s16 +
        3 * pow<2>(me2) * s25 + 4 * me2 * mm2 * s25 + pow<2>(mm2) * s25 -
        4 * me2 * s12 * s25 - 2 * mm2 * s12 * s25 - 4 * pow<2>(s12) * s25 -
        5 * me2 * s15 * s25 - 3 * mm2 * s15 * s25 - 5 * s12 * s15 * s25 -
        me2 * s16 * s25 - 2 * mm2 * s16 * s25 - 3 * s12 * s16 * s25 +
        3 * me2 * pow<2>(s25) + 2 * mm2 * pow<2>(s25) + 2 * s12 * pow<2>(s25) +
        s15 * pow<2>(s25) - 4 * me2 * s12 * s26 - 2 * mm2 * s12 * s26 -
        2 * pow<2>(s12) * s26 - 4 * me2 * s15 * s26 - mm2 * s15 * s26 -
        2 * s12 * s15 * s26 + 2 * me2 * s25 * s26 + 3 * mm2 * s25 * s26 +
        2 * s12 * s25 * s26 + s15 * s25 * s26 - me2 * s12 * s56 -
        mm2 * s12 * s56 - 3 * pow<2>(s12) * s56 - 4 * me2 * s15 * s56 -
        3 * s12 * s15 * s56 + 2 * me2 * s25 * s56 + 2 * mm2 * s25 * s56 +
        3 * s12 * s25 * s56 + s15 * s25 * s56};
    const auto if23{
        -6 * pow<2>(me2) * s12 - 2 * me2 * mm2 * s12 + me2 * pow<2>(s12) -
        mm2 * pow<2>(s12) + 2 * pow<3>(s12) - 10 * pow<2>(me2) * s15 -
        6 * me2 * mm2 * s15 + 8 * me2 * s12 * s15 + 3 * pow<2>(s12) * s15 +
        6 * me2 * pow<2>(s15) + s12 * pow<2>(s15) - 6 * pow<2>(me2) * s16 -
        2 * me2 * mm2 * s16 + me2 * s12 * s16 - mm2 * s12 * s16 +
        5 * pow<2>(s12) * s16 + 6 * me2 * s15 * s16 + 4 * s12 * s15 * s16 +
        3 * s12 * pow<2>(s16) + 3 * pow<2>(me2) * s25 - 2 * me2 * mm2 * s25 -
        pow<2>(mm2) * s25 + 2 * mm2 * s12 * s25 - 6 * pow<2>(s12) * s25 -
        4 * me2 * s15 * s25 - 6 * s12 * s15 * s25 + me2 * s16 * s25 +
        mm2 * s16 * s25 - 10 * s12 * s16 * s25 - 2 * s15 * s16 * s25 -
        2 * pow<2>(s16) * s25 - me2 * pow<2>(s25) - mm2 * pow<2>(s25) +
        6 * s12 * pow<2>(s25) + 3 * s15 * pow<2>(s25) + 5 * s16 * pow<2>(s25) -
        2 * pow<3>(s25) - 3 * me2 * s12 * s26 - 3 * mm2 * s12 * s26 -
        2 * pow<2>(s12) * s26 - 4 * me2 * s15 * s26 - 2 * mm2 * s15 * s26 -
        s12 * s15 * s26 + pow<2>(s15) * s26 - 2 * me2 * s16 * s26 -
        2 * mm2 * s16 * s26 - 3 * s12 * s16 * s26 + s15 * s16 * s26 +
        me2 * s25 * s26 + 3 * mm2 * s25 * s26 + 4 * s12 * s25 * s26 +
        s15 * s25 * s26 + 3 * s16 * s25 * s26 - 2 * pow<2>(s25) * s26 +
        2 * mm2 * pow<2>(s26) + 6 * pow<2>(me2) * s56 + 2 * me2 * mm2 * s56 -
        2 * me2 * s12 * s56 - 5 * pow<2>(s12) * s56 - 4 * me2 * s15 * s56 -
        5 * s12 * s15 * s56 - 7 * s12 * s16 * s56 + 10 * s12 * s25 * s56 +
        3 * s15 * s25 * s56 + 5 * s16 * s25 * s56 - 5 * pow<2>(s25) * s56 +
        2 * me2 * s26 * s56 + 4 * mm2 * s26 * s56 + 3 * s12 * s26 * s56 -
        s15 * s26 * s56 - 3 * s25 * s26 * s56 + 4 * s12 * pow<2>(s56) -
        3 * s25 * pow<2>(s56)};
    const auto if33{
        12 * me2 * mm2 * s12 + 4 * pow<2>(mm2) * s12 - 8 * mm2 * pow<2>(s12) +
        8 * me2 * mm2 * s15 - 6 * me2 * s12 * s15 - 10 * mm2 * s12 * s15 +
        4 * pow<2>(s12) * s15 - 4 * me2 * pow<2>(s15) + 4 * s12 * pow<2>(s15) +
        8 * me2 * mm2 * s16 - 8 * mm2 * s12 * s16 - 4 * me2 * s15 * s16 +
        4 * s12 * s15 * s16 - 12 * me2 * mm2 * s25 - 4 * pow<2>(mm2) * s25 +
        16 * mm2 * s12 * s25 + 6 * me2 * s15 * s25 + 10 * mm2 * s15 * s25 -
        8 * s12 * s15 * s25 - 4 * pow<2>(s15) * s25 + 8 * mm2 * s16 * s25 -
        4 * s15 * s16 * s25 - 8 * mm2 * pow<2>(s25) + 4 * s15 * pow<2>(s25) -
        12 * me2 * mm2 * s26 - 4 * pow<2>(mm2) * s26 + 16 * mm2 * s12 * s26 +
        6 * me2 * s15 * s26 + 10 * mm2 * s15 * s26 - 8 * s12 * s15 * s26 -
        4 * pow<2>(s15) * s26 + 8 * mm2 * s16 * s26 - 4 * s15 * s16 * s26 -
        16 * mm2 * s25 * s26 + 8 * s15 * s25 * s26 - 8 * mm2 * pow<2>(s26) +
        4 * s15 * pow<2>(s26) - 8 * me2 * mm2 * s56 + 8 * mm2 * s12 * s56 +
        4 * me2 * s15 * s56 - 4 * s12 * s15 * s56 - 8 * mm2 * s25 * s56 +
        4 * s15 * s25 * s56 - 8 * mm2 * s26 * s56 + 4 * s15 * s26 * s56};
    const auto if14{
        -12 * pow<2>(me2) * s12 - 4 * me2 * mm2 * s12 + 8 * me2 * pow<2>(s12) -
        6 * pow<2>(me2) * s15 - 2 * me2 * mm2 * s15 + 10 * me2 * s12 * s15 +
        3 * me2 * pow<2>(s15) + 8 * me2 * s12 * s16 + 4 * me2 * s15 * s16 +
        6 * pow<2>(me2) * s25 + 2 * me2 * mm2 * s25 - 13 * me2 * s12 * s25 +
        mm2 * s12 * s25 - pow<2>(s12) * s25 - 10 * me2 * s15 * s25 -
        7 * me2 * s16 * s25 - mm2 * s16 * s25 + 2 * s15 * s16 * s25 +
        2 * pow<2>(s16) * s25 + 6 * me2 * pow<2>(s25) - mm2 * pow<2>(s25) +
        s12 * pow<2>(s25) - 2 * s16 * pow<2>(s25) - 10 * me2 * s12 * s26 +
        2 * mm2 * s12 * s26 - 2 * pow<2>(s12) * s26 - 5 * me2 * s15 * s26 +
        mm2 * s15 * s26 - 3 * s12 * s15 * s26 - 2 * pow<2>(s15) * s26 -
        4 * s12 * s16 * s26 - 2 * s15 * s16 * s26 + 5 * me2 * s25 * s26 -
        3 * mm2 * s25 * s26 + 6 * s12 * s25 * s26 + 4 * s15 * s25 * s26 -
        2 * pow<2>(s25) * s26 + 4 * s12 * pow<2>(s26) + 2 * s15 * pow<2>(s26) -
        2 * s25 * pow<2>(s26) - 9 * me2 * s12 * s56 + mm2 * s12 * s56 -
        pow<2>(s12) * s56 - 7 * me2 * s15 * s56 - 2 * s12 * s16 * s56 +
        7 * me2 * s25 * s56 - 2 * mm2 * s25 * s56 + 3 * s12 * s25 * s56 -
        2 * s16 * s25 * s56 + 6 * s12 * s26 * s56 + 2 * s15 * s26 * s56 -
        2 * s25 * s26 * s56 + 2 * s12 * pow<2>(s56)};
    const auto if24{
        -12 * pow<2>(me2) * s12 - 4 * me2 * mm2 * s12 + 8 * me2 * pow<2>(s12) -
        10 * pow<2>(me2) * s15 - 2 * me2 * mm2 * s15 + 12 * me2 * s12 * s15 +
        2 * me2 * pow<2>(s15) - 12 * pow<2>(me2) * s16 - 4 * me2 * mm2 * s16 +
        16 * me2 * s12 * s16 + 12 * me2 * s15 * s16 + 8 * me2 * pow<2>(s16) +
        12 * pow<2>(me2) * s25 - 16 * me2 * s12 * s25 - 8 * me2 * s15 * s25 -
        16 * me2 * s16 * s25 + 8 * me2 * pow<2>(s25) - 4 * me2 * mm2 * s26 -
        9 * me2 * s12 * s26 + mm2 * s12 * s26 - pow<2>(s12) * s26 -
        5 * me2 * s15 * s26 + mm2 * s15 * s26 - 2 * s12 * s15 * s26 -
        pow<2>(s15) * s26 - 6 * me2 * s16 * s26 + 2 * mm2 * s16 * s26 -
        5 * s12 * s16 * s26 - 6 * s15 * s16 * s26 - 4 * pow<2>(s16) * s26 +
        7 * me2 * s25 * s26 - 3 * mm2 * s25 * s26 + 4 * s12 * s25 * s26 +
        4 * s15 * s25 * s26 + 8 * s16 * s25 * s26 - 3 * pow<2>(s25) * s26 +
        2 * mm2 * pow<2>(s26) + s12 * pow<2>(s26) + s15 * pow<2>(s26) +
        4 * s16 * pow<2>(s26) - 3 * s25 * pow<2>(s26) + 12 * pow<2>(me2) * s56 -
        16 * me2 * s12 * s56 - 8 * me2 * s15 * s56 - 16 * me2 * s16 * s56 +
        16 * me2 * s25 * s56 + 6 * me2 * s26 * s56 + 4 * s12 * s26 * s56 +
        4 * s15 * s26 * s56 + 8 * s16 * s26 * s56 - 7 * s25 * s26 * s56 -
        4 * pow<2>(s26) * s56 + 8 * me2 * pow<2>(s56) - 4 * s26 * pow<2>(s56)};
    const auto if34{
        12 * pow<2>(me2) * mm2 + 4 * me2 * pow<2>(mm2) - 2 * me2 * mm2 * s12 +
        2 * pow<2>(mm2) * s12 - 4 * mm2 * pow<2>(s12) - 6 * pow<2>(me2) * s15 -
        2 * me2 * mm2 * s15 + 3 * me2 * s12 * s15 - 3 * mm2 * s12 * s15 -
        2 * me2 * pow<2>(s15) + s12 * pow<2>(s15) - 6 * mm2 * s12 * s16 +
        s12 * s15 * s16 + 2 * me2 * mm2 * s25 - 2 * pow<2>(mm2) * s25 +
        8 * mm2 * s12 * s25 - 3 * me2 * s15 * s25 + 3 * mm2 * s15 * s25 -
        pow<2>(s15) * s25 + 6 * mm2 * s16 * s25 - s15 * s16 * s25 -
        4 * mm2 * pow<2>(s25) + 8 * me2 * mm2 * s26 + 4 * pow<2>(mm2) * s26 +
        4 * mm2 * s12 * s26 - 4 * me2 * s15 * s26 - 4 * mm2 * s15 * s26 -
        4 * mm2 * s25 * s26 + 6 * mm2 * s12 * s56 - s12 * s15 * s56 -
        6 * mm2 * s25 * s56 + s15 * s25 * s56};
    const auto if44{
        24 * pow<2>(me2) * mm2 + 8 * me2 * pow<2>(mm2) - 16 * me2 * mm2 * s12 -
        12 * pow<2>(me2) * s15 - 12 * me2 * mm2 * s15 + 8 * me2 * s12 * s15 +
        4 * me2 * pow<2>(s15) - 16 * me2 * mm2 * s16 + 8 * me2 * s15 * s16 +
        16 * me2 * mm2 * s25 - 8 * me2 * s15 * s25 + 20 * me2 * mm2 * s26 -
        4 * pow<2>(mm2) * s26 + 4 * mm2 * s12 * s26 - 10 * me2 * s15 * s26 +
        6 * mm2 * s15 * s26 - 2 * s12 * s15 * s26 - 2 * pow<2>(s15) * s26 +
        8 * mm2 * s16 * s26 - 4 * s15 * s16 * s26 - 4 * mm2 * s25 * s26 +
        2 * s15 * s25 * s26 - 8 * mm2 * pow<2>(s26) + 4 * s15 * pow<2>(s26) +
        16 * me2 * mm2 * s56 - 8 * me2 * s15 * s56 - 8 * mm2 * s26 * s56 +
        4 * s15 * s26 * s56};
    const auto if15{
        -6 * pow<2>(me2) * s12 - 2 * me2 * mm2 * s12 + me2 * pow<2>(s12) -
        mm2 * pow<2>(s12) + 2 * pow<3>(s12) - 4 * pow<2>(me2) * s15 +
        me2 * s12 * s15 - mm2 * s12 * s15 + 4 * pow<2>(s12) * s15 -
        2 * me2 * pow<2>(s15) + 2 * s12 * pow<2>(s15) - 6 * pow<2>(me2) * s16 +
        2 * me2 * mm2 * s16 + 4 * me2 * s12 * s16 + 2 * pow<2>(s12) * s16 -
        2 * me2 * s15 * s16 + 2 * s12 * s15 * s16 + 3 * pow<2>(me2) * s25 +
        4 * me2 * mm2 * s25 + pow<2>(mm2) * s25 - 7 * me2 * s12 * s25 -
        3 * mm2 * s12 * s25 - 2 * pow<2>(s12) * s25 - 5 * me2 * s15 * s25 -
        3 * mm2 * s15 * s25 + pow<2>(s15) * s25 - 3 * me2 * s16 * s25 -
        mm2 * s16 * s25 + s15 * s16 * s25 + 3 * me2 * pow<2>(s25) +
        mm2 * pow<2>(s25) - s15 * pow<2>(s25) + s16 * pow<2>(s25) +
        6 * pow<2>(me2) * s26 + 2 * me2 * mm2 * s26 - 5 * me2 * s12 * s26 -
        mm2 * s12 * s26 - 4 * pow<2>(s12) * s26 - 3 * me2 * s15 * s26 -
        mm2 * s15 * s26 - 4 * s12 * s15 * s26 - 4 * me2 * s16 * s26 -
        2 * s12 * s16 * s26 + 9 * me2 * s25 * s26 + 5 * mm2 * s25 * s26 -
        3 * s15 * s25 * s26 - s16 * s25 * s26 + 2 * pow<2>(s25) * s26 +
        4 * me2 * pow<2>(s26) + 2 * mm2 * pow<2>(s26) + 2 * s12 * pow<2>(s26) +
        2 * s25 * pow<2>(s26) + 8 * pow<2>(me2) * s56 + 4 * me2 * mm2 * s56 -
        6 * me2 * s12 * s56 - 4 * pow<2>(s12) * s56 - 4 * s12 * s15 * s56 -
        2 * me2 * s16 * s56 - 2 * s12 * s16 * s56 + 8 * me2 * s25 * s56 +
        4 * mm2 * s25 * s56 - 2 * s15 * s25 * s56 - s16 * s25 * s56 +
        pow<2>(s25) * s56 + 8 * me2 * s26 * s56 + 2 * mm2 * s26 * s56 +
        4 * s12 * s26 * s56 + 3 * s25 * s26 * s56 + 2 * me2 * pow<2>(s56) +
        2 * s12 * pow<2>(s56) + s25 * pow<2>(s56)};
    const auto if25{
        -6 * pow<2>(me2) * s12 - 2 * me2 * mm2 * s12 + me2 * pow<2>(s12) -
        mm2 * pow<2>(s12) + 2 * pow<3>(s12) - 4 * pow<2>(me2) * s15 +
        2 * me2 * s12 * s15 + 3 * pow<2>(s12) * s15 - me2 * pow<2>(s15) -
        6 * pow<2>(me2) * s16 + 2 * me2 * mm2 * s16 - me2 * s12 * s16 -
        mm2 * s12 * s16 + 5 * pow<2>(s12) * s16 - 3 * me2 * s15 * s16 +
        3 * s12 * s15 * s16 - 2 * me2 * pow<2>(s16) + 3 * s12 * pow<2>(s16) +
        3 * pow<2>(me2) * s25 - 2 * me2 * mm2 * s25 - pow<2>(mm2) * s25 -
        4 * me2 * s12 * s25 - 3 * pow<2>(s12) * s25 - 3 * me2 * s15 * s25 +
        mm2 * s15 * s25 + s12 * s15 * s25 - 4 * me2 * s16 * s25 +
        mm2 * s16 * s25 - 2 * s12 * s16 * s25 + 2 * me2 * pow<2>(s25) -
        3 * mm2 * pow<2>(s25) + s12 * pow<2>(s25) + 6 * pow<2>(me2) * s26 +
        2 * me2 * mm2 * s26 - 5 * me2 * s12 * s26 - 3 * mm2 * s12 * s26 -
        3 * pow<2>(s12) * s26 - 3 * me2 * s15 * s26 - 2 * mm2 * s15 * s26 -
        4 * me2 * s16 * s26 - 2 * mm2 * s16 * s26 - 3 * s12 * s16 * s26 +
        5 * me2 * s25 * s26 + 2 * s12 * s25 * s26 - s15 * s25 * s26 +
        4 * me2 * pow<2>(s26) + 2 * mm2 * pow<2>(s26) + s12 * pow<2>(s26) -
        s15 * pow<2>(s26) + 8 * pow<2>(me2) * s56 + 4 * me2 * mm2 * s56 -
        2 * me2 * s12 * s56 - 6 * pow<2>(s12) * s56 - 2 * me2 * s15 * s56 -
        s12 * s15 * s56 - 2 * me2 * s16 * s56 - 5 * s12 * s16 * s56 +
        6 * me2 * s25 * s56 - 2 * mm2 * s25 * s56 + 3 * s12 * s25 * s56 +
        6 * me2 * s26 * s56 + 4 * mm2 * s26 * s56 + 4 * s12 * s26 * s56 -
        s15 * s26 * s56 + 2 * me2 * pow<2>(s56) + 2 * s12 * pow<2>(s56)};
    const auto if35{
        12 * me2 * mm2 * s12 + 4 * pow<2>(mm2) * s12 - 8 * mm2 * pow<2>(s12) +
        8 * me2 * mm2 * s15 - 6 * me2 * s12 * s15 - 10 * mm2 * s12 * s15 +
        4 * pow<2>(s12) * s15 - 4 * me2 * pow<2>(s15) + 4 * s12 * pow<2>(s15) +
        8 * me2 * mm2 * s16 - 3 * me2 * s12 * s16 - 9 * mm2 * s12 * s16 +
        2 * pow<2>(s12) * s16 - 8 * me2 * s15 * s16 + 7 * s12 * s15 * s16 -
        4 * me2 * pow<2>(s16) + 3 * s12 * pow<2>(s16) - 6 * me2 * mm2 * s25 -
        2 * pow<2>(mm2) * s25 + 12 * mm2 * s12 * s25 + 3 * me2 * s15 * s25 +
        7 * mm2 * s15 * s25 - 6 * s12 * s15 * s25 - 3 * pow<2>(s15) * s25 +
        6 * me2 * s16 * s25 + 8 * mm2 * s16 * s25 - 6 * s12 * s16 * s25 -
        7 * s15 * s16 * s25 - 4 * pow<2>(s16) * s25 - 4 * mm2 * pow<2>(s25) +
        2 * s15 * pow<2>(s25) + 4 * s16 * pow<2>(s25) - 12 * me2 * mm2 * s26 -
        4 * pow<2>(mm2) * s26 + 16 * mm2 * s12 * s26 + 9 * me2 * s15 * s26 +
        11 * mm2 * s15 * s26 - 10 * s12 * s15 * s26 - 5 * pow<2>(s15) * s26 +
        6 * me2 * s16 * s26 + 10 * mm2 * s16 * s26 - 6 * s12 * s16 * s26 -
        9 * s15 * s16 * s26 - 4 * pow<2>(s16) * s26 - 12 * mm2 * s25 * s26 +
        8 * s15 * s25 * s26 + 8 * s16 * s25 * s26 - 8 * mm2 * pow<2>(s26) +
        6 * s15 * pow<2>(s26) + 4 * s16 * pow<2>(s26) - 8 * me2 * mm2 * s56 +
        3 * me2 * s12 * s56 + 9 * mm2 * s12 * s56 - 2 * pow<2>(s12) * s56 +
        8 * me2 * s15 * s56 - 7 * s12 * s15 * s56 + 8 * me2 * s16 * s56 -
        6 * s12 * s16 * s56 - 6 * me2 * s25 * s56 - 8 * mm2 * s25 * s56 +
        6 * s12 * s25 * s56 + 7 * s15 * s25 * s56 + 8 * s16 * s25 * s56 -
        4 * pow<2>(s25) * s56 - 6 * me2 * s26 * s56 - 10 * mm2 * s26 * s56 +
        6 * s12 * s26 * s56 + 9 * s15 * s26 * s56 + 8 * s16 * s26 * s56 -
        8 * s25 * s26 * s56 - 4 * pow<2>(s26) * s56 - 4 * me2 * pow<2>(s56) +
        3 * s12 * pow<2>(s56) - 4 * s25 * pow<2>(s56) - 4 * s26 * pow<2>(s56)};
    const auto if45{
        12 * pow<2>(me2) * mm2 + 4 * me2 * pow<2>(mm2) - 2 * me2 * mm2 * s12 +
        2 * pow<2>(mm2) * s12 - 4 * mm2 * pow<2>(s12) - 6 * pow<2>(me2) * s15 -
        6 * me2 * mm2 * s15 + me2 * s12 * s15 - 3 * mm2 * s12 * s15 +
        2 * pow<2>(s12) * s15 + 2 * me2 * pow<2>(s15) + s12 * pow<2>(s15) -
        6 * pow<2>(me2) * s16 - 6 * me2 * mm2 * s16 + 4 * me2 * s12 * s16 -
        6 * mm2 * s12 * s16 + pow<2>(s12) * s16 + 5 * me2 * s15 * s16 +
        4 * s12 * s15 * s16 + 2 * me2 * pow<2>(s16) + 2 * s12 * pow<2>(s16) +
        8 * me2 * mm2 * s25 + 3 * me2 * s12 * s25 + 5 * mm2 * s12 * s25 -
        2 * pow<2>(s12) * s25 - 2 * me2 * s15 * s25 - 4 * s12 * s15 * s25 -
        3 * me2 * s16 * s25 - 3 * s12 * s16 * s25 - 3 * me2 * pow<2>(s25) -
        mm2 * pow<2>(s25) + 4 * s12 * pow<2>(s25) + 2 * s15 * pow<2>(s25) +
        2 * s16 * pow<2>(s25) - 2 * pow<3>(s25) + 8 * me2 * mm2 * s26 +
        4 * pow<2>(mm2) * s26 + 4 * mm2 * s12 * s26 - 5 * me2 * s15 * s26 -
        6 * mm2 * s15 * s26 - s12 * s15 * s26 + 3 * pow<2>(s15) * s26 -
        4 * me2 * s16 * s26 - 2 * mm2 * s16 * s26 - s12 * s16 * s26 +
        2 * s15 * s16 * s26 - 3 * me2 * s25 * s26 + 4 * s12 * s25 * s26 +
        2 * s16 * s25 * s26 - 4 * pow<2>(s25) * s26 - s15 * pow<2>(s26) -
        2 * s25 * pow<2>(s26) + 6 * pow<2>(me2) * s56 + 6 * me2 * mm2 * s56 -
        3 * me2 * s12 * s56 + 5 * mm2 * s12 * s56 - pow<2>(s12) * s56 -
        3 * me2 * s15 * s56 - 4 * s12 * s15 * s56 - 4 * me2 * s16 * s56 -
        4 * s12 * s16 * s56 + 2 * me2 * s25 * s56 + mm2 * s25 * s56 +
        3 * s12 * s25 * s56 - 2 * pow<2>(s25) * s56 + 4 * me2 * s26 * s56 +
        2 * mm2 * s26 * s56 + s12 * s26 * s56 - 2 * s15 * s26 * s56 -
        2 * s25 * s26 * s56 + 2 * me2 * pow<2>(s56) + 2 * s12 * pow<2>(s56)};
    const auto if55{
        12 * me2 * mm2 * s12 + 4 * pow<2>(mm2) * s12 - 8 * mm2 * pow<2>(s12) -
        4 * mm2 * s12 * s15 + 4 * me2 * pow<2>(s15) - 2 * s12 * pow<2>(s15) -
        4 * mm2 * s12 * s16 + 8 * me2 * s15 * s16 - 4 * s12 * s15 * s16 +
        4 * me2 * pow<2>(s16) - 2 * s12 * pow<2>(s16) + 8 * mm2 * s12 * s25 -
        6 * me2 * s15 * s25 - 2 * mm2 * s15 * s25 + 4 * s12 * s15 * s25 +
        4 * pow<2>(s15) * s25 - 6 * me2 * s16 * s25 - 2 * mm2 * s16 * s25 +
        4 * s12 * s16 * s25 + 8 * s15 * s16 * s25 + 4 * pow<2>(s16) * s25 -
        4 * s15 * pow<2>(s25) - 4 * s16 * pow<2>(s25) - 12 * me2 * mm2 * s26 -
        4 * pow<2>(mm2) * s26 + 16 * mm2 * s12 * s26 + 4 * mm2 * s15 * s26 +
        2 * pow<2>(s15) * s26 + 4 * mm2 * s16 * s26 + 4 * s15 * s16 * s26 +
        2 * pow<2>(s16) * s26 - 8 * mm2 * s25 * s26 - 4 * s15 * s25 * s26 -
        4 * s16 * s25 * s26 - 8 * mm2 * pow<2>(s26) + 4 * mm2 * s12 * s56 -
        8 * me2 * s15 * s56 + 4 * s12 * s15 * s56 - 8 * me2 * s16 * s56 +
        4 * s12 * s16 * s56 + 6 * me2 * s25 * s56 + 2 * mm2 * s25 * s56 -
        4 * s12 * s25 * s56 - 8 * s15 * s25 * s56 - 8 * s16 * s25 * s56 +
        4 * pow<2>(s25) * s56 - 4 * mm2 * s26 * s56 - 4 * s15 * s26 * s56 -
        4 * s16 * s26 * s56 + 4 * s25 * s26 * s56 + 4 * me2 * pow<2>(s56) -
        2 * s12 * pow<2>(s56) + 4 * s25 * pow<2>(s56) + 2 * s26 * pow<2>(s56)};
    const auto if16{
        -12 * pow<3>(me2) - 4 * pow<2>(me2) * mm2 + 2 * pow<2>(me2) * s12 -
        2 * me2 * mm2 * s12 + 4 * me2 * pow<2>(s12) + 2 * pow<2>(me2) * s15 -
        2 * me2 * mm2 * s15 + 8 * me2 * s12 * s15 + 4 * me2 * pow<2>(s15) +
        6 * me2 * s12 * s16 + 6 * me2 * s15 * s16 - 20 * pow<2>(me2) * s25 -
        4 * me2 * mm2 * s25 + me2 * s15 * s25 + mm2 * s15 * s25 -
        2 * s12 * s15 * s25 - 2 * pow<2>(s15) * s25 - 3 * s15 * s16 * s25 -
        7 * me2 * pow<2>(s25) + mm2 * pow<2>(s25) + s15 * pow<2>(s25) +
        pow<3>(s25) - 8 * pow<2>(me2) * s26 - 4 * me2 * mm2 * s26 -
        4 * me2 * s12 * s26 - 4 * me2 * s15 * s26 - 6 * me2 * s25 * s26 +
        2 * s15 * s25 * s26 + pow<2>(s25) * s26 - 8 * pow<2>(me2) * s56 -
        4 * me2 * mm2 * s56 - 4 * me2 * s12 * s56 - 4 * me2 * s15 * s56 -
        6 * me2 * s25 * s56 + 2 * mm2 * s25 * s56 + 2 * s15 * s25 * s56 +
        pow<2>(s25) * s56};
    const auto if26{
        -12 * pow<3>(me2) - 4 * pow<2>(me2) * mm2 + 2 * pow<2>(me2) * s12 -
        2 * me2 * mm2 * s12 + 4 * me2 * pow<2>(s12) + 8 * pow<2>(me2) * s15 +
        4 * me2 * s12 * s15 + 6 * me2 * s12 * s16 + me2 * s15 * s16 -
        18 * pow<2>(me2) * s25 - 2 * me2 * mm2 * s25 - 2 * me2 * s12 * s25 +
        2 * me2 * s15 * s25 - s12 * s15 * s25 - 2 * me2 * s16 * s25 +
        mm2 * s16 * s25 - 2 * s15 * s16 * s25 - 2 * pow<2>(s16) * s25 -
        3 * me2 * pow<2>(s25) + mm2 * pow<2>(s25) - 2 * s15 * pow<2>(s25) +
        2 * pow<3>(s25) - 12 * pow<2>(me2) * s26 - 4 * me2 * mm2 * s26 -
        3 * me2 * s12 * s26 - mm2 * s12 * s26 + pow<2>(s12) * s26 -
        mm2 * s15 * s26 + 3 * s12 * s15 * s26 + 2 * pow<2>(s15) * s26 -
        2 * me2 * s16 * s26 + 3 * s12 * s16 * s26 + 2 * s15 * s16 * s26 -
        6 * me2 * s25 * s26 - 3 * mm2 * s25 * s26 - 2 * s15 * s25 * s26 +
        2 * s16 * s25 * s26 + 2 * pow<2>(s25) * s26 - 2 * me2 * pow<2>(s26) -
        2 * mm2 * pow<2>(s26) - s12 * pow<2>(s26) - 2 * s15 * pow<2>(s26) -
        10 * pow<2>(me2) * s56 + 2 * me2 * mm2 * s56 - 4 * me2 * s12 * s56 -
        pow<2>(s12) * s56 - me2 * s15 * s56 - s12 * s15 * s56 -
        4 * me2 * s16 * s56 + s12 * s16 * s56 - 3 * me2 * s25 * s56 +
        3 * s12 * s25 * s56 + 2 * s16 * s25 * s56 + 2 * pow<2>(s25) * s56 -
        2 * me2 * s26 * s56 - 2 * mm2 * s26 * s56 - 2 * s15 * s26 * s56 +
        s12 * pow<2>(s56)};
    const auto if36{
        6 * me2 * pow<2>(s12) + 2 * mm2 * pow<2>(s12) - 4 * pow<3>(s12) +
        8 * me2 * s12 * s15 + 2 * mm2 * s12 * s15 - 8 * pow<2>(s12) * s15 +
        me2 * pow<2>(s15) - 4 * s12 * pow<2>(s15) + 4 * me2 * s12 * s16 -
        4 * pow<2>(s12) * s16 + 2 * me2 * s15 * s16 - 4 * s12 * s15 * s16 -
        2 * me2 * mm2 * s25 - 2 * pow<2>(mm2) * s25 - 6 * me2 * s12 * s25 +
        2 * mm2 * s12 * s25 + 8 * pow<2>(s12) * s25 - 2 * me2 * s15 * s25 +
        6 * mm2 * s15 * s25 + 7 * s12 * s15 * s25 - 2 * pow<2>(s15) * s25 -
        me2 * s16 * s25 + 5 * mm2 * s16 * s25 + 3 * s12 * s16 * s25 -
        5 * s15 * s16 * s25 - 2 * pow<2>(s16) * s25 + 3 * me2 * pow<2>(s25) -
        2 * mm2 * pow<2>(s25) - 6 * s12 * pow<2>(s25) - s15 * pow<2>(s25) +
        2 * pow<3>(s25) - 6 * me2 * s12 * s26 - 2 * mm2 * s12 * s26 +
        8 * pow<2>(s12) * s26 - 3 * me2 * s15 * s26 - mm2 * s15 * s26 +
        10 * s12 * s15 * s26 + 2 * pow<2>(s15) * s26 + 4 * s12 * s16 * s26 +
        2 * s15 * s16 * s26 + 3 * me2 * s25 * s26 - 3 * mm2 * s25 * s26 -
        10 * s12 * s25 * s26 - 2 * s15 * s25 * s26 + 4 * pow<2>(s25) * s26 -
        4 * s12 * pow<2>(s26) - 2 * s15 * pow<2>(s26) + 2 * s25 * pow<2>(s26) -
        5 * me2 * s12 * s56 - mm2 * s12 * s56 + 6 * pow<2>(s12) * s56 -
        me2 * s15 * s56 + 6 * s12 * s15 * s56 + 2 * s12 * s16 * s56 +
        2 * me2 * s25 * s56 - 3 * mm2 * s25 * s56 - 7 * s12 * s25 * s56 +
        2 * s15 * s25 * s56 + 2 * s16 * s25 * s56 + 2 * pow<2>(s25) * s56 -
        6 * s12 * s26 * s56 - 2 * s15 * s26 * s56 + 2 * s25 * s26 * s56 -
        2 * s12 * pow<2>(s56)};
    const auto if46{
        6 * pow<2>(me2) * s12 + 2 * me2 * mm2 * s12 - me2 * pow<2>(s12) +
        mm2 * pow<2>(s12) - 2 * pow<3>(s12) + 6 * pow<2>(me2) * s15 +
        2 * me2 * mm2 * s15 - 5 * me2 * s12 * s15 + mm2 * s12 * s15 -
        3 * pow<2>(s12) * s15 - 3 * me2 * pow<2>(s15) - s12 * pow<2>(s15) -
        3 * pow<2>(s12) * s16 - me2 * s15 * s16 - 3 * s12 * s15 * s16 -
        3 * pow<2>(me2) * s25 - pow<2>(mm2) * s25 + 5 * me2 * s12 * s25 +
        mm2 * s12 * s25 + 3 * pow<2>(s12) * s25 + 5 * me2 * s15 * s25 +
        mm2 * s15 * s25 + 2 * s12 * s15 * s25 - me2 * s16 * s25 +
        2 * mm2 * s16 * s25 + 3 * s12 * s16 * s25 - 2 * me2 * pow<2>(s25) -
        mm2 * pow<2>(s25) - s12 * pow<2>(s25) + 4 * me2 * s12 * s26 +
        2 * mm2 * s12 * s26 + 2 * pow<2>(s12) * s26 + 4 * me2 * s15 * s26 +
        mm2 * s15 * s26 + s12 * s15 * s26 - 2 * me2 * s25 * s26 -
        3 * mm2 * s25 * s26 - s12 * s25 * s26 + 3 * me2 * s12 * s56 +
        mm2 * s12 * s56 + 2 * pow<2>(s12) * s56 + 4 * me2 * s15 * s56 +
        s12 * s15 * s56 - me2 * s25 * s56 - mm2 * s25 * s56 -
        2 * s12 * s25 * s56};
    const auto if56{
        6 * me2 * pow<2>(s12) + 2 * mm2 * pow<2>(s12) - 4 * pow<3>(s12) +
        6 * pow<2>(me2) * s15 + 2 * me2 * mm2 * s15 + 4 * me2 * s12 * s15 +
        2 * mm2 * s12 * s15 - 8 * pow<2>(s12) * s15 - 4 * s12 * pow<2>(s15) +
        6 * pow<2>(me2) * s16 + 2 * me2 * mm2 * s16 - 4 * pow<2>(s12) * s16 -
        2 * me2 * s15 * s16 - 4 * s12 * s15 * s16 - 2 * me2 * pow<2>(s16) -
        6 * me2 * s12 * s25 + 8 * pow<2>(s12) * s25 + 4 * me2 * s15 * s25 +
        2 * mm2 * s15 * s25 + 6 * s12 * s15 * s25 + 5 * me2 * s16 * s25 +
        mm2 * s16 * s25 + 2 * s12 * s16 * s25 - s15 * s16 * s25 -
        pow<2>(s16) * s25 - 4 * s12 * pow<2>(s25) - 12 * me2 * s12 * s26 -
        4 * mm2 * s12 * s26 + 12 * pow<2>(s12) * s26 - 4 * me2 * s15 * s26 -
        2 * mm2 * s15 * s26 + 16 * s12 * s15 * s26 + 4 * pow<2>(s15) * s26 +
        8 * s12 * s16 * s26 + 4 * s15 * s16 * s26 + 6 * me2 * s25 * s26 -
        16 * s12 * s25 * s26 - 6 * s15 * s25 * s26 - 2 * s16 * s25 * s26 +
        4 * pow<2>(s25) * s26 + 6 * me2 * pow<2>(s26) + 2 * mm2 * pow<2>(s26) -
        12 * s12 * pow<2>(s26) - 8 * s15 * pow<2>(s26) - 4 * s16 * pow<2>(s26) +
        8 * s25 * pow<2>(s26) + 4 * pow<3>(s26) - 6 * pow<2>(me2) * s56 -
        2 * me2 * mm2 * s56 - 4 * me2 * s12 * s56 - 2 * mm2 * s12 * s56 +
        8 * pow<2>(s12) * s56 + 8 * s12 * s15 * s56 + 2 * me2 * s16 * s56 +
        4 * s12 * s16 * s56 - 4 * me2 * s25 * s56 - 2 * mm2 * s25 * s56 -
        6 * s12 * s25 * s56 + s16 * s25 * s56 + 4 * me2 * s26 * s56 +
        2 * mm2 * s26 * s56 - 16 * s12 * s26 * s56 - 8 * s15 * s26 * s56 -
        4 * s16 * s26 * s56 + 6 * s25 * s26 * s56 + 8 * pow<2>(s26) * s56 -
        4 * s12 * pow<2>(s56) + 4 * s26 * pow<2>(s56)};
    const auto if66{
        12 * pow<2>(me2) * s12 + 4 * me2 * mm2 * s12 - 8 * me2 * pow<2>(s12) +
        12 * pow<2>(me2) * s15 + 4 * me2 * mm2 * s15 - 16 * me2 * s12 * s15 -
        8 * me2 * pow<2>(s15) + 8 * pow<2>(me2) * s16 - 8 * me2 * s12 * s16 -
        8 * me2 * s15 * s16 + 12 * me2 * s12 * s25 + 10 * me2 * s15 * s25 -
        2 * mm2 * s15 * s25 + 4 * s12 * s15 * s25 + 4 * pow<2>(s15) * s25 +
        8 * me2 * s16 * s25 + 4 * s15 * s16 * s25 - 2 * s12 * pow<2>(s25) -
        4 * s15 * pow<2>(s25) - 2 * s16 * pow<2>(s25) - 12 * pow<2>(me2) * s26 -
        4 * me2 * mm2 * s26 + 16 * me2 * s12 * s26 + 16 * me2 * s15 * s26 +
        8 * me2 * s16 * s26 - 12 * me2 * s25 * s26 - 4 * s15 * s25 * s26 +
        2 * pow<2>(s25) * s26 - 8 * me2 * pow<2>(s26) - 12 * pow<2>(me2) * s56 -
        4 * me2 * mm2 * s56 + 16 * me2 * s12 * s56 + 16 * me2 * s15 * s56 +
        8 * me2 * s16 * s56 - 10 * me2 * s25 * s56 + 2 * mm2 * s25 * s56 -
        4 * s12 * s25 * s56 - 8 * s15 * s25 * s56 - 4 * s16 * s25 * s56 +
        4 * pow<2>(s25) * s56 - 16 * me2 * s26 * s56 + 4 * s25 * s26 * s56 -
        8 * me2 * pow<2>(s56) + 4 * s25 * pow<2>(s56)};

    return if11 / pow<2>(den1) + if22 / pow<2>(den2) + if33 / pow<2>(den3) +
           if44 / pow<2>(den4) + if55 / pow<2>(den5) + if66 / pow<2>(den6) +
           2 * (if12 / (den1 * den2) + if13 / (den1 * den3) +
                if14 / (den1 * den4) + if15 / (den1 * den5) +
                if16 / (den1 * den6) + if23 / (den2 * den3) +
                if24 / (den2 * den4) + if25 / (den2 * den5) +
                if26 / (den2 * den6) + if34 / (den3 * den4) +
                if35 / (den3 * den5) + if36 / (den3 * den6) +
                if45 / (den4 * den5) + if46 / (den4 * den6) +
                if56 / (den5 * den6));
}

} // namespace Mustard::inline Physics::inline Generator
