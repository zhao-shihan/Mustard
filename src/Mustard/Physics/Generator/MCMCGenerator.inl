// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
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
                                      std::optional<int> acfSampleSize, std::optional<int> acfReplicate) :
    Base{pI, pdgID, mass},
    fACFReplicate{10},
    fACFSampleSize{fgDefaultInvalidACFSampleSize},
    fMCMCInitialized{},
    fSamplingInterval{},
    fMC{} {
    if (acfSampleSize) {
        ACFSampleSize(*acfSampleSize);
    }
    if (acfReplicate) {
        ACFReplicate(*acfReplicate);
    }
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
MCMCGenerator<M, N, A>::MCMCGenerator(const InitialStateMomenta& pI, const typename A::InitialStatePolarization& polarization,
                                      const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                      std::optional<int> acfSampleSize, std::optional<int> acfReplicate) // clang-format off
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> : // clang-format on
    MCMCGenerator{pI, pdgID, mass, std::move(acfSampleSize), std::move(acfReplicate)} {
    this->Polarization(polarization);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::Polarization() const -> const typename A::InitialStatePolarization&
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> {
    return Base::Polarization();
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::Polarization(int i) const -> Vector3D
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) {
    return Base::Polarization(i);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::Polarization(const typename A::InitialStatePolarization& pol) -> void
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> {
    if constexpr (M == 1) {
        if (not pol.isNear(Polarization())) {
            MCMCInitializationRequired();
        }
    } else {
        if (not std::ranges::equal(pol, Polarization(),
                                   [](auto&& a, auto&& b) { return a.isNear(b); })) {
            MCMCInitializationRequired();
        }
    }
    Base::Polarization(pol);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::Polarization(int i, Vector3D pol) -> void
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) {
    if (not pol.isNear(Polarization(i))) {
        MCMCInitializationRequired();
    }
    Base::Polarization(i, pol);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::Acceptance(AcceptanceFunction Acceptance) -> void {
    MCMCInitializationRequired();
    Base::Acceptance(Acceptance);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::ACFSampleSize(int n) -> void {
    if (n <= 0) [[unlikely]] {
        PrintError("Zero or negative ACF sample size not allowed, not setting it.");
        return;
    }
    if (n >= std::numeric_limits<int>::max() / 2) [[unlikely]] {
        PrintError(fmt::format("ACF sample size too large (got {}), not setting it.", n));
        return;
    }
    if (n < 256) [[unlikely]] {
        PrintError(fmt::format("ACF sample size too small (got {}), not setting it.", n));
        return;
    }
    fACFSampleSize = n;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::ACFReplicate(int n) -> void {
    if (n < 3) [[unlikely]] {
        PrintError(fmt::format("ACF replicate count must be at least 3 (got {}), not setting it.", n));
        return;
    }
    if (n >= std::numeric_limits<int>::max() / 2) [[unlikely]] {
        PrintError(fmt::format("ACF replicate count too large (got {}), not setting it.", n));
        return;
    }
    fACFReplicate = n;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::MCMCInitialize(Executor<long long>& executor, muc::rng_ref rng) -> MarkovChainStatistic<MarkovChain::dim> {
    if (fACFSampleSize == fgDefaultInvalidACFSampleSize) {
        Throw<std::logic_error>("ACF sample size not set.");
    }
    fMCMCInitialized = false;

    const auto thisName{muc::try_demangle(typeid(*this).name())};
    Random::Uniform<double> uniform{};
    MasterPrintLn("Initializing {}...", thisName);
    muc::chrono::stopwatch stopwatch;

    // Reseed random engine for statistical safety
    Parallel::ReseedRandomEngine(rng);

    // find phase space
    MasterPrintLn("Finding phase space...");
    muc::ranges::iota(fMC.state.pID, 0);
    for (long long counter{};; ++counter) {
        if (counter >= 1'000'000'000) {
            const auto billion{std::div(counter, 1'000'000'000ll)};
            if (billion.rem == 0) [[unlikely]] {
                PrintWarning(fmt::format("Tried {} billion times to find phase space in {} initialization, still trying...", billion.quot, thisName));
            }
            if (billion.quot == 1000) {
                Throw<std::runtime_error>(fmt::format("Failed to find phase space after 1 trillion tries in {} initialization.", thisName));
            }
        }
        std::ranges::generate(fMC.state.u, [&] { return uniform(rng); });
        auto [event, detJ]{DirectPhaseSpace(fMC.state.u)};
        if (not this->InfraredSafe(event.p)) {
            continue;
        }
        const auto acceptance{this->Acceptance(event.p)};
        const auto mSqAcceptanceDetJ{this->MSqAcceptanceDetJ(event.p, acceptance, detJ)};
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

    // Estimate covariance of random state u weighted by |M|^2 * acceptance * |J| for diagnostic purposes
    if (Env::VerboseLevelReach<'I'>()) {
        Statistic<MarkovChain::dim> stat;
        for (int i{}; i < fACFSampleSize;) {
            Eigen::Vector<double, MarkovChain::dim> u;
            std::ranges::generate(u, [&] { return uniform(rng); });
            auto [event, detJ]{DirectPhaseSpace(VectorCast<RandomState>(u))};
            if (not this->InfraredSafe(event.p)) {
                continue;
            }
            const auto acceptance{this->Acceptance(event.p)};
            const auto mSqAcceptanceDetJ{this->MSqAcceptanceDetJ(event.p, acceptance, detJ)};
            stat.Fill(u, mSqAcceptanceDetJ);
            ++i;
        }
        std::ostringstream covOSS;
        covOSS << "Covariance of random state u (weighted by |M|^2 * acceptance * |J|):\n"
               << stat.Covariance();
        PrintInfo(covOSS.view());
    }

    // Estimate autocorrelation on replicate chain segments, distributed over processes
    const auto nProcess{executor.NProcess()};
    auto nReplicate{fACFReplicate};
    if (nReplicate < nProcess) [[unlikely]] {
        MasterPrintWarning(fmt::format("ACF replicate count ({}) is less than the number of processes ({}), using {}.",
                                       nReplicate, nProcess, nProcess));
        nReplicate = nProcess;
        fACFReplicate = nReplicate;
    }
    MasterPrintLn("Estimating autocorrelation with {} replicates...", nReplicate);

    auto originalExecutionName{executor.ExecutionName()};
    auto originalOperationName{executor.OperationName()};
    auto originalTaskName{executor.TaskName()};
    const auto restoreExecutorNames{gsl::finally([&] {
        executor.ExecutionName(std::move(originalExecutionName));
        executor.OperationName(std::move(originalOperationName));
        executor.TaskName(std::move(originalTaskName));
    })};
    executor.ExecutionName("MCMC initialization");
    executor.OperationName("ACF estimation");
    executor.TaskName("replicate");

    std::optional<MarkovChainStatistic<MarkovChain::dim>> local;
    executor.Run(nReplicate, [&](long long) {
        Eigen::Matrix<double, Eigen::Dynamic, MarkovChain::dim> sample(fACFSampleSize, MarkovChain::dim);
        Eigen::Vector<double, MarkovChain::dim> state;
        for (int i{}; i < fACFSampleSize; ++i) {
            NextEvent(rng);
            std::ranges::copy(fMC.state.u, state.begin());
            sample.row(i) = state.transpose();
        }
        MarkovChainStatistic<MarkovChain::dim> statistic{sample};
        if (local) {
            *local += statistic;
        } else {
            local = std::move(statistic);
        }
    });
    if (not local) {
        Throw<std::logic_error>("No ACF replicate was executed on this process.");
    }

    // Merge the per-process statistics into a global one
    auto flat{local->ToFlat()};
    if (mplr::available()) {
        const auto& worldComm{mplr::comm_world()};
        worldComm.allreduce(std::plus{}, flat.first.data(), mplr::contiguous_layout<long long>(flat.first.size()));
        worldComm.allreduce(std::plus{}, flat.second.data(), mplr::contiguous_layout<double>(flat.second.size()));
    }
    auto statistic{MarkovChainStatistic<MarkovChain::dim>::FromFlat(flat, fACFSampleSize, local->MaxLag())};

    const auto& scalarTauStatistic{statistic.ScalarIntegratedAutocorrelationStatistic()};
    if (scalarTauStatistic.SampleSize() < 3) {
        Throw<std::logic_error>("At least 3 ACF replicates are required to determine the thinning interval.");
    }
    const auto tauEstimate{scalarTauStatistic.MeanEstimate()};
    MasterPrintLn("Scalar integrated autocorrelation: {:.3f} +/- {:.3f}.", tauEstimate.Value(), tauEstimate.Uncertainty());
    for (int i{}; i < MarkovChain::dim; ++i) {
        if (statistic.EffectiveSampleSize(i) < 100) [[unlikely]] {
            MasterPrintWarning(fmt::format("Effective sample size for dimension {} is too low: {:.2f} (IACT {:.2f}). "
                                           "The analysis of the chain may not be accurate.",
                                           i, statistic.EffectiveSampleSize(i), statistic.IntegratedAutocorrelation(i)));
        }
    }
    fSamplingInterval = static_cast<int>(std::min({tauEstimate.Value() + 3 * tauEstimate.Uncertainty(),
                                                   static_cast<double>(std::numeric_limits<int>::max() / 2)}));
    MasterPrintLn("Thinning Markov chain by 1/{}.", fSamplingInterval + 1);

    fMCMCInitialized = true;
    auto time{muc::chrono::seconds<double>{stopwatch.read()}.count()};
    if (mplr::available()) {
        mplr::comm_world().reduce(mplr::max<double>{}, 0, time);
    }
    MasterPrint("{} initialized in {:.3f}s.\n"
                "\n",
                thisName, time);
    return statistic;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::operator()(muc::rng_ref rng, InitialStateMomenta) -> Event {
    if (not fMCMCInitialized) {
        Throw<std::logic_error>("Markov chain not initialized. Call MCMCInitialize before generating events.");
    }
    for (int i{}; i < fSamplingInterval; ++i) {
        NextEvent(rng);
    }
    NextEvent(rng);
    return fMC.event;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::Momenta(const InitialStateMomenta& pI) -> void {
    if constexpr (M == 1) {
        if (not pI.isNear(Base::Momenta())) {
            MCMCInitializationRequired();
        }
    } else {
        if (not std::ranges::equal(pI, Base::Momenta(),
                                   [](auto p, auto q) { return p.isNear(q); })) {
            MCMCInitializationRequired();
        }
    }
    Base::Momenta(pI);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::Mass(const std::array<double, N>& mass) -> void {
    if (not std::ranges::equal(mass, this->fGENBOD.Mass(),
                               [](auto a, auto b) { return muc::isclose(a, b); })) {
        MCMCInitializationRequired();
    }
    Base::Mass(mass);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::SoftCutoff(int i, double cutoff) -> void {
    MCMCInitializationRequired();
    Base::SoftCutoff(i, cutoff);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::CollinearCutoff(std::pair<int, int> pID, double cutoff) -> void {
    MCMCInitializationRequired();
    Base::CollinearCutoff(pID, cutoff);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::MCMCInitializationRequired() -> void {
    fMCMCInitialized = false;
    fSamplingInterval = 0;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::DirectPhaseSpace(const RandomState& u) -> std::pair<Event, double> {
    auto event{this->fGENBOD(u, Base::Momenta())};
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
auto MCMCGenerator<M, N, A>::ProposePID(muc::rng_ref rng, const std::array<int, N>& pID0, std::array<int, N>& pID) -> void {
    pID = pID0;
    auto uniform{Random::Uniform<double>{}};
    // Walk particle mapping if there are identical particles
    if (this->IdenticalSet().empty() or uniform(rng) < 0.5) {
        return;
    }
    const auto RandomIndex([&rng, &uniform](int n) {
        muc::assume(n >= 2);
        auto i{static_cast<int>(n * uniform(rng))};
        while (i == n) [[unlikely]] {
            i = n * uniform(rng);
        }
        return i;
    });
    const auto& idSet{this->IdenticalSet().size() == 1 ?
                          this->IdenticalSet(0) :
                          this->IdenticalSet(RandomIndex(this->IdenticalSet().size()))};
    if (idSet.size() == 2) {
        std::swap(pID[idSet[0]], pID[idSet[1]]);
    } else {
        const auto idA{RandomIndex(idSet.size())};
        const auto idB{(idA + 1) % idSet.size()};
        std::swap(pID[idSet[idA]], pID[idSet[idB]]);
    }
}

} // namespace Mustard::inline Physics::inline Generator
