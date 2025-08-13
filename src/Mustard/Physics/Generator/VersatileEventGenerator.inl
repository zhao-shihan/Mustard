// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
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

namespace Mustard::inline Physics::inline Generator {

template<int M, int N, int D>
constexpr VersatileEventGenerator<M, N, D>::VersatileEventGenerator(const std::array<int, N>& pdgID, const std::array<double, N>& mass) :
    EventGenerator<M, N, D>{},
    fPDGID{pdgID},
    fMass{mass},
    fSumMass{muc::ranges::reduce(mass)} {}

template<int M, int N, int D>
MUSTARD_ALWAYS_INLINE auto VersatileEventGenerator<M, N, D>::CheckCMSEnergy(double cmsE, const std::source_location& location) const -> void {
    if (cmsE <= fSumMass) {
        Throw<std::domain_error>(fmt::format("CMS energy ({}) < sum of final state masses ({})", cmsE, fSumMass), location);
    }
}

} // namespace Mustard::inline Physics::inline Generator
