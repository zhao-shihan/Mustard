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

#include "CLHEP/Units/PhysicalConstants.h"
#include "CLHEP/Units/SystemOfUnits.h"

#include "muc/math"

namespace Mustard::inline Utility::PhysicalConstant {

// -- same as G4PhysicalConstants.hh -- //
using CLHEP::alpha_rcl2;
using CLHEP::amu;
using CLHEP::amu_c2;
using CLHEP::Avogadro;
using CLHEP::Bohr_magneton;
using CLHEP::Bohr_radius;
using CLHEP::c_light;
using CLHEP::c_squared;
using CLHEP::classic_electr_radius;
using CLHEP::e_squared;
using CLHEP::electron_charge;
using CLHEP::electron_Compton_length;
using CLHEP::electron_mass_c2;
using CLHEP::elm_coupling;
using CLHEP::epsilon0;
using CLHEP::fine_structure_const;
using CLHEP::h_Planck;
using CLHEP::hbar_Planck;
using CLHEP::hbarc;
using CLHEP::hbarc_squared;
using CLHEP::k_Boltzmann;
using CLHEP::kGasThreshold;
using CLHEP::mu0;
using CLHEP::neutron_mass_c2;
using CLHEP::nuclear_magneton;
using CLHEP::proton_mass_c2;
using CLHEP::STP_Pressure;
using CLHEP::STP_Temperature;
using CLHEP::twopi_mc2_rcl2;
using CLHEP::universe_mean_density;
// -- same as G4PhysicalConstants.hh -- //

// --          Extra usings          -- //
using CLHEP::e_SI;
using CLHEP::eplus;
// --          Extra usings          -- //

// --        Extra constants         -- //
constexpr auto muon_mass_c2 = 105.6583755 * CLHEP::MeV;
constexpr auto muon_lifetime = 2.1969811 * CLHEP::us;
constexpr auto muonium_reduced_mass_c2 = 1 / (1 / electron_mass_c2 + 1 / muon_mass_c2);
constexpr auto muonium_mass_c2 = muon_mass_c2 + electron_mass_c2 -
                                 muc::pow<2>(fine_structure_const * muonium_reduced_mass_c2) / (2 * electron_mass_c2);
constexpr auto muonium_lifetime = muon_lifetime;
// --        Extra constants         -- //

} // namespace Mustard::inline Utility::PhysicalConstant
