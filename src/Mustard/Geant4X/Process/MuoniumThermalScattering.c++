// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
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

#include "Mustard/Geant4X/Particle/Antimuonium.h++"
#include "Mustard/Geant4X/Particle/Muonium.h++"
#include "Mustard/Geant4X/Process/MuoniumThermalScattering.h++"
#include "Mustard/Utility/LiteralUnit.h++"

#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4ParticleDefinition.hh"

#include "muc/numeric"

namespace Mustard::Geant4X::inline Process {

MuoniumThermalScattering::MuoniumThermalScattering() :
    ThermalScattering{"MUONIUM_MFP"},
    fAirMaterial{},
    fAirComposition{} {}

auto MuoniumThermalScattering::IsApplicable(const G4ParticleDefinition& particle) -> bool {
    return &particle == Geant4X::Muonium::Definition() or
           &particle == Geant4X::Antimuonium::Definition();
}

auto MuoniumThermalScattering::InitializeForNewRun() -> void {
    ThermalScattering::InitializeForNewRun();
    // Set up air material and composition for SimilarToAir check.
    fAirMaterial = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");
    fAirComposition.clear();
    const auto fraction{fAirMaterial->GetFractionVector()};
    const auto nAirElement{static_cast<int>(fAirMaterial->GetNumberOfElements())};
    for (int i{}; i < nAirElement; ++i) {
        fAirComposition[fAirMaterial->GetElement(i)->GetZ()] = fraction[i];
    }
}

auto MuoniumThermalScattering::MeanFreePathIn(const G4Material& material) const -> double {
    if (const auto mfp{ThermalScattering::MeanFreePathIn(material)}; mfp) {
        return mfp;
    }
    if (SimilarToAir(material)) {
        using namespace LiteralUnit;
        return 135_nm * 1.2_kg_m3 / material.GetDensity();
    }
    return 0;
}

auto MuoniumThermalScattering::SimilarToAir(const G4Material& material) const -> bool {
    if (&material == fAirMaterial) {
        return true;
    }
    if (material.GetState() != kStateGas) {
        return false;
    }
    int nAirElementMatched{}; // material elements are distinct in Z, so counting matches suffices
    const auto fractionVector{material.GetFractionVector()};
    const auto nMaterialElement{static_cast<int>(material.GetNumberOfElements())};
    for (int i{}; i < nMaterialElement; ++i) {
        const auto fraction{fractionVector[i]};
        if (const auto found{fAirComposition.find(material.GetElement(i)->GetZ())};
            found != fAirComposition.end()) {
            if (not muc::isclose(fraction, found->second, {.rel = 0.01})) {
                return false; // an air element with mismatched fraction
            }
            ++nAirElementMatched;
        } else if (fraction >= 0.001) {
            return false; // a non-trace element absent from air
        }
    }
    return nAirElementMatched == ssize(fAirComposition); // all air elements must be present
}

} // namespace Mustard::Geant4X::inline Process
