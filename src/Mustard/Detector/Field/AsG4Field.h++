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

#pragma once

#include "Mustard/Detector/Field/ElectricField.h++"
#include "Mustard/Detector/Field/ElectromagneticField.h++"
#include "Mustard/Detector/Field/MagneticField.h++"
#include "Mustard/Utility/VectorCast.h++"

#include "G4ElectricField.hh"
#include "G4ElectroMagneticField.hh"
#include "G4MagneticField.hh"

#include "muc/array"

#include <algorithm>
#include <bit>
#include <type_traits>
#include <utility>

namespace Mustard::Detector::Field {

namespace internal {

template<bool AChangeEnergy>
class G4EMFieldBase : public G4ElectroMagneticField {
public:
    auto DoesFieldChangeEnergy() const -> G4bool override final { return AChangeEnergy; }
};

} // namespace internal

template<ElectromagneticField AField, bool AEMFieldChangeEnergy = true>
class AsG4Field : public std::conditional_t<MagneticField<AField>,
                                            G4MagneticField,
                                            std::conditional_t<ElectricField<AField>,
                                                               G4ElectricField,
                                                               internal::G4EMFieldBase<AEMFieldChangeEnergy>>>,
                  public AField {
public:
    using AField::AField;
    virtual ~AsG4Field() = default;

    auto GetFieldValue(const G4double* x, G4double* f) const -> void override final;
};

} // namespace Mustard::Detector::Field

#include "Mustard/Detector/Field/AsG4Field.inl"
