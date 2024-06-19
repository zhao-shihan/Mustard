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

#include "Mustard/Extension/CLHEPX/RAMBO.h++"
#include "Mustard/Extension/Geant4X/DecayChannel/MuonInternalPairProductionDecayChannelMessenger.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256Plus.h++"

#include "G4VDecayChannel.hh"

#include "muc/math"

#include <algorithm>
#include <array>
#include <functional>
#include <utility>

namespace Mustard::inline Extension::Geant4X::inline DecayChannel {

class MuonInternalPairProductionDecayChannel : public G4VDecayChannel {
public:
    MuonInternalPairProductionDecayChannel(const G4String& parentName, G4double br, G4int verbose = 1);

    auto MetropolisDelta(double delta) -> void { fMetropolisDelta = muc::clamp<"()">(delta, 0., 0.5); }
    auto MetropolisDiscard(int n) -> void { fMetropolisDiscard = std::max(0, n); }
    auto PassCut(std::function<bool(const CLHEPX::RAMBO<5>::Event&)> PassCut) -> void { fPassCut = std::move(PassCut); }

    auto DecayIt(G4double) -> G4DecayProducts* override;

private:
    auto UpdateState(double delta) -> void;

    static auto WeightedM2(const CLHEPX::RAMBO<5>::Event& event) -> double;

protected:
    bool fThermalized;

private:
    double fMetropolisDelta;
    int fMetropolisDiscard;
    std::function<bool(const CLHEPX::RAMBO<5>::Event&)> fPassCut;

    CLHEPX::RAMBO<5> fRAMBO;
    std::array<double, 5 * 4> fRawState;
    CLHEPX::RAMBO<5>::Event fEvent;
    double fWeightedM2;

    Math::Random::Xoshiro256Plus fXoshiro256Plus;
    unsigned int fReseedCounter : 8;

    MuonInternalPairProductionDecayChannelMessenger::Register<MuonInternalPairProductionDecayChannel> fMessengerRegister;
};

} // namespace Mustard::inline Extension::Geant4X::inline DecayChannel
