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

template<typename ADerived>
class MagneticFieldBase : public ElectromagneticFieldBase<ADerived> {
public:
    using typename ElectromagneticFieldBase<ADerived>::BEField;

protected:
    constexpr MagneticFieldBase();
    constexpr ~MagneticFieldBase() = default;

public:
    static auto E(Point3D) -> Vector3D { return {0, 0, 0}; }
    auto BE(Point3D x) const -> BEField { return {static_cast<const ADerived*>(this)->B(x), E(x)}; }
};

} // namespace Mustard::Detector::Field

#include "Mustard/Detector/Field/MagneticFieldBase.inl"
