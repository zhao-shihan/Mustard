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

template<int N>
    requires(N >= 2)
constexpr EventGenerator<N, internal::AnyRandomStateDim>::EventGenerator(const std::array<int, N>& pdgID, const std::array<double, N>& mass) :
    fPDGID{pdgID},
    fMass{mass},
    fSumMass{muc::ranges::reduce(mass)} {}

template<int N>
    requires(N >= 2)
auto EventGenerator<N, internal::AnyRandomStateDim>::operator()(double cmsE) const -> Event {
    return (*this)(cmsE, *CLHEP::HepRandom::getTheEngine());
}

template<int N>
    requires(N >= 2)
auto EventGenerator<N, internal::AnyRandomStateDim>::operator()(CLHEP::HepRandomEngine& rng) const -> Event {
    return (*this)(0., rng);
}

template<int N>
    requires(N >= 2)
auto EventGenerator<N, internal::AnyRandomStateDim>::operator()(double cmsE, CLHEP::Hep3Vector beta, CLHEP::HepRandomEngine& rng) const -> Event {
    auto event{(*this)(cmsE, rng)};
    for (auto&& p : event.p) {
        p.boost(beta);
    }
    return event;
}

template<int N>
    requires(N >= 2)
auto EventGenerator<N, internal::AnyRandomStateDim>::operator()(CLHEP::Hep3Vector beta, CLHEP::HepRandomEngine& rng) const -> Event {
    return (*this)(0., std::move(beta), rng);
}

template<int N>
    requires(N >= 2)
MUSTARD_ALWAYS_INLINE auto EventGenerator<N, internal::AnyRandomStateDim>::CheckCMSEnergy(double cmsE) const -> void {
    if (cmsE <= fSumMass) {
        Throw<std::domain_error>(fmt::format("CMS energy ({}) < sum of final state masses ({})", cmsE, fSumMass));
    }
}

template<int N, int M>
    requires(N >= 2 and M >= 3 * N - 4)
auto EventGenerator<N, M>::operator()(const RandomState& u) const -> Event {
    return (*this)(0., u);
}

template<int N, int M>
    requires(N >= 2 and M >= 3 * N - 4)
auto EventGenerator<N, M>::operator()(double cmsE, CLHEP::Hep3Vector beta, const RandomState& u) const -> Event {
    auto event{(*this)(cmsE, u)};
    for (auto&& p : event.p) {
        p.boost(beta);
    }
    return event;
}

template<int N, int M>
    requires(N >= 2 and M >= 3 * N - 4)
auto EventGenerator<N, M>::operator()(CLHEP::Hep3Vector beta, const RandomState& u) const -> Event {
    return (*this)(0., std::move(beta), u);
}

template<int N, int M>
    requires(N >= 2 and M >= 3 * N - 4)
auto EventGenerator<N, M>::operator()(double cmsE, CLHEP::HepRandomEngine& rng) const -> Event {
    RandomState u;
    rng.flatArray(M, u.data());
    return (*this)(cmsE, u);
}

} // namespace Mustard::inline Physics::inline Generator
