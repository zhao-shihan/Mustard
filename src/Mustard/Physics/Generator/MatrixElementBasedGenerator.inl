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
MatrixElementBasedGenerator<M, N, A>::MatrixElementBasedGenerator(const InitialStateMomenta& pI, const std::array<int, N>& pdgID, const std::array<double, N>& mass) :
    EventGenerator<M, N>{},
    fGENBOD{pdgID, mass},
    fISMomenta{},
    fBoostFromLabToCM{},
    fMatrixElement{},
    fIRCut{},
    fBias{[](auto&&) { return 1; }} {
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
auto MatrixElementBasedGenerator<M, N, A>::EstimateNormalizationFactor(Executor<unsigned long long>& executor, double precisionGoal,
                                                                       std::array<Math::MCIntegrationState, 2> integrationState,
                                                                       CLHEP::HepRandomEngine& rng) -> std::pair<Math::IntegrationResult, std::array<Math::MCIntegrationState, 2>> {
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
    const auto fractionPrecisionGoal{precisionGoal / std::numbers::sqrt2};

    // Compute denominator
    MasterPrintLn("Computing denominator integral.");
    const auto DenomIntegrand{[this](const Event& event) {
        const auto& [detJ, _, pF]{event};
        return ValidBiasedMSqDetJ(pF, 1, detJ);
    }};
    const auto denom{Integrate(DenomIntegrand, fractionPrecisionGoal, integrationState[0], executor, rng)};
    MasterPrintLn("Denominator integration completed.");

    // Compute numerator
    MasterPrintLn("Computing numerator integral.");
    const auto NumerIntegrand{[this](const Event& event) {
        const auto& [detJ, _, pF]{event};
        const auto bias{ValidBias(pF)};
        return ValidBiasedMSqDetJ(pF, bias, detJ);
    }};
    const auto numer{Integrate(NumerIntegrand, fractionPrecisionGoal, integrationState[1], executor, rng)};
    MasterPrintLn("Numerator integration completed.");

    // Combine result
    Math::IntegrationResult result;
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
auto MatrixElementBasedGenerator<M, N, A>::ISMomenta(const InitialStateMomenta& pI) -> void {
    fISMomenta = pI;
    fBoostFromLabToCM = -this->CalculateBoost(pI);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::PhaseSpace(CLHEP::HepRandomEngine& rng) {
    if constexpr (M == 1) {
        return fGENBOD(rng, fISMomenta);
    } else {
        return fGENBOD(rng, muc::ranges::reduce(fISMomenta));
    }
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
    const auto cmE{this->CalculateCMEnergy(fISMomenta)};
    if (muc::pow(fGENBOD.Mass(i) / cmE, 2) > muc::default_tolerance<double>) [[unlikely]] {
        PrintWarning(fmt::format("IR cut set for massive particle {} (mass = {})", i, fGENBOD.Mass(i)));
    }
    fIRCut.push_back({i, cut});
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::IRSafe(const FinalStateMomenta& pF) const -> bool {
    for (auto&& [i, cut] : fIRCut) {
        auto p{pF[i]};
        p.boost(fBoostFromLabToCM);
        if (p.e() <= cut) {
            return false;
        }
    }
    return true;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::ValidBias(const FinalStateMomenta& pF) const -> double {
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
auto MatrixElementBasedGenerator<M, N, A>::ValidBiasedMSqDetJ(const FinalStateMomenta& pF, double bias, double detJ) const -> double {
    const auto value{fMatrixElement(fISMomenta, pF) * bias * detJ}; // |M|² × bias × |J|
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

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MatrixElementBasedGenerator<M, N, A>::Integrate(std::regular_invocable<const Event&> auto&& Integrand, double precisionGoal,
                                                     Math::MCIntegrationState& state, Executor<unsigned long long>& executor, CLHEP::HepRandomEngine& rng) -> Math::IntegrationResult {
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
            const auto event{PhaseSpace(rng)};
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
        Math::IntegrationResult result;
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
        const auto factor{1.05 * muc::pow(relativeUncertainty / precisionGoal, 2) - 1};
        if (std::isfinite(factor)) {
            nSample = factor * state.n;
        } else {
            nSample *= 10;
        }
        nSample = std::max<unsigned long long>(executor.NProcess(), nSample);
    }
}

} // namespace Mustard::inline Physics::inline Generator
