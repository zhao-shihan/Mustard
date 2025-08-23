// -*- C++ -*-
//
// Copyright 2020-2025  The Mustard development team
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
#include "Mustard/Detector/Field/MagneticFieldBase.h++"

#include "muc/array"

namespace Mustard::Detector::Field {

class UniformMagneticField : public MagneticFieldBase<UniformMagneticField> {
public:
    constexpr UniformMagneticField(double bx, double by, double bz);
    template<Concept::InputVector3D T = muc::array3d>
    constexpr UniformMagneticField(T b);

    template<Concept::NumericVector3D T>
    constexpr auto B(T) const -> T { return {fBx, fBy, fBz}; }

private:
    double fBx;
    double fBy;
    double fBz;
};

} // namespace Mustard::Detector::Field

#include "Mustard/Detector/Field/UniformMagneticField.inl"
