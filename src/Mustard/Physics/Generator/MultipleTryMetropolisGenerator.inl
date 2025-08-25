// -*- C++ -*-
//
// Copyright 2020-2025  The Mustard development team
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
MultipleTryMetropolisGenerator<M, N, A>::MultipleTryMetropolisGenerator(double cmsE, const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                                                        double delta, unsigned discard) :
    EventGenerator<M, N>{},
    fCMSEnergy{},
    fMatrixElement{},
    fIRCut{},
    fIdenticalSet{},
    fBias{[](auto&&) { return 1; }},
    fGENBOD{pdgID, mass},
    fMCMCDelta{},
    fMCMCDiscard{},
    fBurntIn{},
    fMarkovChain{} {
    CMSEnergy(cmsE);
    MCMCDelta(delta);
    MCMCDiscard(discard);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
MultipleTryMetropolisGenerator<M, N, A>::MultipleTryMetropolisGenerator(double cmsE, CLHEP::Hep3Vector polarization,
                                                                        const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                                                        double delta, unsigned discard) // clang-format off
    requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>> : // clang-format on
    MultipleTryMetropolisGenerator{cmsE, pdgID, mass, delta, discard} {
    fMatrixElement.InitialStatePolarization(polarization);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
MultipleTryMetropolisGenerator<M, N, A>::MultipleTryMetropolisGenerator(double cmsE, const std::array<CLHEP::Hep3Vector, M>& polarization,
                                                                        const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                                                        double delta, unsigned discard) // clang-format off
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) : // clang-format on
    MultipleTryMetropolisGenerator{cmsE, pdgID, mass, delta, discard} {
    fMatrixElement.InitialStatePolarization(polarization);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
MultipleTryMetropolisGenerator<M, N, A>::~MultipleTryMetropolisGenerator() = default;

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::InitialStatePolarization() const -> CLHEP::Hep3Vector
    requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>> {
    return fMatrixElement.InitialStatePolarization();
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::InitialStatePolarization(int i) const -> CLHEP::Hep3Vector
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) {
    return fMatrixElement.InitialStatePolarization(i);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::InitialStatePolarization() const -> const std::array<CLHEP::Hep3Vector, M>&
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) {
    return fMatrixElement.InitialStatePolarization();
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::InitialStatePolarization(CLHEP::Hep3Vector pol) -> void
    requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>> {
    if (not pol.isNear(InitialStatePolarization(), muc::default_tolerance<double>)) {
        BurnInRequired();
    }
    fMatrixElement.InitialStatePolarization(pol);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::InitialStatePolarization(int i, CLHEP::Hep3Vector pol) -> void
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) {
    if (not pol.isNear(InitialStatePolarization(i), muc::default_tolerance<double>)) {
        BurnInRequired();
    }
    fMatrixElement.InitialStatePolarization(i, pol);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::InitialStatePolarization(const std::array<CLHEP::Hep3Vector, M>& pol) -> void
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) {
    if (not std::ranges::equal(pol, InitialStatePolarization(),
                               [](auto&& a, auto&& b) { return a.isNear(b, muc::default_tolerance<double>); })) {
        BurnInRequired();
    }
    fMatrixElement.InitialStatePolarization(pol);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::Bias(BiasFunction B) -> void {
    fBias = std::move(B);
    BurnInRequired();
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::MCMCDelta(double delta) -> void {
    if (delta <= 0 or 0.5 <= delta) [[unlikely]] {
        PrintWarning(fmt::format("Suspicious MCMC delta (got {}, expects 0 < delta < 0.5)", delta));
    }
    fMCMCDelta = delta;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::MCMCDiscard(unsigned n) -> void {
    if (n >= std::numeric_limits<int>::max()) [[unlikely]] {
        PrintWarning(fmt::format("Suspicious number of discarded MCMC samples (got {})", n));
    }
    fMCMCDiscard = n;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::BurnIn(CLHEP::HepRandomEngine& rng) -> void {
    const auto thisName{muc::try_demangle(typeid(*this).name())};
    if (fBurntIn) {
        MasterPrintLn("Markov chain in {} already burnt in.", thisName);
        return;
    }
    MasterPrintLn("Markov chain of {} burning in, please wait...", thisName);
    muc::chrono::stopwatch stopwatch;

    // find phase space
    muc::ranges::iota(fMarkovChain.state.pID, 0);
    while (true) {
        std::ranges::generate(fMarkovChain.state.u, [&rng] { return rng.flat(); });
        const auto event{DirectPhaseSpace(fMarkovChain.state.u)};
        if (not IRSafe(event.p)) {
            continue;
        }
        if (const auto bias{ValidBias(event.p)};
            bias > std::numeric_limits<double>::epsilon()) {
            const auto& [detJ, _, pF]{event};
            fMarkovChain.biasedMSqDetJ = ValidBiasedMSqDetJ(pF, bias, detJ);
            break;
        }
    }
    // burning in
    constexpr auto delta0{0.1};
    constexpr auto epsilon{muc::default_tolerance<double>};
    // E(distance in d-dim space) ~ sqrt(d), and E(random walk displacement) ~ sqrt(random walk distance),
    // nBurnIn should satisfy E(random walk displacement) ~ scale * E(distance in d-dim space),
    // so we solve nBurnIn from sqrt(random walk distance) = scale * sqrt(dimension) with some scale
    constexpr auto travelScale{10};
    const auto nBurnIn{std::log(epsilon / delta0) / std::log(1 - delta0 / (muc::pow(travelScale, 2) * fgMCMCDim))};
    const auto factor{std::pow(epsilon / delta0, 1 / nBurnIn)};
    for (auto delta{delta0}; delta > epsilon; delta *= factor) {
        NextEvent(rng, delta);
    }
    fBurntIn = true;

    auto time{muc::chrono::seconds<double>{stopwatch.read()}.count()};
    if (mplr::available()) {
        mplr::comm_world().ireduce(mplr::max<double>{}, 0, time).wait(mplr::duty_ratio::preset::relaxed);
    }
    MasterPrintLn("Markov chain burnt in in {:.2f}s.", time);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::operator()(CLHEP::HepRandomEngine& rng, InitialStateMomenta pI) -> Event {
    CheckCMSEnergyMatch(pI);
    const auto beta{this->BoostToCMS(pI)};

    if (not fBurntIn) [[unlikely]] {
        PrintWarning("Markov chain not burnt in. Burning it in");
        BurnIn(rng);
    }
    for (unsigned i{}; i < fMCMCDiscard; ++i) {
        NextEvent(rng, fMCMCDelta);
    }
    auto event{NextEvent(rng, fMCMCDelta)};

    this->BoostToOriginalFrame(beta, event.p);
    return event;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::EstimateNormalizationFactor(Executor<unsigned long long>& executor, double precisionGoal,
                                                                          std::array<IntegrationState, 2> integrationState,
                                                                          CLHEP::HepRandomEngine& rng) -> std::pair<IntegrationResult, std::array<IntegrationState, 2>> {
    // Monte Carlo integration method
    const auto Integrate{[&](std::regular_invocable<const Event&> auto&& Integrand, IntegrationState& state) {
        // One integration iteration
        const auto Iteration{[&](unsigned long long nSample) {
            using namespace Mustard::VectorArithmeticOperator::Vector2ArithmeticOperator;
            muc::array2ld sum{};
            muc::array2ld compensation{};
            const auto KahanAdd{[&](muc::array2ld value) { // improve numeric stability
                const auto correctedValue{value - compensation};
                const auto newSum{sum + correctedValue};
                compensation = (newSum - sum) - correctedValue;
                sum = newSum;
            }};
            executor(nSample, [&](auto) {
                const auto event{fGENBOD(rng, {fCMSEnergy, {}})};
                if (not IRSafe(event.p)) {
                    return;
                }
                const long double value{Integrand(event)};
                KahanAdd({value, muc::pow(value, 2)});
            });
            if (mplr::available()) {
                mplr::comm_world().allreduce([](auto a, auto b) { return a + b; }, sum);
            }
            state.sum += sum;
            state.n += nSample;
            IntegrationResult result;
            result.value = state.sum[0] / state.n;
            result.uncertainty = std::sqrt((state.sum[1] / state.n - muc::pow(result.value, 2)) / state.n);
            return result;
        }};
        // Iteration loop
        auto nSample{static_cast<unsigned long long>(10 * muc::pow(precisionGoal, -2))};
        while (true) {
            if (state.n == 0) {
                MasterPrintLn("Restarting integration.");
            } else {
                MasterPrintLn("Continuing integration from state {} {} {}.", state.sum[0], state.sum[1], state.n);
            }
            MasterPrintLn("Integrate with {} samples. Precision goal: {:.3}.", nSample, precisionGoal);
            const auto result{Iteration(nSample)};
            const auto relativeUncertainty{result.uncertainty / result.value};
            if (relativeUncertainty < precisionGoal) {
                MasterPrintLn("Current precision: {:.3}, precision goal reached.", relativeUncertainty);
                return result;
            }
            MasterPrintLn("Current precision: {:.3}, precision goal not reached.", relativeUncertainty);
            // Increase sample size adaptively
            const auto factor{1.2 * muc::pow(relativeUncertainty / precisionGoal, 2) - 1};
            if (std::isfinite(factor)) {
                nSample = factor * state.n;
            } else {
                nSample *= 10;
            }
            nSample = std::max<unsigned long long>(executor.NProcess(), nSample);
        }
    }};

    MasterPrintLn("Estimating normalization factor in {}.", muc::try_demangle(typeid(*this).name()));

    // Set task name
    auto originalExecutionName{executor.ExecutionName()};
    auto originalTaskName{executor.TaskName()};
    auto _{gsl::finally([&] {
        executor.ExecutionName(std::move(originalExecutionName));
        executor.TaskName(std::move(originalTaskName));
    })};
    executor.ExecutionName("Integration");
    executor.TaskName("Sample");

    // Seeding random engine
    Parallel::ReseedRandomEngine(&rng);

    // Start integration
    muc::chrono::stopwatch stopwatch;

    // Compute denominator
    MasterPrintLn("Computing denominator integral.");
    const auto DenomIntegrand{[this](const Event& event) {
        const auto& [detJ, _, pF]{event};
        return ValidBiasedMSqDetJ(pF, 1, detJ);
    }};
    const auto denom{Integrate(DenomIntegrand, integrationState[1])};
    MasterPrintLn("Denominator integration completed.");

    // Compute numerator
    MasterPrintLn("Computing numerator integral.");
    const auto NumerIntegrand{[this](const Event& event) {
        const auto& [detJ, _, pF]{event};
        const auto bias{ValidBias(pF)};
        return ValidBiasedMSqDetJ(pF, bias, detJ);
    }};
    const auto numer{Integrate(NumerIntegrand, integrationState[0])};
    MasterPrintLn("Numerator integration completed.");

    // Combine result
    IntegrationResult result;
    result.value = numer.value / denom.value;
    result.uncertainty = std::hypot(denom.value * numer.uncertainty, numer.value * denom.uncertainty) / muc::pow(denom.value, 2);

    // Report result
    auto time{muc::chrono::seconds<double>{stopwatch.read()}.count()};
    if (mplr::available()) {
        mplr::comm_world().ireduce(mplr::max<double>{}, 0, time).wait(mplr::duty_ratio::preset::relaxed);
    }
    const auto& [s1, s2]{integrationState};
    MasterPrint("Estimation completed in {:.2f}s.\n"
                "Integration state (integration can be contined from here):\n"
                "  {} {} {} {} {} {}\n"
                "Normalization factor from user-defined bias:\n"
                "  {} +/- {}  (rel. unc.: {:.2}%)\n",
                time, s1.sum[0], s1.sum[1], s1.n, s2.sum[0], s2.sum[1], s2.n,
                result.value, result.uncertainty,
                result.uncertainty / result.value * 100);
    return {result, integrationState};
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::CMSEnergy(double cmsE) -> void {
    if (cmsE <= 0) [[unlikely]] {
        PrintError(fmt::format("Non-positive CMS energy (got {})", cmsE));
    }
    if (not muc::isclose(cmsE, fCMSEnergy)) {
        BurnInRequired();
    }
    fCMSEnergy = cmsE;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::Mass(const std::array<double, N>& mass) -> void {
    if (not std::ranges::equal(mass, fGENBOD.Mass(),
                               [](auto a, auto b) { return muc::isclose(a, b); })) {
        BurnInRequired();
    }
    fGENBOD.Mass(mass);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::IRCut(int i, double cut) -> void {
    if (cut <= 0) [[unlikely]] {
        PrintWarning(fmt::format("Non-positive IR cut for particle {} (got {})", i, cut));
    }
    if (muc::pow(fGENBOD.Mass(i) / fCMSEnergy, 2) > muc::default_tolerance<double>) [[unlikely]] {
        PrintWarning(fmt::format("IR cut set for massive particle {} (mass = {})", i, fGENBOD.Mass(i)));
    }
    BurnInRequired();
    fIRCut.push_back({i, cut});
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::AddIdenticalSet(std::vector<int> set) -> void {
    if (set.size() < 2) [[unlikely]] {
        PrintWarning(fmt::format("Identical set should have at least 2 elements (got {}), ignoring it", set.size()));
        return;
    }
    muc::timsort(set);
    if (const auto duplicate{std::ranges::unique(set)};
        duplicate.size() != 0) [[unlikely]] {
        PrintWarning(fmt::format("There are {} duplicate index in identical set, removing them", duplicate.size()));
        set.erase(duplicate.begin(), duplicate.end());
    }
    for (auto&& addedSet : std::as_const(fIdenticalSet)) {
        const auto duplicated{std::ranges::find_first_of(addedSet, set)};
        if (duplicated != addedSet.cend()) [[unlikely]] {
            PrintError(fmt::format("Particle {} added accross different identical sets", *duplicated));
        }
    }
    fIdenticalSet.emplace_back(std::move(set));
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::CheckCMSEnergyMatch(const InitialStateMomenta& pI) const -> void {
    if (pI == InitialStateMomenta{}) {
        return;
    }
    const auto cmsE{this->CalculateCMSEnergy(pI)};
    if (not muc::isclose(cmsE, fCMSEnergy)) [[unlikely]] {
        PrintWarning(fmt::format("Initial state 4-momenta does not match currently set CMS energy (got {}, expect {})", cmsE, fCMSEnergy));
    }
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::NextEvent(CLHEP::HepRandomEngine& rng, double delta) -> Event {
    // Rescale delta first
    // E(distance in d-dim space) ~ sqrt(d), if delta = delta0 / sqrt(d) => E(step size) ~ delta0
    delta /= std::sqrt(fgMCMCDim);
    // Multiple-try Metropolis sampler (Ref: Jun S. Liu et al (2000), https://doi.org/10.2307/2669532)
    constexpr auto kMTM{fgMCMCDim};                     // k
    std::array<struct MarkovChain::State, kMTM> stateY; // y_1, ..., y_k
    std::array<Event, kMTM> eventY;                     // Event at y_1, ..., y_k
    std::array<double, kMTM> biasY;                     // Bias function value at y_1, ..., y_k
    std::array<double, kMTM> piY;                       // pi(y_1), ..., pi(y_k)
    struct MarkovChain::State stateX;                   // x_i
    const auto RandomIndex([&rng](int n) {
        muc::assume(n >= 2);
        auto i{static_cast<int>(n * rng.flat())};
        while (i == n) [[unlikely]] {
            i = n * rng.flat();
        }
        return i;
    });
    // symmetric T(x, y)
    const auto StateProposal{[&](const MarkovChain::State& state0, MarkovChain::State& state) {
        // Walk random state
        std::ranges::transform(state0.u, state.u.begin(), [&](auto u0) {
            const auto u{CLHEP::RandGaussQ::shoot(&rng, u0, delta)};
            return u - std::floor(u); // periodic boundary
        });
        // Walk particle mapping if there are identical particles
        state.pID = state0.pID;
        if (fIdenticalSet.empty() or rng.flat() < 0.5) {
            return;
        }
        const auto& idSet{fIdenticalSet.size() == 1 ?
                              fIdenticalSet.front() :
                              fIdenticalSet[RandomIndex(fIdenticalSet.size())]};
        if (idSet.size() == 2) {
            std::swap(state.pID[idSet[0]], state.pID[idSet[1]]);
        } else {
            const auto idA{RandomIndex(idSet.size())};
            const auto idB{(idA + 1) % idSet.size()};
            std::swap(state.pID[idSet[idA]], state.pID[idSet[idB]]);
        }
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
            StateProposal(fMarkovChain.state, stateY[i]); // Draw y_i from T(x, *)
            eventY[i] = PhaseSpace(stateY[i]);            // y_i -> event(y_i) = g(y_i)
            const auto& [detJ, _, pF]{eventY[i]};
            if (not IRSafe(pF)) {
                piY[i] = 0;
                continue;
            }
            biasY[i] = ValidBias(pF);                        // g(y_i) -> B(g(y_i))
            piY[i] = ValidBiasedMSqDetJ(pF, biasY[i], detJ); // g(y_i) -> pi(y_i) = |M|²(g(y_i)) × B(g(y_i)) × |J|(g(y_i))
        }
        const auto sumPiY{muc::ranges::reduce(piY)};         // pi(y_1) + ... + pi(y_k)
        const auto selected{MultinomialSample(piY, sumPiY)}; // Select Y from y_1, ..., y_k by pi(y_1), ..., pi(y_k)

        auto sumPiX{fMarkovChain.biasedMSqDetJ}; // pi(x_1) + ... + pi(x_k)
        for (int i{}; i < kMTM - 1; ++i) {
            StateProposal(stateY[selected], stateX);      // Draw x_i from T(Y, *)
            const auto [detJ, _, pF]{PhaseSpace(stateX)}; // x_i -> event(x_i) = g(x_i)
            if (not IRSafe(pF)) {
                continue;
            }
            const auto biasX{ValidBias(pF)};               // g(x_i) -> B(g(x_i))
            sumPiX += ValidBiasedMSqDetJ(pF, biasX, detJ); // g(x_i) -> pi(x_i) = |M|²(g(x_i)) × B(g(x_i)) × |J|(g(y_i))
        }

        // accept/reject Y
        if (sumPiY >= sumPiX or
            sumPiY >= sumPiX * rng.flat()) {
            fMarkovChain.state = stateY[selected];
            fMarkovChain.biasedMSqDetJ = piY[selected];
            eventY[selected].weight = 1 / biasY[selected];
            return eventY[selected];
        }
    }
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::PhaseSpace(const MarkovChain::State& state) -> Event {
    auto event{DirectPhaseSpace(state.u)};
    event.p = std::apply([&](auto... i) { return FinalStateMomenta{event.p[i]...}; }, state.pID);
    return event;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::IRSafe(const FinalStateMomenta& pF) const -> bool {
    for (auto&& [i, cut] : fIRCut) {
        if (pF[i].e() <= cut) {
            return false;
        }
    }
    return true;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::ValidBias(const FinalStateMomenta& pF) const -> double {
    const auto bias{fBias(pF)};
    constexpr auto Format{[](const FinalStateMomenta& pF) {
        std::string where;
        for (auto&& p : pF) {
            where += fmt::format("[{}; {}, {}, {}]", p.e(), p.x(), p.y(), p.z());
        }
        return where;
    }};
    if (not std::isfinite(bias)) {
        Throw<std::runtime_error>(fmt::format("Infinite bias found (got {} at {})", bias, Format(pF)));
    }
    if (bias < 0) {
        Throw<std::runtime_error>(fmt::format("Negative bias found (got {} at {})", bias, Format(pF)));
    }
    return bias;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::ValidBiasedMSqDetJ(const FinalStateMomenta& pF, double bias, double detJ) const -> double {
    const auto value{fMatrixElement({fCMSEnergy, {}}, pF) * bias * detJ}; // |M|² × bias × |J|
    const auto Where{[&] {
        auto where{fmt::format("({})", detJ)};
        for (auto&& p : pF) {
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
