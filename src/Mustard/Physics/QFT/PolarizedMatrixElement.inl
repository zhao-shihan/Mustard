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

namespace Mustard::inline Physics::QFT {

template<int M, int N>
PolarizedMatrixElement<M, N>::PolarizedMatrixElement(const InitialStatePolarization& pol) :
    PolarizedMatrixElement{} {
    ISPolarization(pol);
}

template<int M, int N>
auto PolarizedMatrixElement<M, N>::ISPolarization(const InitialStatePolarization& pol) -> void {
    if constexpr (M == 1) {
        const auto polNorm{pol.mag()};
        if (polNorm > 1) [[unlikely]] {
            PrintWarning(fmt::format("Got polarization (pol) with |pol| = {} (expects |pol| <= 1)", polNorm));
        }
        fISPolarization = pol;
    } else {
        for (int i{}; i < M; ++i) {
            ISPolarization(i, pol[i]);
        }
    }
}

template<int M, int N>
auto PolarizedMatrixElement<M, N>::ISPolarization(int i, Vector3D pol) -> void
    requires(M > 1) {
    const auto polNorm{pol.mag()};
    if (polNorm > 1) [[unlikely]] {
        PrintWarning(fmt::format("Got polarization {} (pol) with |pol| = {} (expects |pol| <= 1)", i, polNorm));
    }
    fISPolarization.at(i) = pol;
}

} // namespace Mustard::inline Physics::QFT
