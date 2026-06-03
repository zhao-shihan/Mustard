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
#include "Mustard/Detector/Field/ElectromagneticFieldBase.h++"
#include "Mustard/Detector/Field/MagneticFieldBase.h++"
#include "Mustard/Math/GeometryRepresentation.h++"
#include "Mustard/Math/Vector.h++"

#include <concepts>

namespace Mustard::Detector::Field {

template<typename ADerived>
class MagneticFieldBase;

/// @brief CRTP base class for electric-field-only implementations.
///
/// Provides a static @c B() that always returns the zero vector, so
/// derived classes only need to implement @c E(). The combined @c BE()
/// method pairs this zero @c B with the derived class's @c E().
///
/// Inherits from @c ElectromagneticFieldBase<ADerived>.
///
/// @tparam ADerived The derived class (CRTP parameter).
///
/// @note The constructor enforces at compile time:
/// - @c ADerived derives from @c ElectricFieldBase<ADerived>
/// - @c ADerived does NOT derive from @c MagneticFieldBase
/// - @c ADerived satisfies @c ElectricField<ADerived>
template<typename ADerived>
class ElectricFieldBase : public ElectromagneticFieldBase<ADerived> {
public:
    using typename ElectromagneticFieldBase<ADerived>::BEField;

protected:
    /// @brief Default constructor. Enforces CRTP and concept constraints at compile time.
    constexpr ElectricFieldBase();
    /// @brief Defaulted destructor.
    constexpr ~ElectricFieldBase() = default;

public:
    /// @brief Returns the zero vector (no magnetic field).
    /// @param Unused. Position parameter for interface uniformity.
    /// @return The zero vector @c {0, 0, 0}.
    static auto B(Point3D) -> Vector3D { return {0, 0, 0}; }
    /// @brief Returns the combined B and E field at a point.
    ///
    /// The B component is always zero; the E component is delegated to
    /// @c ADerived::E().
    ///
    /// @param x The position at which to evaluate the field.
    /// @return A @c BEField with @c B = {0, 0, 0} and
    /// @c E = ADerived::E(x).
    auto BE(Point3D x) const -> BEField { return {B(x), static_cast<const ADerived*>(this)->E(x)}; }
};

} // namespace Mustard::Detector::Field

#include "Mustard/Detector/Field/ElectricFieldBase.inl"
