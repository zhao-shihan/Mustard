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

#pragma once

#include "Mustard/Data/Model.h++"
#include "Mustard/Data/Object/Value.h++"

#include <vector>

namespace Mustard::Data {

/// @brief Vertex data model for event generators.
/// GeneratedVertex = GeneratedGeometryVertex U GeneratedKinematics.
/// @see `GeneratedVertex`
using GeneratedGeometryVertex = Model<
    Value<float, "w", "Vertex weight">,
    Value<double, "t0", "Vertex time">,
    Value<float, "x", "Vertex X position">,
    Value<float, "y", "Vertex Y position">,
    Value<float, "z", "Vertex Z position">>;

/// @brief Kinematics data model for event generators.
/// GeneratedVertex = GeneratedGeometryVertex U GeneratedKinematics.
/// @see `GeneratedVertex`
using GeneratedKinematics = Model<
    Value<float, "w", "Vertex weight">,
    Value<std::vector<int>, "pdgID", "Particle PDG IDs">,
    Value<std::vector<float>, "E", "Total energy">,
    Value<std::vector<float>, "px", "Momentum X component">,
    Value<std::vector<float>, "py", "Momentum Y component">,
    Value<std::vector<float>, "pz", "Momentum Z component">>;

/// @brief Event data model for event generators.
/// GeneratedVertex = GeneratedGeometryVertex U GeneratedKinematics.
/// This model is intended to be used with `Geant4X::DataReaderPrimaryGenerator`
/// @see `Geant4X::DataReaderPrimaryGenerator` for a compatible G4 generator
using GeneratedVertex = Model<
    GeneratedGeometryVertex,
    Drop<GeneratedKinematics, "w">>;

} // namespace Mustard::Data
