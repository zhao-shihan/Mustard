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

namespace Mustard::Detector::Field {

template<ElectromagneticField AField, bool AEMFieldChangeEnergy>
auto AsG4Field<AField, AEMFieldChangeEnergy>::GetFieldValue(const G4double* x, G4double* f) const -> void {
    std::ranges::copy(std::bit_cast<std::array<G4double, 6>>(
                          this->BE(VectorCast<muc::array3d>(x))),
                      f);
}

} // namespace Mustard::Detector::Field
