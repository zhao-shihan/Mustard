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
ClassicalMetropolisGenerator<M, N, A>::ClassicalMetropolisGenerator(const InitialStateMomenta& pI, const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                                                    std::optional<double> thinningRatio, std::optional<unsigned> acfSampleSize,
                                                                    std::optional<double> stepSize) :
    Base{pI, pdgID, mass, std::move(thinningRatio), std::move(acfSampleSize)},
    fGaussian{},
    fStepSize{std::numeric_limits<double>::quiet_NaN()} {
    if (stepSize) {
        StepSize(*stepSize);
    }
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
ClassicalMetropolisGenerator<M, N, A>::ClassicalMetropolisGenerator(const InitialStateMomenta& pI, CLHEP::Hep3Vector polarization,
                                                                    const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                                                    std::optional<double> thinningRatio, std::optional<unsigned> acfSampleSize,
                                                                    std::optional<double> stepSize) // clang-format off
    requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>> : // clang-format on
    Base{pI, polarization, pdgID, mass, std::move(thinningRatio), std::move(acfSampleSize)},
    fGaussian{},
    fStepSize{std::numeric_limits<double>::quiet_NaN()} {
    if (stepSize) {
        StepSize(*stepSize);
    }
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
ClassicalMetropolisGenerator<M, N, A>::ClassicalMetropolisGenerator(const InitialStateMomenta& pI, const std::array<CLHEP::Hep3Vector, M>& polarization,
                                                                    const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                                                    std::optional<double> thinningRatio, std::optional<unsigned> acfSampleSize,
                                                                    std::optional<double> stepSize) // clang-format off
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) : // clang-format on
    Base{pI, polarization, pdgID, mass, std::move(thinningRatio), std::move(acfSampleSize)},
    fGaussian{},
    fStepSize{std::numeric_limits<double>::quiet_NaN()} {
    if (stepSize) {
        StepSize(*stepSize);
    }
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
ClassicalMetropolisGenerator<M, N, A>::~ClassicalMetropolisGenerator() = default;

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto ClassicalMetropolisGenerator<M, N, A>::StepSize(double stepSize) -> void {
    if (not std::isfinite(stepSize)) [[unlikely]] {
        PrintError(fmt::format("Infinite MCMC step size (got {})", stepSize));
    }
    if (stepSize <= muc::default_tolerance<double> or 0.5 <= stepSize) [[unlikely]] {
        PrintWarning(fmt::format("Suspicious MCMC step size (got {}, expects {} < step size < 0.5)", stepSize, muc::default_tolerance<double>));
    }
    // Rescale stepSize
    // E(distance in d-dim space) ~ sqrt(d), if stepSize = stepSize0 / sqrt(d) => E(step size) ~ stepSize0
    fStepSize = fgScalingFactor * stepSize;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto ClassicalMetropolisGenerator<M, N, A>::BurnIn(CLHEP::HepRandomEngine& rng) -> void {
    // E(distance in d-dim space) ~ sqrt(d), and E(random walk displacement) ~ sqrt(random walk distance),
    // so we try to ensure E(random walk displacement) >~ scale * E(distance in d-dim space) with some scale
    // i.e. sqrt(random walk distance) >~ scale * sqrt(dimension)
    constexpr auto travelScale{10};
    double distance{};
    while (distance > muc::pow(travelScale, 2) * MarkovChain::dim) {
        if (NextEvent(rng)) {
            distance += fStepSize;
        }
    }
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto ClassicalMetropolisGenerator<M, N, A>::NextEvent(CLHEP::HepRandomEngine& rng) -> bool {
    if (std::isnan(fStepSize)) {
        Throw<std::logic_error>("Step size not set");
    }
    struct MarkovChain::State state;
    // Walk random state
    std::ranges::transform(this->fMC.state.u, state.u.begin(), [&](auto u0) {
        return fGaussian(rng, {u0, fStepSize});
    });
    for (auto&& u : state.u) {
        u = std::abs(muc::fmod(u, 2.)); // Reflection-
        u = u > 1 ? 2 - u : u;          // boundary
    }
    // Walk particle mapping if necessary
    this->ProposePID(rng, this->fMC.state.pID, state.pID);
    auto [event, detJ]{this->PhaseSpace(state)};
    bool accepted{};
    if (this->IRSafe(event.p)) {
        const auto acceptance{this->ValidAcceptance(event.p)};
        const auto mSqAcceptanceDetJ{this->ValidMSqAcceptanceDetJ(event.p, acceptance, detJ)};
        if (mSqAcceptanceDetJ >= this->fMC.mSqAcceptanceDetJ or
            mSqAcceptanceDetJ > this->fMC.mSqAcceptanceDetJ * rng.flat()) {
            this->fMC.state = std::move(state);
            this->fMC.mSqAcceptanceDetJ = mSqAcceptanceDetJ;
            this->fMC.event = std::move(event);
            this->fMC.event.weight = 1 / acceptance;
            accepted = true;
        }
    }
    return accepted;
}

} // namespace Mustard::inline Physics::inline Generator
