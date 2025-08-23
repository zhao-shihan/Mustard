// -*- C++ -*-
//
// Copyright 2020-2025  The Mustard development team
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

namespace Mustard::Geant4X::inline Physics {

template<TargetForMuoniumPhysics ATarget>
MuoniumPhysics<ATarget>::MuoniumPhysics(G4int verbose) :
    G4VPhysicsConstructor{"MuoniumPhysics"} {
    verboseLevel = verbose;
}

template<TargetForMuoniumPhysics ATarget>
auto MuoniumPhysics<ATarget>::ConstructParticle() -> void {
    G4EmBuilder::ConstructMinimalEmSet();
    G4MuonPlus::Definition();
    Muonium::Definition();
    Antimuonium::Definition();
}

template<TargetForMuoniumPhysics ATarget>
auto MuoniumPhysics<ATarget>::ConstructProcess() -> void {
    const auto muoniumFormation{new MuoniumFormation<ATarget>};
    const auto muoniumTransport{new MuoniumTransport<ATarget>};

    const auto muonPlus{G4MuonPlus::Definition()->GetProcessManager()};
    muonPlus->AddRestProcess(muoniumFormation);

    const auto muonium{Muonium::Definition()->GetProcessManager()};
    muonium->AddContinuousProcess(muoniumTransport);

    const auto antiMuonium{Antimuonium::Definition()->GetProcessManager()};
    antiMuonium->AddContinuousProcess(muoniumTransport);
}

} // namespace Mustard::Geant4X::inline Physics
