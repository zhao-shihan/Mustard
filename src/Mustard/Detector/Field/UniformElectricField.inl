// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

namespace Mustard::Detector::Field {

constexpr UniformElectricField::UniformElectricField(double ex, double ey, double ez) :
    ElectricFieldBase<UniformElectricField>{},
    fEx{ex},
    fEy{ey},
    fEz{ez} {}

template<Concept::InputVector3D T>
constexpr UniformElectricField::UniformElectricField(T e) :
    UniformElectricField{e[0], e[1], e[2]} {}

} // namespace Mustard::Detector::Field
