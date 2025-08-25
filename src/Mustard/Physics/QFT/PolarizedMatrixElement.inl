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

namespace Mustard::inline Physics::QFT {

template<int M, int N>
PolarizedMatrixElement<M, N>::PolarizedMatrixElement(const std::array<CLHEP::Hep3Vector, M>& pol) :
    PolarizedMatrixElement{} {
    InitialStatePolarization(pol);
}

template<int M, int N>
auto PolarizedMatrixElement<M, N>::InitialStatePolarization(int i, CLHEP::Hep3Vector pol) -> void {
    const auto polNorm{pol.mag()};
    if (polNorm > 1) [[unlikely]] {
        PrintWarning(fmt::format("Got polarization {} (pol) with |pol| = {} (expects |pol| <= 1)", i, polNorm));
    }
    fInitialStatePolarization.at(i) = pol;
}

template<int M, int N>
auto PolarizedMatrixElement<M, N>::InitialStatePolarization(const std::array<CLHEP::Hep3Vector, M>& pol) -> void {
    for (int i{}; i < M; ++i) {
        InitialStatePolarization(i, pol[i]);
    }
}

template<int N>
PolarizedMatrixElement<1, N>::PolarizedMatrixElement(CLHEP::Hep3Vector pol) :
    PolarizedMatrixElement{} {
    InitialStatePolarization(pol);
}

template<int N>
auto PolarizedMatrixElement<1, N>::InitialStatePolarization(CLHEP::Hep3Vector pol) -> void {
    const auto polNorm{pol.mag()};
    if (polNorm > 1) [[unlikely]] {
        PrintWarning(fmt::format("Got polarization (pol) with |pol| = {} (expects |pol| <= 1)", polNorm));
    }
    fInitialStatePolarization = pol;
}

} // namespace Mustard::inline Physics::QFT
