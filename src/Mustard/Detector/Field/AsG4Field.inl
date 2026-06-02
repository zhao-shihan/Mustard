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

namespace Mustard::Detector::Field {

template<ElectromagneticField AField, bool AEMFieldChangeEnergy>
auto AsG4Field<AField, AEMFieldChangeEnergy>::GetFieldValue(const G4double* x, G4double* f) const -> void {
    const auto field{this->BE({x[0], x[1], x[2]})};
    f[0] = field.B[0];
    f[1] = field.B[1];
    f[2] = field.B[2];
    f[3] = field.E[0];
    f[4] = field.E[1];
    f[5] = field.E[2];
}

} // namespace Mustard::Detector::Field
