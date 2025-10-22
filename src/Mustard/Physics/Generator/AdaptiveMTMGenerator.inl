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

namespace Mustard::inline Physics::inline Generator {

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
AdaptiveMTMGenerator<M, N, A>::AdaptiveMTMGenerator(const InitialStateMomenta& pI, const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                                    std::optional<double> thinningRatio, std::optional<unsigned> acfSampleSize) :
    Base{pI, pdgID, mass, std::move(thinningRatio), std::move(acfSampleSize)},
    fGaussian{},
    fIteration{},
    fLearningRate{},
    fRunningMean{},
    fProposalCovariance{},
    fProposalSigma{} {}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
AdaptiveMTMGenerator<M, N, A>::AdaptiveMTMGenerator(const InitialStateMomenta& pI, CLHEP::Hep3Vector polarization,
                                                    const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                                    std::optional<double> thinningRatio, std::optional<unsigned> acfSampleSize) // clang-format off
    requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>> : // clang-format on
    Base{pI, polarization, pdgID, mass, std::move(thinningRatio), std::move(acfSampleSize)},
    fGaussian{},
    fIteration{},
    fLearningRate{},
    fRunningMean{},
    fProposalCovariance{},
    fProposalSigma{} {}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
AdaptiveMTMGenerator<M, N, A>::AdaptiveMTMGenerator(const InitialStateMomenta& pI, const std::array<CLHEP::Hep3Vector, M>& polarization,
                                                    const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                                    std::optional<double> thinningRatio, std::optional<unsigned> acfSampleSize) // clang-format off
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) : // clang-format on
    Base{pI, polarization, pdgID, mass, std::move(thinningRatio), std::move(acfSampleSize)},
    fGaussian{},
    fIteration{},
    fLearningRate{},
    fRunningMean{},
    fProposalCovariance{},
    fProposalSigma{} {}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
