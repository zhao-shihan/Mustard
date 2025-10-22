// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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

#include "Mustard/Physics/QFT/MSqM2ENNE.h++"
#include "Mustard/Utility/PhysicalConstant.h++"

#include "CLHEP/Vector/LorentzVector.h"

namespace Mustard::inline Physics::QFT {

using namespace PhysicalConstant;

MSqM2ENNE::MSqM2ENNE(Ver ver) :
    MatrixElement{},
    fMSqME2ENNE{ver} {}

auto MSqM2ENNE::operator()(const InitialStateMomenta& pI, const FinalStateMomenta& pF) const -> double {
    CLHEP::HepLorentzVector p1{muon_mass_c2};
    CLHEP::HepLorentzVector p2{electron_mass_c2};
    const auto beta{pI.boostVector()};
    p1.boost(beta);
    p2.boost(beta);
    return muonium_decay_constant * fMSqME2ENNE({p1, p2}, pF);
}

} // namespace Mustard::inline Physics::QFT
