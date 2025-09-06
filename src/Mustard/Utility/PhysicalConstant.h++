// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

#include <algorithm>
#include <vector>

namespace Mustard::inline Utility::PhysicalConstant {

// -- same as CLHEP/Units/PhysicalConstants.h -- // clang-format off
using CLHEP::Avogadro;

using CLHEP::c_light;
using CLHEP::c_squared;

using CLHEP::h_Planck;
using CLHEP::hbar_Planck;
using CLHEP::hbarc;
using CLHEP::hbarc_squared;

using CLHEP::e_squared;
using CLHEP::electron_charge;

using CLHEP::amu;
using CLHEP::amu_c2;
using CLHEP::electron_mass_c2;
using CLHEP::neutron_mass_c2;
using CLHEP::proton_mass_c2;

using CLHEP::epsilon0;
using CLHEP::mu0;

using CLHEP::alpha_rcl2;
using CLHEP::Bohr_magneton;
using CLHEP::Bohr_radius;
using CLHEP::classic_electr_radius;
using CLHEP::electron_Compton_length;
using CLHEP::elm_coupling;
using CLHEP::fine_structure_const;
using CLHEP::nuclear_magneton;
using CLHEP::twopi_mc2_rcl2;

using CLHEP::k_Boltzmann;

using CLHEP::kGasThreshold;
using CLHEP::STP_Pressure;
using CLHEP::STP_Temperature;

using CLHEP::universe_mean_density;
// -- same as CLHEP/Units/PhysicalConstants.h -- // clang-format on

// --              Extra usings               -- //
using CLHEP::e_SI;
using CLHEP::eplus;
// --              Extra usings               -- //

// --            Extra constants              -- //
constexpr auto muon_mass_c2{105.6583755 * CLHEP::MeV};
constexpr auto muonium_reduced_mass_c2{1 / (1 / electron_mass_c2 + 1 / muon_mass_c2)};
constexpr auto muonium_mass_c2{muon_mass_c2 + electron_mass_c2 -
                               muc::pow(fine_structure_const * muonium_reduced_mass_c2, 2) / (2 * electron_mass_c2)};

constexpr auto muon_lifetime{2.1969811 * CLHEP::us};
constexpr auto muonium_lifetime{muon_lifetime};

constexpr auto reduced_fermi_constant{1.1663788e-5 * muc::pow(CLHEP::GeV, -2)};
constexpr auto fermi_constant{reduced_fermi_constant * muc::pow(hbarc, 3)};
// --            Extra constants              -- //

} // namespace Mustard::inline Utility::PhysicalConstant
