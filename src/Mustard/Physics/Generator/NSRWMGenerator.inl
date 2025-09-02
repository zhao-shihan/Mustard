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
auto NSRWMGenerator<M, N, A>::NSRWMProposeState(CLHEP::HepRandomEngine& rng, double delta,
                                                const MarkovChain::State& state0, MarkovChain::State& state) -> void {
    // Walk random state
    std::ranges::transform(state0.u, state.u.begin(), [&](auto u0) {
        return fGaussian(rng, {u0, delta});
    });
    for (auto&& u : state.u) {
        u = std::abs(muc::fmod(u, 2.)); // Reflection-
        u = u > 1 ? 2 - u : u;          // boundary
    }
    // Walk particle mapping if necessary
    this->ProposePID(rng, state0.pID, state.pID);
}

} // namespace Mustard::inline Physics::inline Generator
