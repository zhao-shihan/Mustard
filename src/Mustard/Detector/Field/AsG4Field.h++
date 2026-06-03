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

#include "Mustard/Detector/Field/ElectricField.h++"
#include "Mustard/Detector/Field/ElectromagneticField.h++"
#include "Mustard/Detector/Field/MagneticField.h++"

#include "G4ElectricField.hh"
#include "G4ElectroMagneticField.hh"
#include "G4MagneticField.hh"

#include "muc/array"

#include <type_traits>
#include <utility>

namespace Mustard::Detector::Field {

namespace impl {

/// @brief Helper base that controls the @c DoesFieldChangeEnergy() flag
/// for Geant4 electromagnetic fields.
///
/// @tparam AChangeEnergy If @c true, the field may change particle energy.
template<bool AChangeEnergy>
class G4EMFieldBase : public G4ElectroMagneticField {
public:
    auto DoesFieldChangeEnergy() const -> G4bool override final { return AChangeEnergy; }
};

} // namespace impl

/// @brief Adapter that wraps a Mustard electromagnetic field for use with
/// Geant4.
///
/// The Geant4 base class is selected at compile time based on which Mustard
/// field concept @c AField satisfies:
/// - If @c MagneticField<AField>, inherits from @c G4MagneticField
/// - Else if @c ElectricField<AField>, inherits from @c G4ElectricField
/// - Otherwise, inherits from @c impl::G4EMFieldBase (full EM field)
///
/// This minimizes the Geant4 interface surface: a pure magnetic field
/// exposes only @c GetFieldValue with 3 output components, while an EM
/// field exposes all 6.
///
/// @tparam AField The Mustard field type. Must satisfy
/// @c ElectromagneticField.
/// @tparam AEMFieldChangeEnergy Whether the field changes particle energy.
/// Only meaningful when @c AField is a full EM field (not pure
/// E or B). Default: @c true.
///
/// @warning The Mustard field methods must return field values in CLHEP
/// units expected by Geant4 (tesla for B, volt/m for E). No
/// automatic unit conversion is performed by this adapter.
template<ElectromagneticField AField, bool AEMFieldChangeEnergy = true>
class AsG4Field : public std::conditional_t<MagneticField<AField>,
                                            G4MagneticField,
                                            std::conditional_t<ElectricField<AField>,
                                                               G4ElectricField,
                                                               impl::G4EMFieldBase<AEMFieldChangeEnergy>>>,
                  public AField {
public:
    using AField::AField;

    /// @brief Geant4 callback: evaluates the field at a point.
    ///
    /// Computes the combined BE field via @c AField::BE() and writes the
    /// result into the Geant4 output array.
    ///
    /// @param x Geant4 3D position array (in CLHEP units).
    /// @param f Output array: @c f[0..2] = B field (tesla),
    /// @c f[3..5] = E field (volt/m).
    auto GetFieldValue(const G4double* x, G4double* f) const -> void override final;
};

} // namespace Mustard::Detector::Field

#include "Mustard/Detector/Field/AsG4Field.inl"
