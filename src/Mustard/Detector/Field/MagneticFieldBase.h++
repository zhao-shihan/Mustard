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

#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Detector/Field/ElectromagneticFieldBase.h++"
#include "Mustard/Detector/Field/MagneticField.h++"

#include <concepts>

namespace Mustard::Detector::Field {

template<typename ADerived>
class ElectricFieldBase;

template<typename ADerived>
class MagneticFieldBase : public ElectromagneticFieldBase<ADerived> {
public:
    template<Concept::NumericVector3D T>
    using F = typename ElectromagneticFieldBase<ADerived>::template F<T>;

protected:
    constexpr MagneticFieldBase();
    constexpr ~MagneticFieldBase() = default;

public:
    template<Concept::NumericVector3D T>
    static constexpr auto E(T) -> T { return {0, 0, 0}; }
    template<Concept::NumericVector3D T>
    constexpr auto BE(T x) const -> F<T> { return {static_cast<const ADerived*>(this)->B(x), E(x)}; }
};

} // namespace Mustard::Detector::Field

#include "Mustard/Detector/Field/MagneticFieldBase.inl"
