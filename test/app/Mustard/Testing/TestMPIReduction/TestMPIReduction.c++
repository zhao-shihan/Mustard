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
#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Math/Estimate.h++"
#include "Mustard/Math/Statistic.h++"
#include "Mustard/Testing/TestMPIReduction/TestMPIReduction.h++"

#include "mp1/mp1.hpp"

#include "fmt/format.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <stdexcept>
#include <string_view>

namespace Mustard::Testing {

namespace {

namespace TestMPIReductionSection {

// Every rank computes the same expected values locally, so only the rounding of
// the floating-point additions performed by MPI can differ from them.
constexpr auto relativeTolerance{1.e-12};

// Samples fed by each rank. The values differ within a rank and shift with the
// rank; the weights 1, 2, 0.5 and 1.5 make the reduced n, w and w2 differ from
// each other and from one.
constexpr auto nSamplePerRank{4};
constexpr auto rankStep{0.5};

constexpr auto SampleValue(int rank, int i) -> double {
    constexpr std::array value{0.5, -1.25, 2.0, 0.75};
    return value[i] + rankStep * rank;
}

constexpr auto SampleWeight(int i) -> double {
    constexpr std::array weight{1.0, 2.0, 0.5, 1.5};
    return weight[i];
}

// The estimate contributed by each rank depends on the rank as well, so that a
// missing or duplicated contribution cannot cancel out.
constexpr auto EstimateValue(int rank) -> double { return 0.5 * (rank + 1); }
constexpr auto EstimateUncertainty(int rank) -> double { return 0.1 * (rank + 1); }

struct StatisticExpectation {
    long long sampleCount;
    double weightSum;
    double weightSquareSum;
    double mean;
    double m2;
    double variance;
};

// The result of pooling the samples of all ranks. For a sample x_i with weight w_i,
// Statistic maintains n = number of samples, W = sum w_i, W2 = sum w_i^2,
// M = sum w_i x_i and M2 = sum w_i (x_i - mean)^2, from which
// Mean = M / W and Variance = W / (W^2 - W2) * M2.
auto MakeStatisticExpectation(int worldSize) -> StatisticExpectation {
    auto sampleCount{0LL};
    auto weightSum{0.0};
    auto weightSquareSum{0.0};
    auto weightedSum{0.0};
    for (auto rank{0}; rank < worldSize; ++rank) {
        for (auto i{0}; i < nSamplePerRank; ++i) {
            const auto weight{SampleWeight(i)};
            ++sampleCount;
            weightSum += weight;
            weightSquareSum += weight * weight;
            weightedSum += weight * SampleValue(rank, i);
        }
    }
    const auto mean{weightedSum / weightSum};
    auto m2{0.0};
    for (auto rank{0}; rank < worldSize; ++rank) {
        for (auto i{0}; i < nSamplePerRank; ++i) {
            const auto delta{SampleValue(rank, i) - mean};
            m2 += SampleWeight(i) * delta * delta;
        }
    }
    return {sampleCount, weightSum, weightSquareSum, mean, m2, weightSum * m2 / (weightSum * weightSum - weightSquareSum)};
}

auto CheckClose(double actual, double expected, std::string_view context) -> void {
    if (const auto tolerance{relativeTolerance * std::max(1.0, std::abs(expected))};
        not(std::abs(actual - expected) <= tolerance)) {
        Throw<std::runtime_error>(fmt::format("{}: expected {}, got {}.", context, expected, actual));
    }
}

auto CheckEqual(auto actual, auto expected, std::string_view context) -> void {
    if (actual != expected) {
        Throw<std::runtime_error>(fmt::format("{}: expected {}, got {}.", context, expected, actual));
    }
}

} // namespace TestMPIReductionSection

} // namespace

TestMPIReduction::TestMPIReduction() :
    Subprogram{"TestMPIReduction", "Test MPI reduction of reflected Mustard POD types."} {}

auto TestMPIReduction::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::MPIEnv env{argc, argv, cli};

    using namespace TestMPIReductionSection;

    const auto& worldComm{mp1::comm_world()};
    const auto worldSize{worldComm.size()};
    const auto rank{worldComm.rank()};

    // StatisticPOD: each rank feeds several weighted samples, and the reflected
    // operator+ merges the local statistics, including the between-rank M2 term.
    Statistic<1> local;
    for (auto i{0}; i < nSamplePerRank; ++i) {
        local.Fill(SampleValue(rank, i), SampleWeight(i));
    }

    auto pod{local.ToPOD()};
    mp1::allreduce(worldComm, std::plus{}, mp1::in_place, pod);

    const auto expected{MakeStatisticExpectation(worldSize)};
    const Statistic<1> global{pod};
    CheckEqual(global.SampleSize(), expected.sampleCount, "The reduced StatisticPOD has the wrong sample count");
    CheckClose(global.WeightSum(), expected.weightSum, "The reduced StatisticPOD has the wrong sum of weights");
    CheckClose(global.WeightSquareSum(), expected.weightSquareSum, "The reduced StatisticPOD has the wrong sum of squared weights");
    CheckClose(global.Mean(), expected.mean, "The reduced StatisticPOD has the wrong mean");
    CheckClose(pod.m2[0], expected.m2, "The reduced StatisticPOD has the wrong M2");
    CheckClose(global.Variance(), expected.variance, "The reduced StatisticPOD has the wrong variance");

    // EstimatePOD: Estimate<1>(value, variance) takes the variance, so the known
    // uncertainties are squared. The reflected operator+ adds the values and the
    // covariance matrices of the independent estimates.
    const auto localEstimateValue{EstimateValue(rank)};
    const auto localEstimateVariance{EstimateUncertainty(rank) * EstimateUncertainty(rank)};
    const Estimate<1> localEstimate{localEstimateValue, localEstimateVariance};

    auto estimatePOD{localEstimate.ToPOD()};
    mp1::allreduce(worldComm, std::plus{}, mp1::in_place, estimatePOD);

    auto expectedEstimateValue{0.0};
    auto expectedEstimateVariance{0.0};
    for (auto r{0}; r < worldSize; ++r) {
        expectedEstimateValue += EstimateValue(r);
        expectedEstimateVariance += EstimateUncertainty(r) * EstimateUncertainty(r);
    }
    const Estimate<1> globalEstimate{estimatePOD};
    CheckClose(globalEstimate.Value(), expectedEstimateValue, "The reduced EstimatePOD has the wrong value");
    CheckClose(globalEstimate.Variance(), expectedEstimateVariance, "The reduced EstimatePOD has the wrong variance");
    CheckClose(globalEstimate.Uncertainty(), std::sqrt(expectedEstimateVariance), "The reduced EstimatePOD has the wrong uncertainty");

    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
