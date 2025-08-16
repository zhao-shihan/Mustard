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

#include "Mustard/Data/TupleModel.h++"
#include "Mustard/Data/Value.h++"

#include <vector>

namespace Mustard::Data {

/// @brief Vertex data model for event generators.
/// GeneratedEvent = GeneratedVertex U GeneratedKinematics.
/// @see `GeneratedEvent`
using GeneratedVertex = Mustard::Data::TupleModel<
    Mustard::Data::Value<float, "w", "Vertex weight">,
    Mustard::Data::Value<double, "t0", "Vertex time">,
    Mustard::Data::Value<float, "x", "Vertex X position">,
    Mustard::Data::Value<float, "y", "Vertex Y position">,
    Mustard::Data::Value<float, "z", "Vertex Z position">>;

namespace internal {

using UnweightedGeneratedKinematics = Mustard::Data::TupleModel<
    Mustard::Data::Value<std::vector<int>, "pdgID", "Particle PDG IDs">,
    Mustard::Data::Value<std::vector<float>, "px", "Momentum X components">,
    Mustard::Data::Value<std::vector<float>, "py", "Momentum Y components">,
    Mustard::Data::Value<std::vector<float>, "pz", "Momentum Z components">>;

} // namespace internal

/// @brief Kinematics data model for event generators.
/// GeneratedEvent = GeneratedVertex U GeneratedKinematics.
/// @see `GeneratedEvent`
using GeneratedKinematics = Mustard::Data::TupleModel<
    Mustard::Data::Value<float, "w", "Vertex weight">,
    internal::UnweightedGeneratedKinematics>;

/// @brief Event data model for event generators.
/// GeneratedEvent = GeneratedVertex U GeneratedKinematics.
/// This model is intended to be used with `Geant4X::DataReaderPrimaryGenerator`
/// @see `Geant4X::DataReaderPrimaryGenerator` for a compatible G4 generator
using GeneratedEvent = Mustard::Data::TupleModel<
    GeneratedVertex,
    internal::UnweightedGeneratedKinematics>;

} // namespace Mustard::Data
