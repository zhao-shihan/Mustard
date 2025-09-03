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

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
ClassicalMetropolisGenerator<M, N, A>::~ClassicalMetropolisGenerator() = default;

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto ClassicalMetropolisGenerator<M, N, A>::NextEvent(CLHEP::HepRandomEngine& rng, double stepSize) -> Event {
    struct NSRWMGenerator<M, N, A>::MarkovChain::State state;
    while (true) {
        this->NSRWMProposeState(rng, stepSize, this->fMarkovChain.state, state);
        auto event{this->PhaseSpace(state)};
        const auto& [detJ, _, pF]{event};
        if (not this->IRSafe(pF)) {
            continue;
        }
        const auto acceptance{this->ValidAcceptance(pF)};
        const auto mSqAcceptanceDetJ{this->ValidMSqAcceptanceDetJ(pF, acceptance, detJ)};
        if (mSqAcceptanceDetJ >= this->fMarkovChain.mSqAcceptanceDetJ or
            mSqAcceptanceDetJ >= this->fMarkovChain.mSqAcceptanceDetJ * rng.flat()) {
            this->fMarkovChain.state = state;
            this->fMarkovChain.mSqAcceptanceDetJ = mSqAcceptanceDetJ;
            event.weight = 1 / acceptance;
            return event;
        }
    }
}

} // namespace Mustard::inline Physics::inline Generator
