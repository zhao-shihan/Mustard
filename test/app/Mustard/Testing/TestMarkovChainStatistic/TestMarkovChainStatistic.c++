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

#include "Mustard/CLI/BasicCLI.h++"
#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/IO/Print.h++"
#include "Mustard/Math/Estimate.h++"
#include "Mustard/Math/MarkovChainStatistic.h++"
#include "Mustard/Math/Statistic.h++"
#include "Mustard/Testing/TestMarkovChainStatistic/TestMarkovChainStatistic.h++"

#include "Eigen/Core"

#include "muc/math"

#include "fmt/format.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <random>
#include <stdexcept>
#include <string_view>

namespace Mustard::Testing {

namespace {

auto CheckClose(double actual, double expected, double relativeTolerance, std::string_view context) -> void {
    if (const auto tolerance{relativeTolerance * std::abs(expected) + 1e-12};
        not(std::abs(actual - expected) <= tolerance)) {
        Throw<std::runtime_error>(fmt::format("{}: expected {}, got {}.", context, expected, actual));
    }
}

auto CheckEqual(auto actual, auto expected, std::string_view context) -> void {
    if (actual != expected) {
        Throw<std::runtime_error>(fmt::format("{}: expected {}, got {}.", context, expected, actual));
    }
}

template<typename AException, typename F>
auto ExpectThrow(F&& function, std::string_view context) -> void {
    try {
        function();
    } catch (const AException&) {
        return;
    } catch (const std::exception& e) {
        Throw<std::runtime_error>(fmt::format("{}: expected an exception of the required type, got '{}'.", context, e.what()));
    }
    Throw<std::runtime_error>(fmt::format("{}: expected an exception.", context));
}

// Generate a stationary AR(1) process x_t = phi * x_{t-1} + epsilon_t
auto GenerateAR1(int size, double phi, std::mt19937_64& rng) -> Eigen::VectorXd {
    std::normal_distribution<double> normal{};
    Eigen::VectorXd sample(size);
    sample[0] = normal(rng) / std::sqrt(1 - phi * phi);
    for (int i{1}; i < size; ++i) {
        sample[i] = phi * sample[i - 1] + normal(rng);
    }
    return sample;
}

} // namespace

TestMarkovChainStatistic::TestMarkovChainStatistic() :
    Subprogram{"TestMarkovChainStatistic", "Test Mustard::MarkovChainStatistic."} {}

auto TestMarkovChainStatistic::Main(int argc, char* argv[]) const -> int {
    CLI::BasicCLI<> cli;
    Env::BasicEnv env{argc, argv, cli};

    using namespace Math;

    PrintLn("--- AR(1) autocorrelation, IACT, and ESS ---");
    constexpr auto nSample{200'000};
    constexpr std::array phi{0.5, 0.8};
    std::mt19937_64 rng{12345};
    Eigen::Matrix<double, Eigen::Dynamic, 2> sample(nSample, 2);
    sample.col(0) = GenerateAR1(nSample, phi[0], rng);
    sample.col(1) = GenerateAR1(nSample, phi[1], rng);

    const MarkovChainStatistic<2> statistic{sample};
    const auto expectedMaxLag{static_cast<int>(muc::llround(0.5 * (nSample - 1)))};
    CheckEqual(statistic.Dimension(), 2, "dimension");
    CheckEqual(statistic.SampleSize(), nSample, "sample size");
    CheckEqual(statistic.MaxLag(), expectedMaxLag, "maximum lag");

    const auto acf{statistic.AutocorrelationFunction()};
    CheckEqual(acf.rows(), statistic.MaxLag() + 1, "ACF row count");
    CheckEqual(acf.cols(), 2, "ACF column count");
    for (int i{}; i < 2; ++i) {
        const auto expectedTau{(1 + phi[i]) / (1 - phi[i])};
        CheckClose(acf(0, i), 1, 1e-12, fmt::format("ACF(0, {})", i));
        CheckClose(acf(1, i), phi[i], 0.03, fmt::format("ACF(1, {})", i));
        CheckClose(acf(2, i), phi[i] * phi[i], 0.05, fmt::format("ACF(2, {})", i));
        CheckClose(statistic.IntegratedAutocorrelation(i), expectedTau, 0.1, fmt::format("IACT({})", i));
        CheckClose(statistic.EffectiveSampleSize(i), nSample / expectedTau, 0.1, fmt::format("ESS({})", i));
        CheckEqual(statistic.AutocorrelationFunctionStatistic()[i].SampleSize(), 1, "ACF statistic sample size");
        CheckEqual(statistic.IntegratedAutocorrelationStatistic().SampleSize(), 1, "IACT statistic sample size");
        CheckEqual(statistic.EffectiveSampleSizeStatistic().SampleSize(), 1, "ESS statistic sample size");
    }

    const auto variance0{1 / (1 - phi[0] * phi[0])};
    const auto variance1{1 / (1 - phi[1] * phi[1])};
    const auto tau0{(1 + phi[0]) / (1 - phi[0])};
    const auto tau1{(1 + phi[1]) / (1 - phi[1])};
    const auto expectedTauAverage{(variance0 * tau0 + variance1 * tau1) / (variance0 + variance1)};
    CheckClose(statistic.ScalarIntegratedAutocorrelation(), expectedTauAverage, 0.1, "scalar IACT");
    CheckClose(statistic.ScalarEffectiveSampleSize(), nSample / expectedTauAverage, 0.1, "scalar ESS");
    CheckEqual(statistic.ScalarIntegratedAutocorrelationStatistic().SampleSize(), 1, "scalar IACT statistic sample size");
    CheckEqual(statistic.ScalarEffectiveSampleSizeStatistic().SampleSize(), 1, "scalar ESS statistic sample size");

    PrintLn("--- K = 1 ---");
    Eigen::Matrix<double, Eigen::Dynamic, 1> sample1(nSample, 1);
    sample1.col(0) = GenerateAR1(nSample, phi[0], rng);
    const MarkovChainStatistic<1> statistic1{sample1};
    CheckEqual(statistic1.Dimension(), 1, "K=1 dimension");
    CheckClose(statistic1.IntegratedAutocorrelation(0), tau0, 0.1, "K=1 IACT");
    CheckClose(statistic1.EffectiveSampleSize(0), nSample / tau0, 0.1, "K=1 ESS");
    CheckClose(statistic1.ScalarIntegratedAutocorrelation(), tau0, 0.1, "K=1 scalar IACT");
    CheckClose(statistic1.ScalarEffectiveSampleSize(), nSample / tau0, 0.1, "K=1 scalar ESS");
    const auto restored1{MarkovChainStatistic<1>::FromFlat(statistic1.ToFlat(), nSample, statistic1.MaxLag())};
    CheckClose(restored1.IntegratedAutocorrelation(0), statistic1.IntegratedAutocorrelation(0), 1e-12, "K=1 restored IACT");

    PrintLn("--- Merging statistics from two segments ---");
    const auto half{nSample / 2};
    const auto firstSample{sample.topRows(half).eval()};
    const auto secondSample{sample.bottomRows(half).eval()};
    const MarkovChainStatistic<2> first{firstSample};
    const MarkovChainStatistic<2> second{secondSample};
    auto merged{first + second};
    CheckEqual(merged.SampleSize(), half, "merged sample size");
    CheckEqual(merged.MaxLag(), first.MaxLag(), "merged maximum lag");
    CheckEqual(merged.AutocorrelationFunctionStatistic()[0].SampleSize(), 2, "merged ACF statistic sample size");
    CheckEqual(merged.IntegratedAutocorrelationStatistic().SampleSize(), 2, "merged IACT statistic sample size");
    CheckEqual(merged.ScalarIntegratedAutocorrelationStatistic().SampleSize(), 2, "merged scalar IACT statistic sample size");
    for (int i{}; i < 2; ++i) {
        const auto expectedTau{(1 + phi[i]) / (1 - phi[i])};
        CheckClose(merged.IntegratedAutocorrelation(i), expectedTau, 0.15, fmt::format("merged IACT({})", i));
    }
    CheckClose(merged.ScalarIntegratedAutocorrelation(), expectedTauAverage, 0.15, "merged scalar IACT");
    const auto tauEstimate{merged.ScalarIntegratedAutocorrelationStatistic().MeanEstimate()};
    const auto expectedUncertainty{std::abs(first.ScalarIntegratedAutocorrelation() - second.ScalarIntegratedAutocorrelation()) / 2};
    CheckClose(tauEstimate.Uncertainty(), expectedUncertainty, 1e-12, "merged scalar IACT uncertainty");
    const auto firstFlat{first.ToFlat()};
    const auto secondFlat{second.ToFlat()};
    const auto mergedFlat{merged.ToFlat()};
    CheckEqual(mergedFlat.first[0], firstFlat.first[0] + secondFlat.first[0], "additive flat count");
    CheckEqual(mergedFlat.second.size(), firstFlat.second.size(), "additive flat value count");
    for (std::size_t k{}; k < mergedFlat.second.size(); ++k) {
        CheckClose(mergedFlat.second[k], firstFlat.second[k] + secondFlat.second[k], 1e-12, fmt::format("additive flat value({})", k));
    }
    auto summedFlat{firstFlat};
    summedFlat.first[0] += secondFlat.first[0];
    for (std::size_t k{}; k < summedFlat.second.size(); ++k) {
        summedFlat.second[k] += secondFlat.second[k];
    }
    const auto restoredSummed{MarkovChainStatistic<2>::FromFlat(summedFlat, half, first.MaxLag())};
    CheckClose(restoredSummed.ScalarIntegratedAutocorrelation(), merged.ScalarIntegratedAutocorrelation(), 1e-12, "summed flat scalar IACT");
    CheckClose(restoredSummed.ScalarIntegratedAutocorrelationStatistic().MeanEstimate().Uncertainty(), expectedUncertainty, 1e-12, "summed flat scalar IACT uncertainty");

    PrintLn("--- Flat representation round trip ---");
    const auto flat{statistic.ToFlat()};
    const auto restored{MarkovChainStatistic<2>::FromFlat(flat, statistic.SampleSize(), statistic.MaxLag())};
    CheckEqual(restored.SampleSize(), statistic.SampleSize(), "restored sample size");
    CheckEqual(restored.MaxLag(), statistic.MaxLag(), "restored maximum lag");
    for (int i{}; i < 2; ++i) {
        CheckClose(restored.IntegratedAutocorrelation(i), statistic.IntegratedAutocorrelation(i), 1e-12, fmt::format("restored IACT({})", i));
        CheckClose(restored.EffectiveSampleSize(i), statistic.EffectiveSampleSize(i), 1e-12, fmt::format("restored ESS({})", i));
        const auto acf1{statistic.AutocorrelationFunction(i)};
        const auto acf2{restored.AutocorrelationFunction(i)};
        for (int lag{}; lag <= statistic.MaxLag(); ++lag) {
            CheckClose(acf2[lag], acf1[lag], 1e-12, fmt::format("restored ACF({}, {})", i, lag));
        }
    }
    CheckClose(restored.ScalarIntegratedAutocorrelation(), statistic.ScalarIntegratedAutocorrelation(), 1e-12, "restored scalar IACT");
    CheckClose(restored.ScalarEffectiveSampleSize(), statistic.ScalarEffectiveSampleSize(), 1e-12, "restored scalar ESS");

    StatisticDiagXD statisticFlatSource{3};
    statisticFlatSource.Fill(Eigen::Vector3d{1.0, 2.0, 3.0});
    statisticFlatSource.Fill(Eigen::Vector3d{2.0, 4.0, 6.0});
    StatisticDiagXD statisticFlatTarget{3};
    statisticFlatTarget.FromFlat(statisticFlatSource.ToFlat());
    CheckEqual(statisticFlatTarget.SampleSize(), statisticFlatSource.SampleSize(), "flat statistic sample size");
    for (int i{}; i < 3; ++i) {
        CheckClose(statisticFlatTarget.Mean(i), statisticFlatSource.Mean(i), 1e-12, fmt::format("flat statistic mean({})", i));
        CheckClose(statisticFlatTarget.Variance(i), statisticFlatSource.Variance(i), 1e-12, fmt::format("flat statistic variance({})", i));
    }

    Eigen::VectorXd estimateValue{3};
    estimateValue << 1, 2, 3;
    Eigen::DiagonalMatrix<double, Eigen::Dynamic> estimateCovariance;
    estimateCovariance.resize(3);
    estimateCovariance.diagonal() << 4, 5, 6;
    const EstimateDiagXD estimateFlatSource{estimateValue, estimateCovariance};
    EstimateDiagXD estimateFlatTarget;
    estimateFlatTarget.FromFlat(estimateFlatSource.ToFlat());
    for (int i{}; i < 3; ++i) {
        CheckClose(estimateFlatTarget.Value(i), estimateFlatSource.Value(i), 1e-12, fmt::format("flat estimate value({})", i));
        CheckClose(estimateFlatTarget.Covariance(i, i), estimateFlatSource.Covariance(i, i), 1e-12, fmt::format("flat estimate covariance({})", i));
    }

    StatisticFullXD fullStatisticSource{3};
    fullStatisticSource.Fill(Eigen::Vector3d{1.0, 2.0, 3.0});
    fullStatisticSource.Fill(Eigen::Vector3d{2.0, 4.0, 6.0}, 0.5);
    StatisticFullXD fullStatisticTarget{3};
    fullStatisticTarget.FromFlat(fullStatisticSource.ToFlat());
    for (int i{}; i < 3; ++i) {
        CheckClose(fullStatisticTarget.Mean(i), fullStatisticSource.Mean(i), 1e-12, fmt::format("flat full statistic mean({})", i));
        for (int j{}; j < 3; ++j) {
            CheckClose(fullStatisticTarget.Covariance(i, j), fullStatisticSource.Covariance(i, j), 1e-12, fmt::format("flat full statistic covariance({}, {})", i, j));
        }
    }

    StatisticDiagXD weightedStatisticSource{3};
    weightedStatisticSource.Fill(Eigen::Vector3d{1.0, 2.0, 3.0}, 0.5);
    weightedStatisticSource.Fill(Eigen::Vector3d{2.0, 4.0, 6.0}, 2.0);
    StatisticDiagXD weightedStatisticTarget{3};
    weightedStatisticTarget.FromFlat(weightedStatisticSource.ToFlat());
    for (int i{}; i < 3; ++i) {
        CheckClose(weightedStatisticTarget.Mean(i), weightedStatisticSource.Mean(i), 1e-12, fmt::format("flat weighted statistic mean({})", i));
        CheckClose(weightedStatisticTarget.Variance(i), weightedStatisticSource.Variance(i), 1e-12, fmt::format("flat weighted statistic variance({})", i));
    }

    Eigen::MatrixXd estimateFullCovariance(3, 3);
    estimateFullCovariance << 4, 0.5, 0.2,
        0.5, 5, 0.3,
        0.2, 0.3, 6;
    const EstimateFullXD estimateFullSource{estimateValue, estimateFullCovariance};
    EstimateFullXD estimateFullTarget;
    estimateFullTarget.FromFlat(estimateFullSource.ToFlat());
    for (int i{}; i < 3; ++i) {
        CheckClose(estimateFullTarget.Value(i), estimateFullSource.Value(i), 1e-12, fmt::format("flat full estimate value({})", i));
        for (int j{}; j < 3; ++j) {
            CheckClose(estimateFullTarget.Covariance(i, j), estimateFullSource.Covariance(i, j), 1e-12, fmt::format("flat full estimate covariance({}, {})", i, j));
        }
    }

    PrintLn("--- Error handling ---");
    ExpectThrow<std::invalid_argument>([&] {
        [[maybe_unused]] const MarkovChainStatistic<2> tooFew{Eigen::Matrix<double, Eigen::Dynamic, 2>::Zero(50, 2)};
    },
                                       "too few samples");
    ExpectThrow<std::invalid_argument>([&] {
        [[maybe_unused]] const MarkovChainStatistic<2> badRatio{sample, 1.5};
    },
                                       "invalid maximum lag ratio");
    ExpectThrow<std::runtime_error>([&] {
        [[maybe_unused]] const MarkovChainStatistic<2> constant{Eigen::Matrix<double, Eigen::Dynamic, 2>::Zero(1000, 2)};
    },
                                    "zero variance");
    ExpectThrow<std::invalid_argument>([&] {
        auto mismatched{first};
        mismatched += MarkovChainStatistic<2>{sample.topRows(half / 2).eval()};
    },
                                       "mismatched sample size");

    ExpectThrow<std::invalid_argument>([] {
        StatisticDiagXD target{3};
        target.FromFlat({
            {1, 3},
            {1.0, 2.0, 3.0}
        });
    },
                                       "short flat statistic");
    ExpectThrow<std::invalid_argument>([&] {
        [[maybe_unused]] const auto bad{MarkovChainStatistic<2>::FromFlat(flat, statistic.SampleSize(), -1)};
    },
                                       "invalid flat layout");

    PrintLn("All TestMarkovChainStatistic tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
