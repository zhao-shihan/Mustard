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

#include "Mustard/Detector/Field/ElectromagneticField.h++"
#include "Mustard/Math/Vector.h++"

#include <concepts>

namespace Mustard::Detector::Field {

/// @brief CRTP base class for electromagnetic field implementations.
///
/// Provides the @c BEField aggregate type used as the return value of
/// combined BE queries. Derived classes must implement @c B() and @c E()
/// (or inherit zero-valued defaults from @c ElectricFieldBase or
/// @c MagneticFieldBase).
///
/// @tparam ADerived The derived class (CRTP parameter).
///
/// @note The constructor enforces at compile time:
/// - @c ADerived derives from @c ElectromagneticFieldBase<ADerived>
/// - @c ADerived satisfies @c ElectromagneticField<ADerived>
template<typename ADerived>
class ElectromagneticFieldBase {
public:
    /// @brief Aggregate holding both electric and magnetic field vectors.
    struct BEField {
        Vector3D B; ///< Magnetic field vector.
        Vector3D E; ///< Electric field vector.
    };

protected:
    /// @brief Default constructor. Enforces CRTP and concept constraints at compile time.
    constexpr ElectromagneticFieldBase();
    /// @brief Defaulted destructor.
    constexpr ~ElectromagneticFieldBase() = default;
};

} // namespace Mustard::Detector::Field

#include "Mustard/Detector/Field/ElectromagneticFieldBase.inl"
