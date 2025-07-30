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

namespace Mustard::CLHEPX {

template<int N>
    requires(N >= 2)
constexpr EventGenerator<N, internal::AnyRandomStateDim>::EventGenerator(double eCM, const std::array<double, N>& mass) :
    fECM{eCM},
    fMass{mass} {
    if (eCM <= muc::ranges::reduce(fMass)) {
        Throw<std::domain_error>("No enough energy");
    }
}

template<int N>
    requires(N >= 2)
auto EventGenerator<N, internal::AnyRandomStateDim>::operator()() const -> Event {
    return (*this)(*CLHEP::HepRandom::getTheEngine());
}

template<int N>
    requires(N >= 2)
auto EventGenerator<N, internal::AnyRandomStateDim>::operator()(CLHEP::HepRandomEngine& rng, CLHEP::Hep3Vector beta) const -> Event {
    auto [weight, state]{(*this)(rng)};
    for (auto&& p : state) {
        p.boost(beta);
    }
    return {weight, state};
}

template<int N>
    requires(N >= 2)
auto EventGenerator<N, internal::AnyRandomStateDim>::operator()(CLHEP::Hep3Vector beta) const -> Event {
    return (*this)(*CLHEP::HepRandom::getTheEngine(), beta);
}

template<int N, int M>
    requires(N >= 2 and M >= 3 * N - 4)
auto EventGenerator<N, M>::operator()(CLHEP::HepRandomEngine& rng) const -> Event {
    RandomState u;
    rng.flatArray(M, u.data());
    return (*this)(u);
}

template<int N, int M>
    requires(N >= 2 and M >= 3 * N - 4)
auto EventGenerator<N, M>::operator()(const RandomState& u, CLHEP::Hep3Vector beta) const -> Event {
    auto [weight, state]{(*this)(u)};
    for (auto&& p : state) {
        p.boost(beta);
    }
    return {weight, state};
}

} // namespace Mustard::CLHEPX
