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

#include "Mustard/Physics/QFT/MSqM2ENNE.h++"
#include "Mustard/Utility/MathConstant.h++"
#include "Mustard/Utility/PhysicalConstant.h++"

#include "muc/math"

namespace Mustard::inline Physics::QFT {

using namespace PhysicalConstant;
using namespace MathConstant;

auto MSqM2ENNE::operator()(const InitialStateMomenta& pI, const FinalStateMomenta& pF) const -> double {
    // Ref: Mitrajyoti Ghosh et al., arXiv:2510.25828 [hep-ph] (Eq. 2)
    const auto& [q1, q2, q3, q4]{pF};
    return 512 * pi * muc::pow(reduced_Fermi_constant, 2) * muc::pow(fine_structure_const, 3) *
           (pI * q2) * (q3 * q4) / muc::hypot_sq(q4.e(), muonium_Bohr_radius / hbarc * (q1.vect() * q4.vect()));
}

} // namespace Mustard::inline Physics::QFT
