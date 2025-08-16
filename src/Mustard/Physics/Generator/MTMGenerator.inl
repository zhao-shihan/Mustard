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

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
MTMGenerator<M, N, A>::MTMGenerator(double cmsE, const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                    double delta, int discard) :
    EventGenerator<M, N>{},
    fSquaredAmplitude{},
    fCMSEnergy{cmsE},
    fGENBOD{pdgID, mass},
    fBias{[](auto&&) { return 1; }},
    fMCMCDelta{},
    fMCMCDiscard{},
    fBurntIn{},
    fMarkovChain{},
    fEvent{} {
    MCMCDelta(delta);
    MCMCDiscard(discard);
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MTMGenerator<M, N, A>::MCMCDelta(double delta) -> void {
    if (delta <= 0 or 0.5 <= delta) [[unlikely]] {
        PrintError(fmt::format("Erroneous MCMC delta (got {}, expects 0 < delta < 0.5)", delta));
    }
    fMCMCDelta = delta;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MTMGenerator<M, N, A>::MCMCDiscard(int n) -> void {
    if (n < 0) [[unlikely]] {
        PrintWarning(fmt::format("Negative discarded MCMC samples (got {})", n));
    }
    fMCMCDiscard = n;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MTMGenerator<M, N, A>::Bias(BiasFunction B) -> void {
    fBias = std::move(B);
    BurnInRequired();
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MTMGenerator<M, N, A>::BurnIn(CLHEP::HepRandomEngine& rng) -> void {
    if (fBurntIn) {
        return;
    }
    // find phase space
    while (true) {
        std::ranges::generate(fMarkovChain.state, [&rng] { return rng.flat(); });
        fEvent = fGENBOD({fCMSEnergy, {}}, fMarkovChain.state);
        if (const auto bias{ValidBias(fEvent.p)};
            bias >= std::numeric_limits<double>::min()) {
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
auto MTMGenerator<M, N, A>::operator()(InitialStateMomenta pI, CLHEP::HepRandomEngine& rng) -> Event {
    CheckCMSEnergyUnchanged(pI);
    const auto beta{this->BoostToCMS(pI)};

    BurnIn(rng);
    for (int i{}; i < fMCMCDiscard; ++i) {
        NextEvent(fMCMCDelta, rng);
    }
    NextEvent(fMCMCDelta, rng);

    this->BoostToOriginalFrame(beta, fEvent.p);
    return fEvent;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MTMGenerator<M, N, A>::EstimateWeightNormalizationFactor(unsigned long long n) -> WeightNormalizationFactor {
    auto originalBias{std::move(fBias)};
    auto originalBurntIn{std::move(fBurntIn)};
    auto originalMarkovChain{std::move(fMarkovChain)};
    auto originalEvent{std::move(fEvent)};
    auto _{gsl::finally([&] {
        fBias = std::move(originalBias);
        fBurntIn = std::move(originalBurntIn);
        fMarkovChain = std::move(originalMarkovChain);
        fEvent = std::move(originalEvent);
    })};

    WeightNormalizationFactor result{.factor = std::numeric_limits<double>::quiet_NaN(),
                                     .error = std::numeric_limits<double>::quiet_NaN(),
                                     .nEff = 0};
    if (n == 0) {
        return result;
    }

    Bias([](auto&&) { return 1; }); // to evaluate the weight normalization factor of user-defined bias, temporarily switch to unbiased function
    auto& rng{*CLHEP::HepRandom::getTheEngine()};
    BurnIn(rng);

    using namespace Mustard::VectorArithmeticOperator::Vector2ArithmeticOperator;
    muc::array2ld sum{};
    const auto nProc{mplr::comm_world()};
    { // Monte Carlo integration here
        Parallel::ReseedRandomEngine(&rng);
        Executor<unsigned long long> executor{"Estimation", "Sample"};
        muc::array2ld partialSum{}; // improve numeric stability
        const auto partialSumThreshold{muc::llround(std::sqrt(n / executor.NProcess()))};
        executor.Execute(n, [&](auto i) {
            (*this)(rng);
            const auto bias{originalBias(fEvent.p)};
            partialSum += muc::array2ld{bias, muc::pow<2>(bias)};
            if ((i + 1) % partialSumThreshold == 0) {
                sum += partialSum;
                partialSum = {};
            }
        });
        sum += partialSum;
    }
    if (mplr::available()) {
        mplr::comm_world().allreduce([](auto a, auto b) { return a + b; }, sum);
    }
    result.factor = gsl::narrow_cast<double>(sum[0] / n);
    result.error = gsl::narrow_cast<double>(std::sqrt(sum[1]) / n);
    result.nEff = muc::pow<2>(result.factor / result.error);

    return result;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MTMGenerator<M, N, A>::CheckWeightNormalizationFactor(WeightNormalizationFactor wnf) -> bool {
    const auto [result, error, nEff]{wnf};
    const auto ok{nEff >= 10000};
    MasterPrintLn("Weight normalization factor of user-defined bias:\n"
                  "  {} +/- {}\n"
                  "    rel. err. = {:.2}% ,  N_eff = {:.2f} {}\n",
                  result, error, error / result * 100, nEff, ok ? "(OK)" : "(**INACCURATE**)");
    if (not ok) {
        MasterPrintWarning("N_eff TOO LOW. "
                           "This generally means there are a few highly weighted events "
                           "and THEY CAN BIAS THE ESTIMATIONS. "
                           "The estimation should be considered inaccurate. "
                           "Try increasing statistics.");
    }
    return ok;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MTMGenerator<M, N, A>::CMSEnergy(double cmsE) -> void {
    if (not muc::isclose(cmsE, fCMSEnergy)) {
        BurnInRequired();
    }
    fCMSEnergy = cmsE;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MTMGenerator<M, N, A>::Mass(const std::array<double, N>& mass) -> void {
    if (not std::ranges::equal(mass, fGENBOD.Mass(),
                               [](auto a, auto b) { return muc::isclose(a, b); })) {
        BurnInRequired();
    }
    fGENBOD.Mass(mass);
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MTMGenerator<M, N, A>::CheckCMSEnergyUnchanged(const InitialStateMomenta& pI) const -> void {
    if (pI == InitialStateMomenta{}) {
        return;
    }
    const auto cmsE{this->CalculateCMSEnergy(pI)};
    if (not muc::isclose(cmsE, fCMSEnergy)) [[unlikely]] {
        PrintWarning(fmt::format("Initial state 4-momenta does not match currently set CMS energy (got {}, expect {})", cmsE, fCMSEnergy));
    }
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MTMGenerator<M, N, A>::NextEvent(double delta, CLHEP::HepRandomEngine& rng) -> void {
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
            biasY[i] = ValidBias(eventY[i].p);                // g(y_i) -> B(g(y_i))
            if (biasY[i] > std::numeric_limits<double>::min()) {
                piY[i] = ValidBiasedPDF(eventY[i], biasY[i]); // g(y_i) -> pi(y_i) = |M|²(g(y_i)) B(g(y_i))
            } else {
                piY[i] = biasY[i];
            }
        }
        const auto sumPiY{muc::ranges::reduce(piY)};         // pi(y_1) + ... + pi(y_k)
        const auto selected{MultinomialSample(piY, sumPiY)}; // Select Y from y_1, ..., y_k by pi(y_1), ..., pi(y_k)
        for (int i{}; i < kMTM - 1; ++i) {
            StateProposal(stateY[selected], stateX);    // Draw x_i from T(Y, *)
            eventX = fGENBOD({fCMSEnergy, {}}, stateX); // x_i -> event(x_i) = g(x_i)
            const auto biasX{ValidBias(eventX.p)};      // g(x_i) -> B(g(x_i))
            if (biasX > std::numeric_limits<double>::min()) {
                piX[i] = ValidBiasedPDF(eventX, biasX); // g(x_i) -> pi(x_i) = |M|²(g(x_i)) B(g(x_i))
            } else {
                piX[i] = biasX;
            }
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
auto MTMGenerator<M, N, A>::ValidBias(const FinalStateMomenta& momenta) const -> double {
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
auto MTMGenerator<M, N, A>::ValidBiasedPDF(const Event& event, double bias) const -> double {
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

} // namespace Mustard::inline Physics::inline Generator
