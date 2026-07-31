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

#pragma once

#include "Mustard/Env/ObjectRegistry/Singleton.h++"

#include "G4ParticleDefinition.hh"
#include "G4String.hh"

namespace Mustard::Geant4X::inline Particle {

template<typename ADerived>
class MuoniumBase : public Env::Singleton<ADerived>,
                    public G4ParticleDefinition {
protected:
    MuoniumBase(const G4String& name, int pdgEncoding, int iSpin, int magneticMomentSign);

public:
    static auto Definition() -> auto { return &ADerived::Instance(); }
};

} // namespace Mustard::Geant4X::inline Particle

#include "Mustard/Geant4X/Particle/MuoniumBase.inl"
