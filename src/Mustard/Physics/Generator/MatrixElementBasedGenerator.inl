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
MatrixElementBasedGenerator<M, N, A>::MatrixElementBasedGenerator(const InitialStateMomenta& pI, const std::array<int, N>& pdgID, const std::array<double, N>& mass) :
    EventGenerator<M, N>{},
    fMatrixElement{},
    fGENBOD{pdgID, mass},
    fISMomenta{},
    fBoostFromLabToCM{},
    fIRCut{},
    fAcceptance{[](auto&&) { return 1; }},
    fAcceptanceGt1Counter{},
    fNegativeMSqCounter{} {
    ISMomenta(pI);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
MatrixElementBasedGenerator<M, N, A>::MatrixElementBasedGenerator(const InitialStateMomenta& pI, CLHEP::Hep3Vector polarization,
                                                                  const std::array<int, N>& pdgID, const std::array<double, N>& mass) // clang-format off
    requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>> : // clang-format on
    MatrixElementBasedGenerator{pI, pdgID, mass} {
    fMatrixElement.InitialStatePolarization(polarization);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
MatrixElementBasedGenerator<M, N, A>::MatrixElementBasedGenerator(const InitialStateMomenta& pI, const std::array<CLHEP::Hep3Vector, M>& polarization,
                                                                  const std::array<int, N>& pdgID, const std::array<double, N>& mass) // clang-format off
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) : // clang-format on
    MatrixElementBasedGenerator{pI, pdgID, mass} {
    fMatrixElement.InitialStatePolarization(polarization);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::PhaseSpaceIntegral(Executor<unsigned long long>& executor, double precisionGoal,
                                                              Math::MCIntegrationState integrationState,
                                                              CLHEP::HepRandomEngine& rng) -> std::tuple<Math::Estimate, double, Math::MCIntegrationState> {
    MasterPrint("Integrate |M|^2 x (Acceptance) on phase space in {}.\n"
                "\n",
                muc::try_demangle(typeid(*this).name()));

    // Reseed random engine for statistical safety
    Parallel::ReseedRandomEngine(&rng);

    // Set task name
    auto originalExecutionName{executor.ExecutionName()};
    auto originalTaskName{executor.TaskName()};
    auto _{gsl::finally([&] {
        executor.ExecutionName(std::move(originalExecutionName));
        executor.TaskName(std::move(originalTaskName));
    })};
    executor.ExecutionName("Integration");
    executor.TaskName("Sample");

    // Start integration
    const auto Integrand{[this](const Event& event) {
        const auto& [detJ, _, pF]{event};
        const auto acceptance{ValidAcceptance(pF)};
        return ValidMSqAcceptanceDetJ(pF, acceptance, detJ);
    }};
    muc::chrono::stopwatch stopwatch;
    const auto [integral, nEff]{Integrate(Integrand, precisionGoal, integrationState, executor, rng)};
    auto time{muc::chrono::seconds<double>{stopwatch.read()}.count()};
    if (mplr::available()) {
        mplr::comm_world().ireduce(mplr::max<double>{}, 0, time).wait(mplr::duty_ratio::preset::relaxed);
    }

    // Report result
    const auto& [summation, nSample]{integrationState};
    MasterPrint("Integration completed in {:.3f}s.\n"
                "Integration state (integration can be contined from here):\n"
                "  {} {} {}\n"
                "|M|^2 x (Acceptance) phase-space integral:\n"
                "  {} +/- {}  (rel. unc.: {:.3}%, N_eff: {:.2f})\n",
                time, summation[0], summation[1], nSample, integral.value, integral.uncertainty,
                integral.uncertainty / integral.value * 100, nEff);
    return {integral, nEff, integrationState};
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::ISMomenta(const InitialStateMomenta& pI) -> void {
    fISMomenta = pI;
    fBoostFromLabToCM = -this->CalculateBoost(pI);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::InitialStatePolarization() const -> CLHEP::Hep3Vector
    requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>> {
    return fMatrixElement.InitialStatePolarization();
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::InitialStatePolarization(int i) const -> CLHEP::Hep3Vector
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) {
    return fMatrixElement.InitialStatePolarization(i);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::InitialStatePolarization() const -> const std::array<CLHEP::Hep3Vector, M>&
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) {
    return fMatrixElement.InitialStatePolarization();
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::InitialStatePolarization(CLHEP::Hep3Vector pol) -> void
    requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>> {
    fMatrixElement.InitialStatePolarization(pol);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::InitialStatePolarization(int i, CLHEP::Hep3Vector pol) -> void
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) {
    fMatrixElement.InitialStatePolarization(i, pol);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::InitialStatePolarization(const std::array<CLHEP::Hep3Vector, M>& pol) -> void
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) {
    fMatrixElement.InitialStatePolarization(pol);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::IRCut(int i, double cut) -> void {
    if (cut <= 0) [[unlikely]] {
        PrintWarning(fmt::format("Non-positive IR cut for particle {} (got {})", i, cut));
    }
    fIRCut.push_back({i, cut});
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::IRSafe(const FinalStateMomenta& pF) const -> bool {
    for (auto&& [i, cut] : fIRCut) {
        auto p{pF[i]};
        p.boost(fBoostFromLabToCM);
        if (p.e() - p.m() <= cut) {
            return false;
        }
    }
    return true;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::Acceptance(AcceptanceFunction Acceptance) -> void {
    fAcceptance = std::move(Acceptance);
    fAcceptanceGt1Counter = 0;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::ValidAcceptance(const FinalStateMomenta& pF) const -> double {
    const auto acceptance{fAcceptance(pF)};
    constexpr auto Format{[](const FinalStateMomenta& pF) {
        std::string where;
        for (auto&& p : pF) {
            where += fmt::format("[{}; {}, {}, {}]", p.e(), p.x(), p.y(), p.z());
        }
        return where;
    }};
    if (not std::isfinite(acceptance)) {
        Throw<std::runtime_error>(fmt::format("Infinite acceptance found (got {} at {})", acceptance, Format(pF)));
    }
    if (acceptance < 0) {
        Throw<std::runtime_error>(fmt::format("Negative acceptance found (got {} at {})", acceptance, Format(pF)));
    }
    if (acceptance > 1) [[unlikely]] {
        constexpr std::int8_t maxIncidentReport{10};
        if (fAcceptanceGt1Counter < maxIncidentReport) {
            ++fAcceptanceGt1Counter;
            PrintWarning(fmt::format("Acceptance > 1 (incident: {}, this warning will be suppressed after {} incidents)",
                                     fAcceptanceGt1Counter, maxIncidentReport));
            if (fAcceptanceGt1Counter == maxIncidentReport) {
                PrintWarning("Warning of acceptance > 1 suppressed");
            }
        }
    }
    return acceptance;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::ValidMSqAcceptanceDetJ(const FinalStateMomenta& pF, double acceptance, double detJ) const -> double {
    Expects(acceptance >= 0);
    Expects(detJ > 0);
    if (acceptance <= std::numeric_limits<double>::epsilon()) {
        return 0;
    }
    const auto mSq{fMatrixElement(fISMomenta, pF)};
    const auto result{mSq * acceptance * detJ}; // |M|² × acceptance × |J|
    constexpr auto Format{[](const FinalStateMomenta& pF, double acceptance, double detJ) {
        auto where{fmt::format("({})", detJ)};
        for (auto&& p : pF) {
            where += fmt::format("[{}; {}, {}, {}]", p.e(), p.x(), p.y(), p.z());
        }
        where += fmt::format(" Acceptance={}", acceptance);
        return where;
    }};
    if (mSq < 0) [[unlikely]] {
        constexpr std::int8_t maxIncidentReport{10};
        if (fNegativeMSqCounter < maxIncidentReport) {
            ++fNegativeMSqCounter;
            PrintWarning(fmt::format("Negative |M|^2 (got {} at {}, incident: {}, this warning will be suppressed after {} incidents)",
                                     mSq, Format(pF, acceptance, detJ), fNegativeMSqCounter, maxIncidentReport));
            if (fNegativeMSqCounter == maxIncidentReport) {
                PrintWarning("Warning of negative |M|^2 suppressed");
            }
        }
    }
    if (not std::isfinite(result)) {
        Throw<std::runtime_error>(fmt::format("Infinite |M|^2 x (Acceptance) x |J| found (got {} at {})", result, Format(pF, acceptance, detJ)));
    }
    return result;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::Integrate(std::regular_invocable<const Event&> auto&& Integrand, double precisionGoal,
                                                     Math::MCIntegrationState& state, Executor<unsigned long long>& executor, CLHEP::HepRandomEngine& rng) -> std::pair<Math::Estimate, double> {
    if (precisionGoal <= 0) [[unlikely]] {
        Mustard::PrintWarning(fmt::format("Non-positive precision goal (got {}), taking its absolute value", precisionGoal));
        precisionGoal = std::abs(precisionGoal);
    }
    // Core integration method
    const auto Integrate{[&](unsigned long long nSample) {
        using namespace Mustard::VectorArithmeticOperator::Vector2ArithmeticOperator;
        muc::array2d sum{};
        muc::array2d compensation{};
        const auto KahanAdd{[&](muc::array2d value) { // improve numeric stability
            const auto correctedValue{value - compensation};
            const auto newSum{sum + correctedValue};
            compensation = (newSum - sum) - correctedValue;
            sum = newSum;
        }};
        executor(nSample, [&](auto) {
            const auto event{PhaseSpace(rng)};
            if (not IRSafe(event.p)) {
                return;
            }
            const auto value{Integrand(event)};
            KahanAdd({value, muc::pow(value, 2)});
        });
        if (mplr::available()) {
            mplr::comm_world().allreduce([](auto a, auto b) { return a + b; }, sum);
        }
        state.sum += sum;
        state.n += nSample;
        Math::Estimate integral;
        integral.value = state.sum[0] / state.n;
        integral.uncertainty = std::sqrt((state.sum[1] / state.n - muc::pow(integral.value, 2)) / state.n);
        const auto nEff{muc::pow(state.sum[0], 2) / state.sum[1]};
        return std::pair{integral, nEff};
    }};
    // Integration loop
    MasterPrintLn("Integration starts. Precision goal: {:.3}.", precisionGoal);
    const auto initialBatchSize{muc::to_unsigned(muc::llround(muc::pow(precisionGoal, -2)))};
    auto batchSize{std::max(1000000ull * executor.NProcess(), initialBatchSize)};
    double nSamplePerMin{}; // just a very approximate value
    for (int checkpoint{};; ++checkpoint) {
        if (state.n == 0) {
            MasterPrintLn("[Checkpoint {}] Restarting integration.", checkpoint);
        } else {
            MasterPrintLn("[Checkpoint {}] Continuing integration from state\n"
                          "  {} {} {}",
                          checkpoint, state.sum[0], state.sum[1], state.n);
        }
        MasterPrintLn("Integrate with {} samples. Precision goal: {:.3}.", batchSize, precisionGoal);
        const auto [integral, nEff]{Integrate(batchSize)};
        const auto precision{integral.uncertainty / integral.value};
        if (precision <= precisionGoal) {
            MasterPrint("Current precision: {:.3}, N_eff: {:.2f}, precision goal {:.3} reached.\n"
                        "\n"
                        "Integration completed with {} samples.\n",
                        precision, nEff, precisionGoal, state.n);
            return {integral, nEff};
        }
        MasterPrint("Current precision: {:.3}, N_eff: {:.2f}, precision goal {:.3} not reached.\n"
                    "\n",
                    precision, nEff, precisionGoal);
        // Estimate throughput
        nSamplePerMin = batchSize / muc::chrono::minutes<double>{executor.ExecutionInfo().wallTime}.count();
        // Increase total sample size adaptively
        constexpr auto zFactor{1}; // decrease z sigma to increase stability
        const auto counterFactor{1 - zFactor / std::sqrt(nEff)};
        const auto factor{std::max(0., counterFactor * muc::pow(precision / precisionGoal, 2) - 1)};
        if (std::isfinite(factor)) {
            batchSize = factor * state.n;
        } else {
            batchSize *= 10;
        }
        // Batch size should not be too samll,
        const auto batchSizeLowerBound{std::max<long long>(executor.NProcess(), std::llround(nSamplePerMin))};
        // and not too large
        const auto batchSizeUpperBound{std::llround(15 * nSamplePerMin)};
        batchSize = std::clamp<unsigned long long>(batchSize, batchSizeLowerBound, batchSizeUpperBound);
    }
}

} // namespace Mustard::inline Physics::inline Generator
