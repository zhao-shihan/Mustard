// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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
    return (*this)(*CLHEP::HepRandom::getTheEngine(), pI);
}

template<int M, int N>
auto EventGenerator<M, N>::operator()(CLHEP::HepRandomEngine& rng) -> Event {
    return (*this)(rng, InitialStateMomenta{});
}

template<int M, int N>
auto EventGenerator<M, N>::CalculateCMEnergy(const InitialStateMomenta& pI) -> double {
    if constexpr (M == 1) {
        return pI.m();
    } else {
        return muc::ranges::reduce(pI).m();
    }
}

template<int M, int N>
auto EventGenerator<M, N>::CalculateBoost(const InitialStateMomenta& pI) -> CLHEP::Hep3Vector {
    if constexpr (M == 1) {
        return pI.boostVector();
    } else {
        return muc::ranges::reduce(pI).boostVector();
    }
}

template<int M, int N>
auto EventGenerator<M, N>::BoostToCMFrame(InitialStateMomenta& pI) -> CLHEP::Hep3Vector {
    const auto beta{CalculateBoost(pI)};
    if constexpr (M == 1) {
        pI = CLHEP::HepLorentzVector{pI.m()};
    } else {
        std::ranges::for_each(pI, [b = -beta](auto&& p) { p.boost(b); });
    }
    return beta;
}

template<int M, int N>
auto EventGenerator<M, N>::BoostToLabFrame(CLHEP::Hep3Vector beta, FinalStateMomenta& pF) -> void {
    std::ranges::for_each(pF, [&beta](auto&& p) { p.boost(beta); });
}

template<int M, int N, int D>
    requires(M >= 1 and N >= 1 and (D == -1 or D >= 3 * N - 4))
auto EventGenerator<M, N, D>::operator()(const RandomState& u) -> Event {
    return (*this)(u, InitialStateMomenta{});
}

template<int M, int N, int D>
    requires(M >= 1 and N >= 1 and (D == -1 or D >= 3 * N - 4))
auto EventGenerator<M, N, D>::operator()(CLHEP::HepRandomEngine& rng, InitialStateMomenta pI) -> Event {
    RandomState u;
    rng.flatArray(u.size(), u.data());
    return (*this)(u, std::move(pI));
}

} // namespace Mustard::inline Physics::inline Generator
