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

namespace Mustard::inline Physics::internal {

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
MultipleTryMetropolisCore<M, N, A>::MultipleTryMetropolisCore(double cmsE, const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                                              double delta, int discard) :
    fCMSEnergy{},
    fSquaredAmplitude{},
    fIRCut{},
    fBias{[](auto&&) { return 1; }},
    fGENBOD{pdgID, mass},
    fMCMCDelta{},
    fMCMCDiscard{},
    fBurntIn{},
    fMarkovChain{},
    fEvent{} {
    CMSEnergy(cmsE);
    MCMCDelta(delta);
    MCMCDiscard(discard);
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
MultipleTryMetropolisCore<M, N, A>::MultipleTryMetropolisCore(double cmsE, CLHEP::Hep3Vector polarization,
                                                              const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                                              double delta, int discard) // clang-format off
    requires std::derived_from<A, PolarizedSquaredAmplitude<1, N>> : // clang-format on
    MultipleTryMetropolisCore{cmsE, pdgID, mass, delta, discard} {
    fSquaredAmplitude.InitialStatePolarization(polarization);
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
MultipleTryMetropolisCore<M, N, A>::MultipleTryMetropolisCore(double cmsE, const std::array<CLHEP::Hep3Vector, M>& polarization,
                                                              const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                                              double delta, int discard) // clang-format off
    requires std::derived_from<A, PolarizedSquaredAmplitude<M, N>> and (M > 1) : // clang-format on
    MultipleTryMetropolisCore{cmsE, pdgID, mass, delta, discard} {
    fSquaredAmplitude.InitialStatePolarization(polarization);
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisCore<M, N, A>::InitialStatePolarization() const -> CLHEP::Hep3Vector
    requires std::derived_from<A, PolarizedSquaredAmplitude<1, N>> {
    return fSquaredAmplitude.InitialStatePolarization();
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisCore<M, N, A>::InitialStatePolarization(int i) const -> CLHEP::Hep3Vector
    requires std::derived_from<A, PolarizedSquaredAmplitude<M, N>> and (M > 1) {
    return fSquaredAmplitude.InitialStatePolarization(i);
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisCore<M, N, A>::InitialStatePolarization() const -> const std::array<CLHEP::Hep3Vector, M>&
    requires std::derived_from<A, PolarizedSquaredAmplitude<M, N>> and (M > 1) {
    return fSquaredAmplitude.InitialStatePolarization();
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisCore<M, N, A>::InitialStatePolarization(CLHEP::Hep3Vector p) -> void
    requires std::derived_from<A, PolarizedSquaredAmplitude<1, N>> {
    if (not p.isNear(InitialStatePolarization(), muc::default_tolerance<double>)) {
        BurnInRequired();
    }
    fSquaredAmplitude.InitialStatePolarization(p);
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisCore<M, N, A>::InitialStatePolarization(int i, CLHEP::Hep3Vector p) -> void
    requires std::derived_from<A, PolarizedSquaredAmplitude<M, N>> and (M > 1) {
    if (not p.isNear(InitialStatePolarization(i), muc::default_tolerance<double>)) {
        BurnInRequired();
    }
    fSquaredAmplitude.InitialStatePolarization(i, p);
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisCore<M, N, A>::InitialStatePolarization(const std::array<CLHEP::Hep3Vector, M>& p) -> void
    requires std::derived_from<A, PolarizedSquaredAmplitude<M, N>> and (M > 1) {
    if (not std::ranges::equal(p, InitialStatePolarization(),
                               [](auto&& a, auto&& b) { return a.isNear(b, muc::default_tolerance<double>); })) {
        BurnInRequired();
    }
    fSquaredAmplitude.InitialStatePolarization(p);
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisCore<M, N, A>::Bias(BiasFunction B) -> void {
    fBias = std::move(B);
    BurnInRequired();
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisCore<M, N, A>::MCMCDelta(double delta) -> void {
    if (delta <= 0 or 0.5 <= delta) [[unlikely]] {
        PrintWarning(fmt::format("Suspicious MCMC delta (got {}, expects 0 < delta < 0.5)", delta));
    }
    fMCMCDelta = delta;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisCore<M, N, A>::MCMCDiscard(int n) -> void {
    if (n < 0) [[unlikely]] {
        PrintWarning(fmt::format("Negative discarded MCMC samples (got {})", n));
    }
    fMCMCDiscard = n;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisCore<M, N, A>::BurnIn(CLHEP::HepRandomEngine& rng) -> void {
    if (fBurntIn) {
        return;
    }
    // find phase space
    while (true) {
        std::ranges::generate(fMarkovChain.state, [&rng] { return rng.flat(); });
        fEvent = fGENBOD({fCMSEnergy, {}}, fMarkovChain.state);
        if (not IRSafe(fEvent.p)) {
            continue;
        }
        if (const auto bias{ValidBias(fEvent.p)};
            bias > std::numeric_limits<double>::min()) {
            fMarkovChain.acceptance = ValidBiasedPDF(fEvent, bias);
            break;
        }
    }
    // burning in
    constexpr auto delta0{0.1};
    constexpr auto epsilon{muc::default_tolerance<double>};
    constexpr auto nBurnIn{10000. * fgMCMCDim}; // E(distance in d-dim space) ~ sqrt(d), E(random walk distance) ~ sqrt(n) => n ~ d
    const auto factor{std::pow(epsilon / delta0, 1 / nBurnIn)};
    for (auto delta{delta0}; delta > epsilon; delta *= factor) {
        NextEvent(delta, rng);
    }
    fBurntIn = true;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisCore<M, N, A>::CMSEnergy(double cmsE) -> void {
    if (cmsE <= 0) [[unlikely]] {
        PrintError(fmt::format("Non-positive CMS energy (got {})", cmsE));
    }
    if (not muc::isclose(cmsE, fCMSEnergy)) {
        BurnInRequired();
    }
    fCMSEnergy = cmsE;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisCore<M, N, A>::Mass(const std::array<double, N>& mass) -> void {
    if (not std::ranges::equal(mass, fGENBOD.Mass(),
                               [](auto a, auto b) { return muc::isclose(a, b); })) {
        BurnInRequired();
    }
    fGENBOD.Mass(mass);
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisCore<M, N, A>::IRCut(int i, double cut) -> void {
    if (cut < 0) [[unlikely]] {
        PrintWarning(fmt::format("Negative IR cut for particle {} (got {})", i, cut));
    }
    if (muc::pow(fGENBOD.Mass(i) / fCMSEnergy, 2) > muc::default_tolerance<double>) [[unlikely]] {
        PrintWarning(fmt::format("IR cut set for massive particle {} (mass = {})", i, fGENBOD.Mass(i)));
    }
    if (not muc::isclose(cut, fIRCut.at(i))) {
        BurnInRequired();
    }
    fIRCut[i] = cut;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisCore<M, N, A>::NextEvent(double delta, CLHEP::HepRandomEngine& rng) -> void {
    // Rescale delta first
    // E(distance in d-dim space) ~ sqrt(d), if delta = delta0 / sqrt(d) => E(step size) ~ delta0
    delta /= std::sqrt(fgMCMCDim);
    // Multiple-try Metropolis sampler (Ref: Jun S. Liu et al (2000), https://doi.org/10.2307/2669532)
    using State = typename GENBOD<M, N>::RandomState;
    constexpr auto kMTM{fgMCMCDim};                                   // k
    std::array<State, kMTM> stateY;                                   // y_1, ..., y_k
    std::array<double, kMTM> piY;                                     // pi(y_1), ..., pi(y_k)
    State stateX;                                                     // x_1, ..., x_k-1
    std::array<double, kMTM - 1> piX;                                 // pi(x_1), ..., pi(x_k-1)
    std::array<double, kMTM> biasY;                                   // Bias function value at y_1, ..., y_k
    std::array<Event, kMTM> eventY;                                   // Event at y_1, ..., y_k
    Event eventX;                                                     // Event at x_1, ..., x_k-1
    const auto StateProposal{[&](const State& state0, State& state) { // T(x, y) (symmetric)
        std::ranges::transform(state0, state.begin(), [&](auto u0) {
            auto u{CLHEP::RandGaussQ::shoot(&rng, u0, delta)};
            u = std::abs(muc::fmod(u, 2.)); // Reflection-
            return u > 1 ? 2 - u : u;       // boundary
        });
    }};
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
            StateProposal(fMarkovChain.state, stateY[i]);     // Draw y_i from T(x, *)
            eventY[i] = fGENBOD({fCMSEnergy, {}}, stateY[i]); // y_i -> event(y_i) = g(y_i)
            if (not IRSafe(eventY[i].p)) {
                piY[i] = 0;
                continue;
            }
            biasY[i] = ValidBias(eventY[i].p); // g(y_i) -> B(g(y_i))
            if (biasY[i] <= std::numeric_limits<double>::min()) {
                piY[i] = biasY[i];
                continue;
            }
            piY[i] = ValidBiasedPDF(eventY[i], biasY[i]); // g(y_i) -> pi(y_i) = |M|²(g(y_i)) B(g(y_i))
        }
        const auto sumPiY{muc::ranges::reduce(piY)};         // pi(y_1) + ... + pi(y_k)
        const auto selected{MultinomialSample(piY, sumPiY)}; // Select Y from y_1, ..., y_k by pi(y_1), ..., pi(y_k)
        for (int i{}; i < kMTM - 1; ++i) {
            StateProposal(stateY[selected], stateX);    // Draw x_i from T(Y, *)
            eventX = fGENBOD({fCMSEnergy, {}}, stateX); // x_i -> event(x_i) = g(x_i)
            if (not IRSafe(eventX.p)) {
                piX[i] = 0;
                continue;
            }
            const auto biasX{ValidBias(eventX.p)}; // g(x_i) -> B(g(x_i))
            if (biasX <= std::numeric_limits<double>::min()) {
                piX[i] = biasX;
            }
            piX[i] = ValidBiasedPDF(eventX, biasX); // g(x_i) -> pi(x_i) = |M|²(g(x_i)) B(g(x_i))
        }
        const auto sumPiX{muc::ranges::reduce(piX, fMarkovChain.acceptance)}; // pi(x_1) + ... + pi(x_k)
        // accept/reject Y
        if (sumPiY >= sumPiX or
            sumPiY >= sumPiX * rng.flat()) {
            fMarkovChain.state = stateY[selected];
            fEvent = eventY[selected];
            fEvent.weight = 1 / biasY[selected];
            fMarkovChain.acceptance = piY[selected];
            return;
        }
    }
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisCore<M, N, A>::IRSafe(const FinalStateMomenta& momenta) const -> bool {
    for (int i{}; i < N; ++i) {
        if (momenta[i].e() <= fIRCut[i]) {
            return false;
        }
    }
    return true;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisCore<M, N, A>::ValidBias(const FinalStateMomenta& momenta) const -> double {
    const auto bias{fBias(momenta)};
    constexpr auto Format{[](const FinalStateMomenta& momenta) {
        std::string where;
        for (auto&& p : momenta) {
            where += fmt::format("[{}; {}, {}, {}]", p.e(), p.x(), p.y(), p.z());
        }
        return where;
    }};
    if (not std::isfinite(bias)) {
        Throw<std::runtime_error>(fmt::format("Infinite bias found (got {} at {})", bias, Format(momenta)));
    }
    if (bias < 0) {
        Throw<std::runtime_error>(fmt::format("Negative bias found (got {} at {})", bias, Format(momenta)));
    }
    return bias;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisCore<M, N, A>::ValidBiasedPDF(const Event& event, double bias) const -> double {
    const auto value{event.weight * fSquaredAmplitude({fCMSEnergy, {}}, event.p) * bias};
    const auto Where{[&] {
        auto where{fmt::format("({})", event.weight)};
        for (auto&& p : event.p) {
            where += fmt::format("[{}; {}, {}, {}]", p.e(), p.x(), p.y(), p.z());
        }
        where += fmt::format(" Bias={}", bias);
        return where;
    }};
    if (not std::isfinite(value)) {
        Throw<std::runtime_error>(fmt::format("Infinite biased PDF found (got {} at {})", value, Where()));
    }
    if (value < 0) {
        Throw<std::runtime_error>(fmt::format("Negative biased PDF found (got {} at {})", value, Where()));
    }
    return value;
}

} // namespace Mustard::inline Physics::internal
