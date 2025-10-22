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

#include "Mustard/Concept/InputVector.h++"
#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Detector/Field/ElectricFieldBase.h++"

namespace Mustard::Detector::Field {

class UniformElectricField : public ElectricFieldBase<UniformElectricField> {
public:
    constexpr UniformElectricField(double ex, double ey, double ez);
    template<Concept::InputVector3D T = muc::array3d>
    constexpr UniformElectricField(T e);

    template<Concept::NumericVector3D T>
    constexpr auto E(T) const -> T { return {fEx, fEy, fEz}; }

private:
    double fEx;
    double fEy;
    double fEz;
};

} // namespace Mustard::Detector::Field

#include "Mustard/Detector/Field/UniformElectricField.inl"
