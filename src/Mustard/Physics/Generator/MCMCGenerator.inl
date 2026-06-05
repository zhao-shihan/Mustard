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
                                      std::optional<double> thinningRatio, std::optional<int> acfSampleSize) :
    Base{pI, pdgID, mass},
    fThinningRatio{1.2},
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
MCMCGenerator<M, N, A>::MCMCGenerator(const InitialStateMomenta& pI, const typename A::InitialStatePolarization& polarization,
                                      const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                      std::optional<double> thinningRatio, std::optional<int> acfSampleSize) // clang-format off
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> : // clang-format on
    MCMCGenerator{pI, pdgID, mass, std::move(thinningRatio), std::move(acfSampleSize)} {
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
auto MCMCGenerator<M, N, A>::ThinningRatio(double value) -> void {
    if (not std::isfinite(value)) [[unlikely]] {
        PrintError(fmt::format("Non-finite thinning ratio not allowed (got {}), not setting it.", value));
        return;
    }
    if (value < 0) [[unlikely]] {
        PrintError(fmt::format("Negative thinning ratio not allowed (got {}), not setting it.", value));
        return;
    }
    if (value > 10) [[unlikely]] {
        PrintWarning(fmt::format("Suspicious thinning ratio (got {}).", value));
    }
    fThinningRatio = value;
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
auto MCMCGenerator<M, N, A>::MCMCInitialize(CLHEP::HepRandomEngine& rng) -> AutocorrelationFunction {
    if (fACFSampleSize == fgDefaultInvalidACFSampleSize) {
        Throw<std::logic_error>("ACF sample size not set.");
    }

    const auto thisName{muc::try_demangle(typeid(*this).name())};
    MasterPrintLn("Initializing {}...", thisName);
    muc::chrono::stopwatch stopwatch;

    // Reseed random engine for statistical safety
    Parallel::ReseedRandomEngine(&rng);

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
        rng.flatArray(fMC.state.u.size(), fMC.state.u.data());
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

    if (Env::VerboseLevelReach<'I'>()) {
        struct {
            Eigen::Vector<double, MarkovChain::dim> u;
            Eigen::Matrix<double, MarkovChain::dim, MarkovChain::dim> uuT;
        } mean;
        mean.u.setZero();
        mean.uuT.setZero();
        double sumMSqAcceptanceDetJ{};
        for (int i{}; i < fACFSampleSize;) {
            Eigen::Vector<double, MarkovChain::dim> u;
            rng.flatArray(u.size(), u.data());
            auto [event, detJ]{DirectPhaseSpace(VectorCast<RandomState>(u))};
            if (not this->InfraredSafe(event.p)) {
                continue;
            }
            const auto acceptance{this->Acceptance(event.p)};
            const auto mSqAcceptanceDetJ{this->MSqAcceptanceDetJ(event.p, acceptance, detJ)};
            const auto uMSqAcceptanceDetJ{(mSqAcceptanceDetJ * u).eval()};
            mean.u += uMSqAcceptanceDetJ;
            mean.uuT += uMSqAcceptanceDetJ * u.transpose();
            sumMSqAcceptanceDetJ += mSqAcceptanceDetJ;
            ++i;
        }
        mean.u /= sumMSqAcceptanceDetJ;
        mean.uuT /= sumMSqAcceptanceDetJ;
        const auto covariance{(mean.uuT - mean.u * mean.u.transpose()).eval()};
        std::ostringstream covOSS;
        covOSS << "Covariance of random state u (weighted by |M|^2 * acceptance * |J|):\n"
               << covariance;
        PrintInfo(covOSS.view());
    }

    // Estimate autocorrelation and decide thinning
    auto autocorrelationFunction{EstimateACFAndDecideThinning(rng)};

    fMCMCInitialized = true;
    auto time{muc::chrono::seconds<double>{stopwatch.read()}.count()};
    if (mplr::available()) {
        mplr::comm_world().reduce(mplr::max<double>{}, 0, time);
    }
    MasterPrint("{} initialized in {:.3f}s.\n"
                "\n",
                thisName, time);
    return autocorrelationFunction;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::operator()(CLHEP::HepRandomEngine& rng, InitialStateMomenta) -> Event {
    if (not fMCMCInitialized) [[unlikely]] {
        PrintWarning("Markov chain not initialized. Initializing it.");
        MCMCInitialize(rng);
    }
    for (int i{}; i < fThinningSize; ++i) {
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
    fThinningSize = 0;
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
auto MCMCGenerator<M, N, A>::ProposePID(CLHEP::HepRandomEngine& rng, const std::array<int, N>& pID0, std::array<int, N>& pID) -> void {
    pID = pID0;
    // Walk particle mapping if there are identical particles
    if (this->IdenticalSet().empty() or rng.flat() < 0.5) {
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

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::EstimateACFAndDecideThinning(CLHEP::HepRandomEngine& rng) -> AutocorrelationFunction {
    // Estimate autocorrelation and decide thinning
    MasterPrintLn("Estimating autocorrelation and decide thinning...");
    using ArrayDimMC = Eigen::Array<double, MarkovChain::dim, 1>;

    std::vector<ArrayDimMC> sample(fACFSampleSize);
    for (int i{}; i < fACFSampleSize; ++i) {
        NextEvent(rng);
        std::ranges::copy(fMC.state.u, sample[i].begin());
    }
    ArrayDimMC sampleMean;
    sampleMean.setZero();
    for (auto&& x : sample) {
        sampleMean += x;
    }
    sampleMean /= fACFSampleSize;

    // Compute autocorrelation via Wiener-Khinchin theorem (FFT-based)
    const auto maxLag{2 * fACFSampleSize / 3};
    AutocorrelationFunction autocorrelationFunction(maxLag + 1);
    {
        Eigen::FFT<double> fft{{}, Eigen::FFT<double>::HalfSpectrum};
        const auto paddedSize{2 * fACFSampleSize};
        const auto halfPaddedSize{paddedSize / 2 + 1};
        Eigen::VectorXd signal(paddedSize);
        Eigen::VectorXcd spectrum(halfPaddedSize);
        for (int k{}; k < MarkovChain::dim; ++k) {
            // Build mean-subtracted zero-padded signal
            for (int i{}; i < fACFSampleSize; ++i) {
                signal[i] = sample[i][k] - sampleMean[k];
            }
            signal.segment(fACFSampleSize, paddedSize - fACFSampleSize).setZero();
            // Spec(x) = FFT(x)
            fft.fwd(spectrum, signal);
            // Power(x) = |Spec(x)|^2 = |FFT(x)|^2
            for (int i{}; i < halfPaddedSize; ++i) {
                spectrum[i] = {std::norm(spectrum[i]), 0.};
            }
            // ACF(x) ∝ IFFT(Power(x)) = IFFT(|FFT(x)|^2)
            fft.inv(signal, spectrum);
            // Check zero-lag autocovariance
            if (signal[0] < std::numeric_limits<double>::min()) [[unlikely]] {
                PrintWarning(fmt::format("Zero-lag autocovariance for dimension {} is not normal (got {}). Setting ACF to 1.", k, signal[0]));
                std::ranges::for_each(autocorrelationFunction, [k](auto& rho) { rho[k] = 1; });
                continue;
            }
            // Keep only the first maxLag+1 lags
            for (int lag{}; lag <= maxLag; ++lag) {
                autocorrelationFunction[lag][k] = signal[lag] / signal[0];
            }
        }
    }

    // Integrate autocorrelation by summing all lags until "convergence"
    // sumAutocorrelation = sum(rho_k, 1, k_c)
    ArrayDimMC sumAutocorrelation;
    sumAutocorrelation.setZero();
    std::array<int, MarkovChain::dim> negativeRhoCount{};
    const auto convergenceThreshold{maxLag / 20}; // require at least some rho to be negative to avoid false convergence from noise spikes
    for (int lag{1}; lag <= maxLag; ++lag) {
        if (std::ranges::all_of(negativeRhoCount, [&](auto n) { return n >= convergenceThreshold; })) {
            break;
        }
        for (int k{}; k < MarkovChain::dim; ++k) {
            if (negativeRhoCount[k] >= convergenceThreshold) {
                continue;
            }
            sumAutocorrelation[k] += autocorrelationFunction[lag][k];
            negativeRhoCount[k] += (autocorrelationFunction[lag][k] < 0);
        }
    }
    if (not std::ranges::all_of(negativeRhoCount, [&](auto n) { return n >= convergenceThreshold; })) {
        PrintWarning(fmt::format("Autocorrelation not converged. Try increasing ACF sample size (current: {}). "
                                 "Generated events may be highly correlated.",
                                 fACFSampleSize));
    }

    // Quadratic mean of sumAutocorrelation across dimensions
    auto meanSumAutocorrelation{std::sqrt(sumAutocorrelation.square().mean())};
    if (mplr::available()) {
        const auto worldComm{mplr::comm_world()};
        worldComm.iallreduce(std::plus{}, meanSumAutocorrelation)
            .wait(mplr::duty_ratio::preset::moderate);
        meanSumAutocorrelation /= worldComm.size();
    }
    // Here sumAutocorrelation = sum(rho_k, 1, k_c),
    // So N_eff = N / (1 + 2 * sum(rho_k, 1, k_c)) = N / (1 + 2 * sumAutocorrelation)
    //   => integratedAutocorrelation = N / N_eff = 1 + 2 * sumAutocorrelation
    const auto integratedAutocorrelation{1 + 2 * meanSumAutocorrelation};
    MasterPrintLn("Approximate mean integrated autocorrelation: {:.2f}.", integratedAutocorrelation);
    fThinningSize = std::min(fThinningRatio * integratedAutocorrelation,
                             static_cast<double>(std::numeric_limits<int>::max() / 2));
    MasterPrintLn("Thinning Markov chain by 1/{}.", fThinningSize + 1);

    return autocorrelationFunction;
}

} // namespace Mustard::inline Physics::inline Generator
