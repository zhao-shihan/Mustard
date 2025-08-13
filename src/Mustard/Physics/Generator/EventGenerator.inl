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

template<int M, int N>
auto EventGenerator<M, N>::operator()(const InitialStateMomenta& pI) -> Event {
    return (*this)(pI, *CLHEP::HepRandom::getTheEngine());
}

template<int M, int N>
auto EventGenerator<M, N>::operator()(CLHEP::HepRandomEngine& rng) -> Event {
    return (*this)(InitialStateMomenta{}, rng);
}

template<int M, int N>
auto EventGenerator<M, N>::CalculateCMSEnergy(const InitialStateMomenta& pI) -> double {
    if constexpr (M == 1) {
        return pI.m();
    } else {
        return muc::ranges::reduce(pI).m();
    }
}

template<int M, int N>
auto EventGenerator<M, N>::BoostToCMS(InitialStateMomenta& p) -> CLHEP::Hep3Vector {
    if constexpr (M == 1) {
        p = CLHEP::HepLorentzVector{p.m()};
        return p.boostVector();
    } else {
        const auto beta{muc::ranges::reduce(p).boostVector()};
        std::ranges::for_each(p, [b = -beta](auto&& p) { p.boost(b); });
        return beta;
    }
}

template<int M, int N>
auto EventGenerator<M, N>::BoostToOriginalFrame(CLHEP::Hep3Vector beta, FinalStateMomenta& p) -> void {
    std::ranges::for_each(p, [&beta](auto&& p) { p.boost(beta); });
}

template<int M, int N, int D>
    requires(M >= 1 and N >= 1 and (D == -1 or D >= 3 * N - 4))
auto EventGenerator<M, N, D>::operator()(const RandomState& u) -> Event {
    return (*this)(InitialStateMomenta{}, u);
}

template<int M, int N, int D>
    requires(M >= 1 and N >= 1 and (D == -1 or D >= 3 * N - 4))
auto EventGenerator<M, N, D>::operator()(InitialStateMomenta pI, CLHEP::HepRandomEngine& rng) -> Event {
    RandomState u;
    rng.flatArray(D, u.data());
    return (*this)(std::move(pI), u);
}

} // namespace Mustard::inline Physics::inline Generator
