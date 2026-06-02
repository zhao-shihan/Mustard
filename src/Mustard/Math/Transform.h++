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

#include "CLHEP/Geometry/Transform3D.h"

// We use CLHEP geometry as default transform types in Mustard.

namespace Mustard::inline Math {

/// @brief Generic 3D transformation
using Transform3D = HepGeom::Transform3D;

/// @brief 3D rotation
using Rotate3D = HepGeom::Rotate3D;

/// @brief 3D rotation around x-axis
using RotateX3D = HepGeom::RotateX3D;

/// @brief 3D rotation around y-axis
using RotateY3D = HepGeom::RotateY3D;

/// @brief 3D rotation around z-axis
using RotateZ3D = HepGeom::RotateZ3D;

/// @brief 3D translation
using Translate3D = HepGeom::Translate3D;

/// @brief 3D translation along x-axis
using TranslateX3D = HepGeom::TranslateX3D;

/// @brief 3D translation along y-axis
using TranslateY3D = HepGeom::TranslateY3D;

/// @brief 3D translation along z-axis
using TranslateZ3D = HepGeom::TranslateZ3D;

/// @brief 3D reflection
using Reflect3D = HepGeom::Reflect3D;

/// @brief 3D reflection in plane x = a
using ReflectX3D = HepGeom::ReflectX3D;

/// @brief 3D reflection in plane y = a
using ReflectY3D = HepGeom::ReflectY3D;

/// @brief 3D reflection in plane z = a
using ReflectZ3D = HepGeom::ReflectZ3D;

/// @brief 3D scaling
using Scale3D = HepGeom::Scale3D;

/// @brief 3D scaling along x-axis
using ScaleX3D = HepGeom::ScaleX3D;

/// @brief 3D scaling along y-axis
using ScaleY3D = HepGeom::ScaleY3D;

/// @brief 3D scaling along z-axis
using ScaleZ3D = HepGeom::ScaleZ3D;

} // namespace Mustard::inline Math
