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

#include "Mustard/Geant4X/DecayChannel/DecayChannelExtension.h++"
#include "Mustard/Geant4X/DecayChannel/MuonInternalConversionDecayChannelMessenger.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256Plus.h++"
#include "Mustard/Physics/Generator/GENBOD.h++"

#include "G4VDecayChannel.hh"

#include "muc/math"

#include <algorithm>
#include <array>
#include <functional>
#include <tuple>
#include <utility>

namespace Mustard::Geant4X::inline DecayChannel {

class MuonInternalConversionDecayChannel : public G4VDecayChannel,
                                           public DecayChannelExtension {
public:
    enum struct MSqVersion {
        McMule2020,
        RR2009PRD // unpolarized!
    };

public:
    MuonInternalConversionDecayChannel(const G4String& parentName, G4double br, G4int verbose = 1);

    auto MSqVersion(MSqVersion ver) { fMSqVersion = ver; }
    auto MSqVersion(std::string_view ver);

    auto MetropolisDelta(double delta) -> void { fMetropolisDelta = muc::clamp<"()">(delta, 0., 0.5); }
    auto MetropolisDiscard(int n) -> void { fMetropolisDiscard = std::max(0, n); }
    auto Bias(std::function<auto(const GENBOD<1, 5>::FinalStateMomenta&)->double> b) -> void;

    auto Initialize() -> void;
    auto EstimateWeightNormalizationFactor(unsigned long long n) -> std::tuple<double, double, double>; // return: factor, error, Neff

    auto DecayIt(G4double) -> G4DecayProducts* override;

protected:
    auto BiasWithCheck(const GENBOD<1, 5>::FinalStateMomenta& momenta) const -> double;
    auto UpdateState(double delta) -> void;
    auto MainSamplingLoop() -> void;

    auto WeightedMSq(const GENBOD<1, 5>::Event& event) -> double;
    auto MSqMcMule2020(const GENBOD<1, 5>::FinalStateMomenta& momenta) -> double;
    static auto MSqRR2009PRD(const GENBOD<1, 5>::FinalStateMomenta& momenta) -> double;

protected:
    enum MSqVersion fMSqVersion;
    double fMetropolisDelta;
    int fMetropolisDiscard;
    std::function<auto(const GENBOD<1, 5>::FinalStateMomenta&)->double> fBias;

    GENBOD<1, 5> fGENBOD;

    bool fReady;
    GENBOD<1, 5>::RandomState fRandomState;
    GENBOD<1, 5>::Event fEvent;
    double fBiasedMSq;

    Math::Random::Xoshiro256Plus fXoshiro256Plus;
    unsigned fReseedCounter : 8;

private:
    MuonInternalConversionDecayChannelMessenger::Register<MuonInternalConversionDecayChannel> fMessengerRegister;
};

} // namespace Mustard::Geant4X::inline DecayChannel
