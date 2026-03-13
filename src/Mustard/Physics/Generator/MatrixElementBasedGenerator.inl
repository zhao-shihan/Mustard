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
    fMomenta{},
    fBoostFromLabToCM{},
    fFSSymmetryFactor{1},
    fIdenticalSet{},
    fSoftCutoff{},
    fCollinearCutoff{},
    fInfraredUnsafePID{},
    fAcceptance{},
    fAcceptanceGt1Counter{},
    fNegativeMSqCounter{} {
    Momenta(pI);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
MatrixElementBasedGenerator<M, N, A>::MatrixElementBasedGenerator(const InitialStateMomenta& pI, const typename A::InitialStatePolarization& polarization,
                                                                  const std::array<int, N>& pdgID, const std::array<double, N>& mass) // clang-format off
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> : // clang-format on
    MatrixElementBasedGenerator{pI, pdgID, mass} {
    Polarization(polarization);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::PhaseSpaceIntegral(Executor<unsigned long long>& executor, double precisionGoal,
                                                              MCIntegrationState integrationState,
                                                              CLHEP::HepRandomEngine& rng) -> std::tuple<Estimate, double, MCIntegrationState> {
    MasterPrintLn("Integrating |M|^2 * (Acceptance) over phase space in {}.\n", muc::try_demangle(typeid(*this).name()));
    if (fInfraredUnsafePID.empty()) {
        MasterPrintLn("No infrared cutoff is set.\n");
    } else {
        MasterPrintLn("Infrared cutoff(s) in the c.m. frame:");
        for (auto&& [i, cutoff] : std::as_const(fSoftCutoff)) {
            MasterPrintLn("  Soft cutoff for particle {}: k.e. > {}", i, cutoff);
        }
        for (auto&& [i, cutoff] : std::as_const(fCollinearCutoff)) {
            const auto radian{std::acos(cutoff)};
            const auto degree{radian / CLHEP::degree};
            MasterPrintLn("  Collinear cutoff for particle {}: angle(p{}, p{}) > {:.6} rad ({:.6} deg)", i, i.first, i.second, radian, degree);
        }
        MasterPrint("\n");
    }

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
        const auto acceptance{Acceptance(pF)};
        return MSqAcceptanceDetJ(pF, acceptance, detJ);
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
                "Integration state (integration can be continued from here):\n"
                "  {} {} {}\n"
                "The integral of |M|^2 * (Acceptance) over phase space:\n"
                "  {} +/- {}  (rel. unc.: {:.3}%, N_eff: {:.2f})\n",
                time, summation[0], summation[1], nSample, integral.value, integral.uncertainty,
                integral.uncertainty / integral.value * 100, nEff);
    return {integral, nEff, integrationState};
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::Momenta(const InitialStateMomenta& pI) -> void {
    fMomenta = pI;
    fBoostFromLabToCM = -this->CalculateBoost(pI);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::Polarization() const -> const typename A::InitialStatePolarization&
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> {
    return fMatrixElement.Polarization();
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::Polarization(int i) const -> Vector3D
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) {
    return fMatrixElement.Polarization(i);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::Polarization(const typename A::InitialStatePolarization& pol) -> void
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> {
    fMatrixElement.Polarization(pol);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::Polarization(int i, Vector3D pol) -> void
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) {
    fMatrixElement.Polarization(i, pol);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::AddIdenticalSet(std::vector<int> set) -> void {
    for (auto&& i : std::as_const(set)) {
        if (i < 0 or i >= N) [[unlikely]] {
            PrintError(fmt::format("Invalid particle index in identical set (valid range is [0, {}), got {}), ignoring the set", N, i));
            return;
        }
    }
    muc::timsort(set);
    if (const auto duplicate{std::ranges::unique(set)};
        not duplicate.empty()) [[unlikely]] {
        PrintWarning(fmt::format("There is/are {} duplicate index/indices in identical set, removing it/them", duplicate.size()));
        set.erase(duplicate.begin(), duplicate.end());
    }
    if (set.size() < 2) [[unlikely]] {
        PrintWarning(fmt::format("Identical set should have at least 2 elements (got {}), ignoring it", set.size()));
        return;
    }
    for (auto&& addedSet : std::as_const(fIdenticalSet)) {
        const auto duplicated{std::ranges::find_first_of(addedSet, set)};
        if (duplicated != addedSet.cend()) [[unlikely]] {
            PrintError(fmt::format("Particle {} added across different identical sets, ignoring the set", *duplicated));
            return;
        }
    }
    fFSSymmetryFactor /= muc::factorial(set.size());
    fIdenticalSet.emplace_back(std::move(set));
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::SoftCutoff(int i, double cutoff) -> void {
    if (i < 0 or i >= N) [[unlikely]] {
        PrintError(fmt::format("Invalid particle index (valid range is [0, {}), got {})", N, i));
        return;
    }
    if (cutoff <= 0) [[unlikely]] {
        PrintWarning(fmt::format("Non-positive soft cutoff for particle {} (got {})", i, cutoff));
    }
    fSoftCutoff[i] = cutoff;
    fInfraredUnsafePID.insert(i);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::CollinearCutoff(std::pair<int, int> pID, double cutoff) -> void {
    auto& [i, j]{pID};
    if (i > j) {
        std::swap(i, j);
    }
    if (i < 0 or i >= N or j < 0 or j >= N) [[unlikely]] {
        PrintError(fmt::format("Invalid particle index (valid range is [0, {}), got (i, j) = {})", N, pID));
        return;
    }
    if (i == j) [[unlikely]] {
        PrintError(fmt::format("Collinear cutoff cannot be set for the same particle (got (i, j) = {})", pID));
        return;
    }
    if (cutoff <= 0) [[unlikely]] {
        PrintWarning(fmt::format("Non-positive collinear cutoff for particle pair {} (got {})", pID, cutoff));
    }
    fCollinearCutoff[pID] = std::cos(cutoff); // store cosθ to speed up the check (cosθ ≥ cutoff means θ ≤ cutoff, i.e. collinear)
    fInfraredUnsafePID.insert(i);
    fInfraredUnsafePID.insert(j);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::InfraredSafe(const FinalStateMomenta& pF) const -> bool {
    FinalStateMomenta infraredUnsafePF;
    for (auto&& i : std::as_const(fInfraredUnsafePID)) {
        infraredUnsafePF[i] = pF[i];
        infraredUnsafePF[i].boost(fBoostFromLabToCM);
    }
    for (auto&& [i, cutoff] : std::as_const(fSoftCutoff)) {
        const auto& p{infraredUnsafePF[i]};
        if (p.e() - p.m() <= cutoff) {
            return false;
        }
    }
    for (auto&& [i, cutoff] : std::as_const(fCollinearCutoff)) {
        const auto p1{infraredUnsafePF[i.first].vect()};
        const auto p2{infraredUnsafePF[i.second].vect()};
        if (p1.cosTheta(p2) >= cutoff) { // cosθ ≥ cutoff means θ ≤ cutoff, i.e. collinear
            return false;
        }
    }
    return true;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::Acceptance(AcceptanceFunction acceptance) -> void {
    fAcceptance = std::move(acceptance);
    fAcceptanceGt1Counter = 0;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::Acceptance(const FinalStateMomenta& pF) const -> double {
    if (not fAcceptance) {
        return 1;
    }
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
auto MatrixElementBasedGenerator<M, N, A>::MSqAcceptanceDetJ(const FinalStateMomenta& pF, double acceptance, double detJ) const -> double {
    Expects(acceptance >= 0);
    Expects(detJ > 0);
    if (acceptance <= std::numeric_limits<double>::epsilon()) {
        return 0;
    }
    const auto mSq{fMatrixElement(fMomenta, pF)};
    const auto result{fFSSymmetryFactor * mSq * acceptance * detJ}; // 1/S × |M|² × acceptance × |J|
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
        Throw<std::runtime_error>(fmt::format("Infinite 1/S * |M|^2 * acceptance * |J| found (got {} at {})", result, Format(pF, acceptance, detJ)));
    }
    return result;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::Integrate(std::regular_invocable<const Event&> auto&& Integrand, double precisionGoal,
                                                     MCIntegrationState& state, Executor<unsigned long long>& executor, CLHEP::HepRandomEngine& rng) -> std::pair<Estimate, double> {
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
            if (not InfraredSafe(event.p)) {
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
        Estimate integral;
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
