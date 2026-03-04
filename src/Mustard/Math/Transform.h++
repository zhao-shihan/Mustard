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

#include "CLHEP/Vector/AxisAngle.h"
#include "CLHEP/Vector/Boost.h"
#include "CLHEP/Vector/BoostX.h"
#include "CLHEP/Vector/BoostY.h"
#include "CLHEP/Vector/BoostZ.h"
#include "CLHEP/Vector/EulerAngles.h"
#include "CLHEP/Vector/LorentzRotation.h"
#include "CLHEP/Vector/Rotation.h"
#include "CLHEP/Vector/RotationX.h"
#include "CLHEP/Vector/RotationY.h"
#include "CLHEP/Vector/RotationZ.h"

// We use CLHEP vector as default vector types in Mustard.

namespace Mustard::inline Math {

/// @brief Generic boost
using Boost = CLHEP::HepBoost;

/// @brief Boost along x-axis
using BoostX = CLHEP::HepBoostX;

/// @brief Boost along y-axis
using BoostY = CLHEP::HepBoostY;

/// @brief Boost along z-axis
using BoostZ = CLHEP::HepBoostZ;

/// @brief Generic rotation
using Rotation = CLHEP::HepRotation;

/// @brief Rotation around x-axis
using RotationX = CLHEP::HepRotationX;

/// @brief Rotation around y-axis
using RotationY = CLHEP::HepRotationY;

/// @brief Rotation around z-axis
using RotationZ = CLHEP::HepRotationZ;

/// @brief Generic Lorentz transformation (rotation + boost)
using LorentzRotation = CLHEP::HepLorentzRotation;

// below are helper classes for constructing transformation

/// @brief Euler angles (phi, theta, psi) in the z-y-z convention
using EulerAngles = CLHEP::HepEulerAngles;

/// @brief Axis-angle representation of a rotation
using AxisAngle = CLHEP::HepAxisAngle;

} // namespace Mustard::inline Math
