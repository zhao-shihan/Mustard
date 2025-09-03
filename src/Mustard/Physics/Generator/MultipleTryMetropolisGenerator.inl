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
MultipleTryMetropolisGenerator<M, N, A>::~MultipleTryMetropolisGenerator() = default;

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::NextEvent(CLHEP::HepRandomEngine& rng, double stepSize) -> Event {
    // Multiple-try Metropolis sampler (Ref: Jun S. Liu et al (2000), https://doi.org/10.2307/2669532)
    using MarkovChain = struct NSRWMGenerator<M, N, A>::MarkovChain;
    constexpr auto kMTM{MarkovChain::dim};              // k
    std::array<struct MarkovChain::State, kMTM> stateY; // y_1, ..., y_k
    std::array<Event, kMTM> eventY;                     // Event at y_1, ..., y_k
    std::array<double, kMTM> acceptanceY;               // Acceptance function value at y_1, ..., y_k
    std::array<double, kMTM> piY;                       // pi(y_1), ..., pi(y_k)
    struct MarkovChain::State stateX;                   // x_i

    const auto MultinomialSample{[&rng](const std::array<double, kMTM>& pi, double piSum) {
        const auto u{piSum * rng.flat()};
        double c{};
        for (int i{}; i < kMTM; ++i) {
            c += pi[i];
            if (u < c) {
                return i;
            }
        }
        return kMTM - 1;
    }};
    while (true) {
        for (int i{}; i < kMTM; ++i) {
            this->NSRWMProposeState(rng, stepSize, this->fMarkovChain.state, stateY[i]); // Draw y_i from T(x, *)
            eventY[i] = this->PhaseSpace(stateY[i]);                                  // y_i -> event(y_i) = g(y_i)
            const auto& [detJ, _, pF]{eventY[i]};
            if (not this->IRSafe(pF)) {
                piY[i] = 0;
                continue;
            }
            acceptanceY[i] = this->ValidAcceptance(pF);                      // g(y_i) -> B(g(y_i))
            piY[i] = this->ValidMSqAcceptanceDetJ(pF, acceptanceY[i], detJ); // g(y_i) -> pi(y_i) = |M|²(g(y_i)) × B(g(y_i)) × |J|(g(y_i))
        }
        const auto sumPiY{muc::ranges::reduce(piY)};         // pi(y_1) + ... + pi(y_k)
        const auto selected{MultinomialSample(piY, sumPiY)}; // Select Y from y_1, ..., y_k by pi(y_1), ..., pi(y_k)

        auto sumPiX{this->fMarkovChain.mSqAcceptanceDetJ}; // pi(x_1) + ... + pi(x_k)
        for (int i{}; i < kMTM - 1; ++i) {
            this->NSRWMProposeState(rng, stepSize, stateY[selected], stateX); // Draw x_i from T(Y, *)
            const auto [detJ, _, pF]{this->PhaseSpace(stateX)};            // x_i -> event(x_i) = g(x_i)
            if (not this->IRSafe(pF)) {
                continue;
            }
            const auto acceptanceX{this->ValidAcceptance(pF)};             // g(x_i) -> B(g(x_i))
            sumPiX += this->ValidMSqAcceptanceDetJ(pF, acceptanceX, detJ); // g(x_i) -> pi(x_i) = |M|²(g(x_i)) × B(g(x_i)) × |J|(g(y_i))
        }

        // accept/reject Y
        if (sumPiY >= sumPiX or
            sumPiY >= sumPiX * rng.flat()) {
            this->fMarkovChain.state = stateY[selected];
            this->fMarkovChain.mSqAcceptanceDetJ = piY[selected];
            eventY[selected].weight = 1 / acceptanceY[selected];
            return eventY[selected];
        }
    }
}

} // namespace Mustard::inline Physics::inline Generator