AdaptiveMTMGenerator<M, N, A>::~AdaptiveMTMGenerator() = default;

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto AdaptiveMTMGenerator<M, N, A>::BurnIn(CLHEP::HepRandomEngine& rng) -> void {
    // E(distance in d-dim space) ~ sqrt(d), and E(random walk displacement) ~ sqrt(random walk distance),
    // so we try to ensure E(random walk displacement) >~ scale * E(distance in d-dim space) with some scale
    // i.e. sqrt(random walk distance) >~ scale * sqrt(dimension)
    constexpr auto travelScale{10};
    double distance{};
    while (distance > muc::pow(travelScale, 2) * MarkovChain::dim) {
        if (NextEventImpl(rng, fgInitProposalStepSize)) {
            distance += fgInitProposalStepSize;
        }
    }
    // Then let's learn for a while
    fIteration = 1;
    std::ranges::copy(this->fMC.state.u, fRunningMean.begin());
    fProposalCovariance.setIdentity();
    fProposalCovariance *= muc::pow(fgInitProposalStepSize, 2);
    fProposalSigma.setIdentity();
    fProposalSigma *= fgScalingFactor * fgInitProposalStepSize;
    do {
        NextEvent(rng);
    } while (-fgLearningRatePower * fLearningRate > 1e-6 * fIteration); // delta fLearningRate > 1e-6
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto AdaptiveMTMGenerator<M, N, A>::NextEventImpl(CLHEP::HepRandomEngine& rng, double burnInStepSize) -> bool {
    // Adaptive multiple-try Metropolis sampler (Ref: Simon Fontaine, Mylène Bédard (2022), https://doi.org/10.3150/21-BEJ1408)
    // see also: Jun S. Liu et al (2000), https://doi.org/10.2307/2669532
    std::array<struct MarkovChain::State, fgNTrial> stateY; // y_1, ..., y_k
    std::array<struct Base::Event, fgNTrial> eventY;        // Event at y_1, ..., y_k
    std::array<double, fgNTrial> acceptanceY;               // Acceptance function value at y_1, ..., y_k
    std::array<double, fgNTrial> piY;                       // pi(y_1), ..., pi(y_k)
    struct MarkovChain::State stateX;                       // x_i
    // Symmetric T(x, y)
    const auto ProposeState{[&](const struct MarkovChain::State& state0, struct MarkovChain::State& state) {
        // Walk random state
        if (burnInStepSize) {
            std::ranges::transform(state0.u, state.u.begin(), [&](auto u0) {
                return fGaussian(rng, {u0, burnInStepSize});
            });
        } else {
            Eigen::Vector<double, MarkovChain::dim> vector;
            std::ranges::generate(vector, [&] { return fGaussian(rng); });
            vector = VectorCast<decltype(vector)>(state0.u) + fProposalSigma * vector;
            state.u <<= vector;
        }
        if (std::ranges::any_of(state.u, [](auto u) { return u <= 0 or 1 <= u; })) { // "Xian's half-hearted suggestion"
            state.pID = state0.pID;
            return false;
        } else { // Walk particle mapping if necessary
            this->ProposePID(rng, state0.pID, state.pID);
            return true;
        }
    }};

    // y_1, ..., y_k
    for (int i{}; i < fgNTrial; ++i) {
        if (not ProposeState(this->fMC.state, stateY[i])) { // Draw y_i from T(x, *)
            piY[i] = 0;
            continue;
        }
        double detJ;
        std::tie(eventY[i], detJ) = this->PhaseSpace(stateY[i]); // y_i -> event(y_i) = g(y_i), also get |J|(g(y_i))
        if (not this->IRSafe(eventY[i].p)) {
            piY[i] = 0;
            continue;
        }
        acceptanceY[i] = this->ValidAcceptance(eventY[i].p);                      // g(y_i) -> B(g(y_i))
        piY[i] = this->ValidMSqAcceptanceDetJ(eventY[i].p, acceptanceY[i], detJ); // g(y_i) -> pi(y_i) = |M|²(g(y_i)) × B(g(y_i)) × |J|(g(y_i))
    }
    const auto sumPiY{muc::ranges::reduce(piY)}; // pi(y_1) + ... + pi(y_k)
    const auto selected{[&] {                    // Select Y from y_1, ..., y_k by pi(y_1), ..., pi(y_k)
        const auto u{sumPiY * rng.flat()};
        double c{};
        for (int i{}; i < fgNTrial; ++i) {
            c += piY[i];
            if (u < c) {
                return i;
            }
        }
        return fgNTrial - 1;
    }()};

    // x_1, ..., x_k (note that x_k = x)
    auto sumPiX{this->fMC.mSqAcceptanceDetJ}; // pi(x_1) + ... + pi(x_k)
    for (int i{}; i < fgNTrial - 1; ++i) {
        if (not ProposeState(stateY[selected], stateX)) { // Draw x_i from T(Y, *)
            continue;
        }
        const auto [event, detJ]{this->PhaseSpace(stateX)}; // x_i -> event(x_i) = g(x_i), also get |J|(g(x_i))
        if (not this->IRSafe(event.p)) {
            continue;
        }
        const auto acceptanceX{this->ValidAcceptance(event.p)};             // g(x_i) -> B(g(x_i))
        sumPiX += this->ValidMSqAcceptanceDetJ(event.p, acceptanceX, detJ); // g(x_i) -> pi(x_i) = |M|²(g(x_i)) × B(g(x_i)) × |J|(g(y_i))
    }

    // accept/reject Y
    const auto accepted{sumPiY >= sumPiX or
                        sumPiY > sumPiX * rng.flat()};
    if (accepted) {
        this->fMC.state = stateY[selected];
        this->fMC.mSqAcceptanceDetJ = piY[selected];
        this->fMC.event = std::move(eventY[selected]);
        this->fMC.event.weight = 1 / acceptanceY[selected];
    }

    // Adaptation
    if (not burnInStepSize) {
        ++fIteration;
        fLearningRate = std::pow(fIteration, fgLearningRatePower);
        const auto deltaMu{(VectorCast<decltype(fRunningMean)>(this->fMC.state.u) - fRunningMean).eval()};
        fRunningMean += fLearningRate * deltaMu;
        fProposalCovariance += fLearningRate * (deltaMu * deltaMu.transpose() - fProposalCovariance);
        fProposalSigma = Eigen::LDLT<decltype(fProposalCovariance)>{fProposalCovariance}.matrixL();
        fProposalSigma *= fgScalingFactor;
    }

    return accepted;
}

} // namespace Mustard::inline Physics::inline Generator
