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

template<int M, int N, std::derived_from<PolarizedSquaredAmplitude<M, N>> A>
PolarizedMetropolisHastingsGenerator<M, N, A>::PolarizedMetropolisHastingsGenerator(double cmsE, const std::array<CLHEP::Hep3Vector, M>& polarization,
                                                                                    const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                                                                    double delta, int discard) :
    MetropolisHastingsGenerator<M, N, A>{cmsE, pdgID, mass, delta, discard} {
    this->fSquaredAmplitude.InitialStatePolarization(polarization);
}

template<int M, int N, std::derived_from<PolarizedSquaredAmplitude<M, N>> A>
auto PolarizedMetropolisHastingsGenerator<M, N, A>::InitialStatePolarization(int i, CLHEP::Hep3Vector p) -> void {
    if (not p.isNear(InitialStatePolarization(i), muc::default_tolerance<double>)) {
        this->BurnInRequired();
    }
    this->fSquaredAmplitude.InitialStatePolarization(i, p);
}

template<int M, int N, std::derived_from<PolarizedSquaredAmplitude<M, N>> A>
auto PolarizedMetropolisHastingsGenerator<M, N, A>::InitialStatePolarization(const std::array<CLHEP::Hep3Vector, M>& p) -> void {
    if (not std::ranges::equal(p, InitialStatePolarization(),
                               [](auto&& a, auto&& b) { return a.isNear(b, muc::default_tolerance<double>); })) {
        this->BurnInRequired();
    }
    this->fSquaredAmplitude.InitialStatePolarization(p);
}

template<int N, std::derived_from<PolarizedSquaredAmplitude<1, N>> A>
PolarizedMetropolisHastingsGenerator<1, N, A>::PolarizedMetropolisHastingsGenerator(double cmsE, CLHEP::Hep3Vector polarization,
                                                                                    const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                                                                    double delta, int discard) :
    MetropolisHastingsGenerator<1, N, A>{cmsE, pdgID, mass, delta, discard} {
    this->fSquaredAmplitude.InitialStatePolarization(polarization);
}

template<int N, std::derived_from<PolarizedSquaredAmplitude<1, N>> A>
auto PolarizedMetropolisHastingsGenerator<1, N, A>::InitialStatePolarization(CLHEP::Hep3Vector p) -> void {
    if (not p.isNear(InitialStatePolarization(), muc::default_tolerance<double>)) {
        this->BurnInRequired();
    }
    this->fSquaredAmplitude.InitialStatePolarization(p);
}

} // namespace Mustard::inline Physics::inline Generator
