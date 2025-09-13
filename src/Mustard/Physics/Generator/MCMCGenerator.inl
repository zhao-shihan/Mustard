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
MCMCGenerator<M, N, A>::MCMCGenerator(const InitialStateMomenta& pI, const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                      std::optional<double> thinningRatio, std::optional<unsigned> acfSampleSize) :
    Base{pI, pdgID, mass},
    fIdenticalSet{},
    fThinningRatio{1.5},
    fACFSampleSize{fgDefaultInvalidACFSampleSize},
    fMCMCInitialized{},
    fThinningSize{},
    fMC{} {
    if (thinningRatio) {
        ThinningRatio(*thinningRatio);
    }
    if (acfSampleSize) {
        ACFSampleSize(*acfSampleSize);
    }
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
MCMCGenerator<M, N, A>::MCMCGenerator(const InitialStateMomenta& pI, CLHEP::Hep3Vector polarization,
                                      const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                      std::optional<double> thinningRatio, std::optional<unsigned> acfSampleSize) // clang-format off
    requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>> : // clang-format on
    MCMCGenerator{pI, pdgID, mass, std::move(thinningRatio), std::move(acfSampleSize)} {
    this->InitialStatePolarization(polarization);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
MCMCGenerator<M, N, A>::MCMCGenerator(const InitialStateMomenta& pI, const std::array<CLHEP::Hep3Vector, M>& polarization,
                                      const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                      std::optional<double> thinningRatio, std::optional<unsigned> acfSampleSize) // clang-format off
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) : // clang-format on
    MCMCGenerator{pI, pdgID, mass, std::move(thinningRatio), std::move(acfSampleSize)} {
    this->InitialStatePolarization(polarization);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::InitialStatePolarization() const -> CLHEP::Hep3Vector
    requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>> {
    return Base::InitialStatePolarization();
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::InitialStatePolarization(int i) const -> CLHEP::Hep3Vector
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) {
    return Base::InitialStatePolarization(i);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::InitialStatePolarization() const -> const std::array<CLHEP::Hep3Vector, M>&
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) {
    return Base::InitialStatePolarization();
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::InitialStatePolarization(CLHEP::Hep3Vector pol) -> void
    requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>> {
    if (not pol.isNear(InitialStatePolarization(), muc::default_tolerance<double>)) {
        MCMCInitializeRequired();
    }
    Base::InitialStatePolarization(pol);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::InitialStatePolarization(int i, CLHEP::Hep3Vector pol) -> void
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) {
    if (not pol.isNear(InitialStatePolarization(i), muc::default_tolerance<double>)) {
        MCMCInitializeRequired();
    }
    Base::InitialStatePolarization(i, pol);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::InitialStatePolarization(const std::array<CLHEP::Hep3Vector, M>& pol) -> void
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) {
    if (not std::ranges::equal(pol, InitialStatePolarization(),
                               [](auto&& a, auto&& b) { return a.isNear(b, muc::default_tolerance<double>); })) {
        MCMCInitializeRequired();
    }
    Base::InitialStatePolarization(pol);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::Acceptance(AcceptanceFunction Acceptance) -> void {
    Base::Acceptance(Acceptance);
    MCMCInitializeRequired();
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::ThinningRatio(double value) -> void {
    if (not std::isfinite(value)) [[unlikely]] {
        PrintError(fmt::format("Infinite thinning factor (got {})", value));
    }
    if (value < 0) [[unlikely]] {
        PrintWarning(fmt::format("Negative thinning factor (got {}), setting to 0", value));
        value = 0;
    }
    if (value > 10) [[unlikely]] {
        PrintWarning(fmt::format("Suspicious thinning factor (got {})", value));
    }
    fThinningRatio = value;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::ACFSampleSize(unsigned n) -> void {
    if (n >= std::numeric_limits<int>::max()) [[unlikely]] {
        PrintWarning(fmt::format("Suspicious ACF sample size (got {})", n));
    }
    fACFSampleSize = n;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::MCMCInitialize(CLHEP::HepRandomEngine& rng) -> AutocorrelationFunction {
    if (fACFSampleSize == fgDefaultInvalidACFSampleSize) {
        Throw<std::logic_error>("ACF sample size not set");
    }

    const auto thisName{muc::try_demangle(typeid(*this).name())};
    MasterPrintLn("Initializing {}...", thisName);
    muc::chrono::stopwatch stopwatch;

    // Reseed random engine for statistical safety
    Parallel::ReseedRandomEngine(&rng);

    // find phase space
    MasterPrintLn("Finding phase space...");
    muc::ranges::iota(fMC.state.pID, 0);
    while (true) {
        rng.flatArray(fMC.state.u.size(), fMC.state.u.data());
        auto [event, detJ]{DirectPhaseSpace(fMC.state.u)};
        if (not this->IRSafe(event.p)) {
            continue;
        }
        const auto acceptance{this->ValidAcceptance(event.p)};
        if (acceptance <= std::numeric_limits<double>::min()) {
            continue;
        }
        const auto mSqAcceptanceDetJ{this->ValidMSqAcceptanceDetJ(event.p, acceptance, detJ)};
        if (mSqAcceptanceDetJ <= std::numeric_limits<double>::min()) {
            continue;
        }
        fMC.mSqAcceptanceDetJ = mSqAcceptanceDetJ;
        fMC.event = std::move(event);
        fMC.event.weight = 1 / acceptance;
        break;
    }
    MasterPrintLn("Phase space found.");

    // Burning in
    MasterPrintLn("Markov chain burning in...");
    BurnIn(rng);
    MasterPrintLn("Markov chain burnt in.");

    if (Env::VerboseLevelReach<'I'>()) {
        struct {
            Eigen::Vector<double, MarkovChain::dim> u;
            Eigen::Matrix<double, MarkovChain::dim, MarkovChain::dim> uuT;
        } mean;
        mean.u.setZero();
        mean.uuT.setZero();
        double sumMSqAcceptanceDetJ{};
        for (unsigned i{}; i < 100 * fACFSampleSize; ++i) {
            Eigen::Vector<double, MarkovChain::dim> u;
            rng.flatArray(u.size(), u.data());
            auto [event, detJ]{DirectPhaseSpace(VectorCast<RandomState>(u))};
            if (not this->IRSafe(event.p)) {
                continue;
            }
            const auto acceptance{this->ValidAcceptance(event.p)};
            const auto mSqAcceptanceDetJ{this->ValidMSqAcceptanceDetJ(event.p, acceptance, detJ)};
            const auto uMSqAcceptanceDetJ{(mSqAcceptanceDetJ * u).eval()};
            mean.u += uMSqAcceptanceDetJ;
            mean.uuT += uMSqAcceptanceDetJ * u.transpose();
            sumMSqAcceptanceDetJ += mSqAcceptanceDetJ;
        }
        mean.u /= sumMSqAcceptanceDetJ;
        mean.uuT /= sumMSqAcceptanceDetJ;
        const auto covariance{(mean.uuT - mean.u * mean.u.transpose()).eval()};
        const auto sqrtDiagCov{Eigen::SelfAdjointEigenSolver<decltype(covariance)>{covariance}.eigenvalues().array().sqrt()};
        PrintInfo(fmt::format("Sqrt(diag(covariance)): {}", sqrtDiagCov));
    }

    // Estimate autocorrelation and decide thinning
    MasterPrintLn("Estimating autocorrelation and decide thinning...");
    using ArrayDimMC = Eigen::Array<double, MarkovChain::dim, 1>;

    std::vector<ArrayDimMC> sample(fACFSampleSize);
    for (unsigned i{}; i < fACFSampleSize; ++i) {
        NextEvent(rng);
        std::ranges::copy(fMC.state.u, sample[i].begin());
    }
    ArrayDimMC sampleMean;
    sampleMean.setZero();
    for (auto&& x : sample) {
        sampleMean += x;
    }
    sampleMean /= fACFSampleSize;
    ArrayDimMC autocorrelationDenominator;
    autocorrelationDenominator.setZero();
    for (auto&& x : sample) {
        const auto delta{(x - sampleMean).eval()};
        autocorrelationDenominator += delta * delta;
    }

    const auto maxLag{fACFSampleSize / 2};
    const auto deltaLag{std::max(1u, maxLag / 1000)};
    AutocorrelationFunction autocorrelationFunction;
    autocorrelationFunction.reserve(maxLag / deltaLag + 2);
    autocorrelationFunction.emplace_back(0, ArrayDimMC::Ones());
    for (auto lag{deltaLag}; lag <= maxLag; lag += deltaLag) {
        ArrayDimMC autocorrelationNumerator;
        autocorrelationNumerator.setZero();
        for (unsigned i{}; i < fACFSampleSize - lag; ++i) {
            autocorrelationNumerator += (sample[i] - sampleMean) * (sample[i + lag] - sampleMean);
        }
        autocorrelationFunction.emplace_back(lag, autocorrelationNumerator / autocorrelationDenominator);
    }

    ArrayDimMC sumAutocorrelation;
    sumAutocorrelation.setZero();
    std::array<bool, MarkovChain::dim> autocorrelationConverged{};
    auto lastAutocorrelation{&autocorrelationFunction.front().second};
    for (auto&& [lag, autocorrelation] : autocorrelationFunction) {
        if (lag == 0) {
            continue;
        }
        for (int k{}; k < MarkovChain::dim; ++k) {
            if (autocorrelationConverged[k]) {
                continue;
            }
            if (autocorrelation[k] < 0) {
                autocorrelationConverged[k] = true;
            }
            sumAutocorrelation[k] += ((*lastAutocorrelation)[k] + autocorrelation[k]) / 2 * deltaLag +
                                     ((*lastAutocorrelation)[k] - autocorrelation[k]) / 2; // approximate sum
        }
        lastAutocorrelation = &autocorrelation;
    }
    if (not std::ranges::all_of(autocorrelationConverged, [](auto c) { return c; })) {
        PrintWarning(fmt::format("Autocorrelation not converged. Try increasing ACF sample size (current: {})", fACFSampleSize));
    }

    double meanSumAutocorrelation{};
    for (auto&& rho : std::as_const(sumAutocorrelation)) {
        meanSumAutocorrelation += muc::pow(rho, 2);
    }
    meanSumAutocorrelation = std::sqrt(meanSumAutocorrelation / sumAutocorrelation.size());
    if (mplr::available()) {
        const auto worldComm{mplr::comm_world()};
        worldComm.iallreduce(std::plus{}, meanSumAutocorrelation).wait(mplr::duty_ratio::preset::relaxed);
        meanSumAutocorrelation /= worldComm.size();
    }
    // Here sumAutocorrelation = sum(rho_k,0,inf) = sum(rho_k,1,inf)+1,
    // So N_eff = N/(1+2*sum(rho_k,1,inf)) = N/(2*sum(rho_k,0,inf)-1) => int. ac. = 2*sum(rho_k,0,inf)-1
    const auto integratedAutocorrelation{2 * meanSumAutocorrelation - 1};
    MasterPrintLn("Approximate mean integrated autocorrelation: {:.2f}.", integratedAutocorrelation);
    fThinningSize = fThinningRatio * integratedAutocorrelation;
    MasterPrintLn("Thinning Markov chain by 1/{}.", fThinningSize + 1);

    fMCMCInitialized = true;
    auto time{muc::chrono::seconds<double>{stopwatch.read()}.count()};
    if (mplr::available()) {
        mplr::comm_world().ireduce(mplr::max<double>{}, 0, time).wait(mplr::duty_ratio::preset::relaxed);
    }
    MasterPrint("{} initialized in {:.3f}s.\n"
                "\n",
                thisName, time);
    return autocorrelationFunction;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::operator()(CLHEP::HepRandomEngine& rng, InitialStateMomenta) -> Event {
    if (not fMCMCInitialized) [[unlikely]] {
        PrintWarning("Markov chain not initialized. Initializing it");
        MCMCInitialize(rng);
    }
    for (unsigned i{}; i < fThinningSize; ++i) {
        NextEvent(rng);
    }
    NextEvent(rng);
    return fMC.event;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::ISMomenta(const InitialStateMomenta& pI) -> void {
    if constexpr (M == 1) {
        if (pI.isNear(Base::ISMomenta(), muc::default_tolerance<double>)) {
            MCMCInitializeRequired();
        }
    } else {
        if (not std::ranges::equal(pI, Base::ISMomenta(),
                                   [](auto p, auto q) { return p.isNear(q, muc::default_tolerance<double>); })) {
            MCMCInitializeRequired();
        }
    }
    Base::ISMomenta(pI);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::Mass(const std::array<double, N>& mass) -> void {
    if (not std::ranges::equal(mass, this->fGENBOD.Mass(),
                               [](auto a, auto b) { return muc::isclose(a, b); })) {
        MCMCInitializeRequired();
    }
    Base::Mass(mass);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::IRCut(int i, double cut) -> void {
    MCMCInitializeRequired();
    Base::IRCut(i, cut);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::AddIdenticalSet(std::vector<int> set) -> void {
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
auto MCMCGenerator<M, N, A>::MCMCInitializeRequired() -> void {
    fMCMCInitialized = false;
    fThinningSize = 0;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::DirectPhaseSpace(const RandomState& u) -> std::pair<Event, double> {
    auto event{this->fGENBOD(u, Base::ISMomenta())};
    auto detJ{event.weight};
    event.weight = 1;
    return {std::move(event), detJ};
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::PhaseSpace(const MarkovChain::State& state) -> std::pair<Event, double> {
    auto [event, detJ]{DirectPhaseSpace(state.u)};
    event.p = std::apply([&](auto... i) { return FinalStateMomenta{event.p[i]...}; }, state.pID);
    return {std::move(event), detJ};
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::ProposePID(CLHEP::HepRandomEngine& rng, const std::array<int, N>& pID0, std::array<int, N>& pID) -> void {
    pID = pID0;
    // Walk particle mapping if there are identical particles
    if (fIdenticalSet.empty() or rng.flat() < 0.5) {
        return;
    }
    const auto RandomIndex([&rng](int n) {
        muc::assume(n >= 2);
        auto i{static_cast<int>(n * rng.flat())};
        while (i == n) [[unlikely]] {
            i = n * rng.flat();
        }
        return i;
    });
    const auto& idSet{fIdenticalSet.size() == 1 ?
                          fIdenticalSet.front() :
                          fIdenticalSet[RandomIndex(fIdenticalSet.size())]};
    if (idSet.size() == 2) {
        std::swap(pID[idSet[0]], pID[idSet[1]]);
    } else {
        const auto idA{RandomIndex(idSet.size())};
        const auto idB{(idA + 1) % idSet.size()};
        std::swap(pID[idSet[idA]], pID[idSet[idB]]);
    }
}

} // namespace Mustard::inline Physics::inline Generator
