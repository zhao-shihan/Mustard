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
MultipleTryMetropolisGenerator<M, N, A>::MultipleTryMetropolisGenerator(double cmsE, const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                                                        double delta, int discard) :
    EventGenerator<M, N>{},
    fCMSEnergy{},
    fSquaredAmplitude{},
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

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
MultipleTryMetropolisGenerator<M, N, A>::MultipleTryMetropolisGenerator(double cmsE, CLHEP::Hep3Vector polarization,
                                                                        const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                                                        double delta, int discard) // clang-format off
    requires std::derived_from<A, PolarizedSquaredAmplitude<1, N>> : // clang-format on
    MultipleTryMetropolisGenerator{cmsE, pdgID, mass, delta, discard} {
    fSquaredAmplitude.InitialStatePolarization(polarization);
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
MultipleTryMetropolisGenerator<M, N, A>::MultipleTryMetropolisGenerator(double cmsE, const std::array<CLHEP::Hep3Vector, M>& polarization,
                                                                        const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                                                        double delta, int discard) // clang-format off
    requires std::derived_from<A, PolarizedSquaredAmplitude<M, N>> and (M > 1) : // clang-format on
    MultipleTryMetropolisGenerator{cmsE, pdgID, mass, delta, discard} {
    fSquaredAmplitude.InitialStatePolarization(polarization);
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::InitialStatePolarization() const -> CLHEP::Hep3Vector
    requires std::derived_from<A, PolarizedSquaredAmplitude<1, N>> {
    return fSquaredAmplitude.InitialStatePolarization();
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::InitialStatePolarization(int i) const -> CLHEP::Hep3Vector
    requires std::derived_from<A, PolarizedSquaredAmplitude<M, N>> and (M > 1) {
    return fSquaredAmplitude.InitialStatePolarization(i);
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::InitialStatePolarization() const -> const std::array<CLHEP::Hep3Vector, M>&
    requires std::derived_from<A, PolarizedSquaredAmplitude<M, N>> and (M > 1) {
    return fSquaredAmplitude.InitialStatePolarization();
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::InitialStatePolarization(CLHEP::Hep3Vector pol) -> void
    requires std::derived_from<A, PolarizedSquaredAmplitude<1, N>> {
    if (not pol.isNear(InitialStatePolarization(), muc::default_tolerance<double>)) {
        BurnInRequired();
    }
    fSquaredAmplitude.InitialStatePolarization(pol);
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::InitialStatePolarization(int i, CLHEP::Hep3Vector pol) -> void
    requires std::derived_from<A, PolarizedSquaredAmplitude<M, N>> and (M > 1) {
    if (not pol.isNear(InitialStatePolarization(i), muc::default_tolerance<double>)) {
        BurnInRequired();
    }
    fSquaredAmplitude.InitialStatePolarization(i, pol);
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::InitialStatePolarization(const std::array<CLHEP::Hep3Vector, M>& pol) -> void
    requires std::derived_from<A, PolarizedSquaredAmplitude<M, N>> and (M > 1) {
    if (not std::ranges::equal(pol, InitialStatePolarization(),
                               [](auto&& a, auto&& b) { return a.isNear(b, muc::default_tolerance<double>); })) {
        BurnInRequired();
    }
    fSquaredAmplitude.InitialStatePolarization(pol);
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::Bias(BiasFunction B) -> void {
    fBias = std::move(B);
    BurnInRequired();
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::MCMCDelta(double delta) -> void {
    if (delta <= 0 or 0.5 <= delta) [[unlikely]] {
        PrintWarning(fmt::format("Suspicious MCMC delta (got {}, expects 0 < delta < 0.5)", delta));
    }
    fMCMCDelta = delta;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::MCMCDiscard(int n) -> void {
    if (n < 0) [[unlikely]] {
        PrintWarning(fmt::format("Negative discarded MCMC samples (got {})", n));
    }
    fMCMCDiscard = n;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::BurnIn(CLHEP::HepRandomEngine& rng) -> void {
    if (fBurntIn) {
        return;
    }
    // find phase space
    while (true) {
        std::ranges::generate(fMarkovChain.state, [&rng] { return rng.flat(); });
        const auto event{PhaseSpace(fMarkovChain.state)};
        if (not IRSafe(event.p)) {
            continue;
        }
        if (const auto bias{ValidBias(event.p)};
            bias > std::numeric_limits<double>::min()) {
            const auto& [detJ, _, momenta]{event};
            fMarkovChain.biasedMSqDetJ = ValidBiasedMSqDetJ(momenta, bias, detJ);
            break;
        }
    }
    // burning in
    constexpr auto delta0{0.1};
    constexpr auto epsilon{muc::default_tolerance<double>};
    constexpr auto nBurnIn{10000. * fgMCMCDim}; // E(distance in d-dim space) ~ sqrt(d), E(random walk distance) ~ sqrt(n) => n ~ d
    const auto factor{std::pow(epsilon / delta0, 1 / nBurnIn)};
    for (auto delta{delta0}; delta > epsilon; delta *= factor) {
        NextEvent(rng, delta);
    }
    fBurntIn = true;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::operator()(CLHEP::HepRandomEngine& rng, InitialStateMomenta pI) -> Event {
    CheckCMSEnergyMatch(pI);
    const auto beta{this->BoostToCMS(pI)};

    BurnIn(rng);
    for (int i{}; i < fMCMCDiscard; ++i) {
        NextEvent(rng, fMCMCDelta);
    }
    auto event{NextEvent(rng, fMCMCDelta)};

    this->BoostToOriginalFrame(beta, event.p);
    return event;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::EstimateWeightNormalizationFactor(unsigned long long n) -> WeightNormalizationFactor {
    auto originalBias{std::move(fBias)};
    auto originalBurntIn{std::move(fBurntIn)};
    auto originalMarkovChain{std::move(fMarkovChain)};
    auto _{gsl::finally([&] {
        fBias = std::move(originalBias);
        fBurntIn = std::move(originalBurntIn);
        fMarkovChain = std::move(originalMarkovChain);
    })};

    WeightNormalizationFactor result{.value = std::numeric_limits<double>::quiet_NaN(),
                                     .error = std::numeric_limits<double>::quiet_NaN(),
                                     .nEff = 0};
    if (n == 0) {
        return result;
    }

    Bias([](auto&&) { return 1; }); // to calculate the mean of user-defined bias, sample from unbiased |M|²
    auto& rng{*CLHEP::HepRandom::getTheEngine()};
    BurnIn(rng);

    using namespace Mustard::VectorArithmeticOperator::Vector2ArithmeticOperator;
    muc::array2d sum{};
    { // Monte Carlo integration here
        muc::array2d compensation{};
        const auto KahanAdd{[&](muc::array2d value) { // improve numeric stability
            const auto correctedValue{value - compensation};
            const auto newSum{sum + correctedValue};
            compensation = (newSum - sum) - correctedValue;
            sum = newSum;
        }};
        Parallel::ReseedRandomEngine(&rng);
        Executor<unsigned long long>{"Integration", "Sample"}(n, [&](auto) {
            const auto event{NextEvent(rng)};
            const auto bias{originalBias(event.p)};
            KahanAdd({bias, muc::pow(bias, 2)});
        });
    }
    if (mplr::available()) {
        mplr::comm_world().allreduce([](auto a, auto b) { return a + b; }, sum);
    }
    const auto& [sumBias, sumBiasSq]{sum};
    result.value = sumBias / n;
    result.error = std::sqrt(sumBiasSq / n - muc::pow(result.value, 2)) / n;
    result.nEff = muc::pow(result.value / result.error, 2);

    return result;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::CheckWeightNormalizationFactor(WeightNormalizationFactor wnf) -> bool {
    const auto [result, error, nEff]{wnf};
    const auto ok{nEff >= 10000};
    MasterPrintLn("Weight normalization factor from user-defined bias:\n"
                  "  {} +/- {}\n"
                  "    rel. err. = {:.2}% ,  N_eff = {:.2f} {}\n",
                  result, error, error / result * 100, nEff, ok ? "(OK)" : "(**INACCURATE**)");
    if (not ok) {
        MasterPrintWarning("N_eff TOO LOW. "
                           "The estimation should be considered inaccurate. "
                           "Try increasing statistics.");
    }
    return ok;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::CMSEnergy(double cmsE) -> void {
    if (cmsE <= 0) [[unlikely]] {
        PrintError(fmt::format("Non-positive CMS energy (got {})", cmsE));
    }
    if (not muc::isclose(cmsE, fCMSEnergy)) {
        BurnInRequired();
    }
    fCMSEnergy = cmsE;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::Mass(const std::array<double, N>& mass) -> void {
    if (not std::ranges::equal(mass, fGENBOD.Mass(),
                               [](auto a, auto b) { return muc::isclose(a, b); })) {
        BurnInRequired();
    }
    fGENBOD.Mass(mass);
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::IRCut(int i, double cut) -> void {
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

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::CheckCMSEnergyMatch(const InitialStateMomenta& pI) const -> void {
    if (pI == InitialStateMomenta{}) {
        return;
    }
    const auto cmsE{this->CalculateCMSEnergy(pI)};
    if (not muc::isclose(cmsE, fCMSEnergy)) [[unlikely]] {
        PrintWarning(fmt::format("Initial state 4-momenta does not match currently set CMS energy (got {}, expect {})", cmsE, fCMSEnergy));
    }
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::NextEvent(CLHEP::HepRandomEngine& rng, double delta) -> Event {
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
            const auto u{CLHEP::RandGaussQ::shoot(&rng, u0, delta)};
            return u - std::floor(u); // periodic boundary
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
            StateProposal(fMarkovChain.state, stateY[i]); // Draw y_i from T(x, *)
            eventY[i] = FairPhaseSpace(stateY[i], rng);   // y_i -> event(y_i) = g(y_i)
            const auto& [detJ, _, momenta]{eventY[i]};
            if (not IRSafe(momenta)) {
                piY[i] = 0;
                continue;
            }
            biasY[i] = ValidBias(momenta); // g(y_i) -> B(g(y_i))
            if (biasY[i] <= std::numeric_limits<double>::min()) {
                piY[i] = biasY[i];
                continue;
            }
            piY[i] = ValidBiasedMSqDetJ(momenta, biasY[i], detJ); // g(y_i) -> pi(y_i) = |M|²(g(y_i)) × B(g(y_i)) × |J|(g(y_i))
        }
        const auto sumPiY{muc::ranges::reduce(piY)};         // pi(y_1) + ... + pi(y_k)
        const auto selected{MultinomialSample(piY, sumPiY)}; // Select Y from y_1, ..., y_k by pi(y_1), ..., pi(y_k)
        for (int i{}; i < kMTM - 1; ++i) {
            StateProposal(stateY[selected], stateX); // Draw x_i from T(Y, *)
            eventX = FairPhaseSpace(stateX, rng);    // x_i -> event(x_i) = g(x_i)
            const auto& [detJ, _, momenta]{eventX};
            if (not IRSafe(momenta)) {
                piX[i] = 0;
                continue;
            }
            const auto biasX{ValidBias(momenta)}; // g(x_i) -> B(g(x_i))
            if (biasX <= std::numeric_limits<double>::min()) {
                piX[i] = biasX;
            }
            piX[i] = ValidBiasedMSqDetJ(momenta, biasX, detJ); // g(x_i) -> pi(x_i) = |M|²(g(x_i)) × B(g(x_i)) × |J|(g(y_i))
        }
        const auto sumPiX{muc::ranges::reduce(piX, fMarkovChain.biasedMSqDetJ)}; // pi(x_1) + ... + pi(x_k)
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

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::FairPhaseSpace(typename GENBOD<M, N>::RandomState u, CLHEP::HepRandomEngine& rng) -> Event {
    auto event{PhaseSpace(u)};
    if (fIdenticalSet.empty() or rng.flat() < 0.5) {
        return event;
    }
    const auto& idSet{fIdenticalSet[RandomIndex(rng, fIdenticalSet.size())]};
    if (idSet.size() == 2) {
        std::swap(event.p[idSet.front()], event.p[idSet.back()]);
    } else {
        const auto idA{RandomIndex(rng, idSet.size())};
        const auto idB{(idA + 1) % idSet.size()};
        std::swap(event.p[idSet[idA]], event.p[idSet[idB]]);
    }
    return event;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::IRSafe(const FinalStateMomenta& momenta) const -> bool {
    for (int i{}; i < N; ++i) {
        if (momenta[i].e() <= fIRCut[i]) {
            return false;
        }
    }
    return true;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::ValidBias(const FinalStateMomenta& momenta) const -> double {
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
auto MultipleTryMetropolisGenerator<M, N, A>::ValidBiasedMSqDetJ(const FinalStateMomenta& momenta, double bias, double detJ) const -> double {
    const auto value{fSquaredAmplitude({fCMSEnergy, {}}, momenta) * bias * detJ}; // |M|² × bias × |J|
    const auto Where{[&] {
        auto where{fmt::format("({})", detJ)};
        for (auto&& p : momenta) {
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

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MultipleTryMetropolisGenerator<M, N, A>::RandomIndex(CLHEP::HepRandomEngine& rng, int n) -> int {
    muc::assume(n > 0);
    if (n == 1) {
        return 0;
    }
    auto i{static_cast<int>(n * rng.flat())};
    while (i == n) [[unlikely]] {
        i = n * rng.flat();
    }
    return i;
}

} // namespace Mustard::inline Physics::inline Generator
