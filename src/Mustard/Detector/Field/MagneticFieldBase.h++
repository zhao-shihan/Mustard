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

#include "Mustard/Detector/Field/ElectromagneticFieldBase.h++"
#include "Mustard/Detector/Field/MagneticField.h++"
#include "Mustard/Math/GeometryRepresentation.h++"
#include "Mustard/Math/Vector.h++"

#include <concepts>

namespace Mustard::Detector::Field {

template<typename ADerived>
class ElectricFieldBase;

/// @brief CRTP base class for magnetic-field-only implementations.
///
/// Provides a static @c E() that always returns the zero vector, so
/// derived classes only need to implement @c B(). The combined @c BE()
/// method pairs the derived class's @c B() with this zero @c E.
///
/// Inherits from @c ElectromagneticFieldBase<ADerived>.
///
/// @tparam ADerived The derived class (CRTP parameter).
///
/// @note The constructor enforces at compile time:
/// - @c ADerived derives from @c MagneticFieldBase<ADerived>
/// - @c ADerived does NOT derive from @c ElectricFieldBase
/// - @c ADerived satisfies @c MagneticField<ADerived>
template<typename ADerived>
class MagneticFieldBase : public ElectromagneticFieldBase<ADerived> {
public:
    using typename ElectromagneticFieldBase<ADerived>::BEField;

protected:
    /// @brief Default constructor. Enforces CRTP and concept constraints at compile time.
    constexpr MagneticFieldBase();
    /// @brief Defaulted destructor.
    constexpr ~MagneticFieldBase() = default;

public:
    /// @brief Returns the zero vector (no electric field).
    /// @param Unused. Position parameter for interface uniformity.
    /// @return The zero vector @c {0, 0, 0}.
    static auto E(Point3D) -> Vector3D { return {0, 0, 0}; }
    /// @brief Returns the combined B and E field at a point.
    ///
    /// The E component is always zero; the B component is delegated to
    /// @c ADerived::B().
    ///
    /// @param x The position at which to evaluate the field.
    /// @return A @c BEField with @c B = ADerived::B(x) and
    /// @c E = {0, 0, 0}.
    auto BE(Point3D x) const -> BEField { return {static_cast<const ADerived*>(this)->B(x), E(x)}; }
};

} // namespace Mustard::Detector::Field

#include "Mustard/Detector/Field/MagneticFieldBase.inl"
