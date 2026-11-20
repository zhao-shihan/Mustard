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

#pragma once

#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Math/Statistic.h++"

#include "Eigen/Core"
#include "unsupported/Eigen/FFT"

#include "muc/math"

#include "fmt/format.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace Mustard::inline Math {

/// @brief Statistics of a Markov chain sample.
///
/// Computes the per-component autocorrelation function (ACF), the integrated
/// autocorrelation time (IACT) with Geyer's initial-positive/monotone sequence
/// estimator (C. J. Geyer, Statistical Science 7 (1992) 473,
/// doi:10.1214/ss/1177011137), and the effective sample size (ESS). The
/// estimator is truncated at `MaxLag()`; for slowly mixing chains the estimate
/// is therefore biased low.
///
/// A scalar IACT, defined as the trace ratio
/// @f$\tau_{\mathrm{tr}} = \operatorname{tr}\Sigma / \operatorname{tr}\Lambda@f$
/// (the variance-weighted average of the per-component IACTs), is provided for
/// thinning and reporting.
///
/// A statistic represents one chain of @f$N@f$ samples. Several statistics with
/// the same @f$N@f$ and maximum lag can be merged by `operator+=`, so that the
/// mean and the standard error of each quantity can be estimated from
/// replicates. The standard error requires at least two merged chains.
///
/// @tparam K Dimension of the chain state (compile-time constant; `Eigen::Dynamic` is not supported)
template<int K>
    requires(impl::GoodStatisticDimension<K>::value and K != Eigen::Dynamic)
class MarkovChainStatistic {
private:
    struct EmptyTag {};

public:
    /// @brief Construct from a sample of a Markov chain.
    /// @param sample Sample matrix, row = time-ordered sample, column = component
    /// @param maxLagRatio Maximum lag as a fraction of the sample size minus one
    /// @throws std::invalid_argument if the sample size is less than 100 or @p maxLagRatio is invalid
    /// @throws std::runtime_error if a component has non-normal variance or an invalid IACT
    MarkovChainStatistic(const Eigen::Matrix<double, Eigen::Dynamic, K>& sample,
                         double maxLagRatio = 0.5);

    /// @brief Merge another statistic of the same chain length and maximum lag.
    /// @param other Another statistic from a replicate
    /// @return Reference to `*this`
    /// @throws std::invalid_argument if the sample sizes or maximum lags differ
    auto operator+=(const MarkovChainStatistic& other) -> MarkovChainStatistic&;

    /// @brief Dimension of the chain state.
    auto Dimension() const -> int { return K; }
    /// @brief Number of samples in one chain.
    auto SampleSize() const -> int { return fSampleSize; }
    /// @brief Maximum lag of the autocorrelation function.
    auto MaxLag() const -> int { return fMaxLag; }

    /// @brief Mean autocorrelation function of component @p i over merged chains.
    /// @param i Component index (0-based)
    /// @return ACF at lags 0 to `MaxLag()` (convenience copy)
    auto AutocorrelationFunction(int i) const -> Eigen::VectorXd { return fRho[i].Mean(); }
    /// @brief Mean autocorrelation function of all components over merged chains.
    /// @return ACF matrix, row = lag, column = component (convenience copy)
    auto AutocorrelationFunction() const -> Eigen::MatrixXd;
    /// @brief Autocorrelation function statistic of all components.
    auto AutocorrelationFunctionStatistic() const -> const auto& { return fRho; }

    /// @brief Mean integrated autocorrelation time of component @p i over merged chains.
    /// @param i Component index (0-based)
    /// @return Mean IACT of component @p i
    auto IntegratedAutocorrelation(int i) const -> double { return fTau.Mean(i); }
    /// @brief Integrated autocorrelation time statistic of all components.
    auto IntegratedAutocorrelationStatistic() const -> const auto& { return fTau; }
    /// @brief Mean scalar integrated autocorrelation time over merged chains,
    ///        @f$\operatorname{tr}\Sigma / \operatorname{tr}\Lambda@f$ per chain.
    /// @return Mean scalar IACT
    auto ScalarIntegratedAutocorrelation() const -> double { return fScalarTau.Mean(); }
    /// @brief Scalar integrated autocorrelation time statistic.
    auto ScalarIntegratedAutocorrelationStatistic() const -> const auto& { return fScalarTau; }

    /// @brief Mean effective sample size of component @p i over merged chains,
    ///        @f$N / \tau_i@f$ per chain.
    /// @param i Component index (0-based)
    /// @return Mean ESS of component @p i
    auto EffectiveSampleSize(int i) const -> double { return fESS.Mean(i); }
    /// @brief Effective sample size statistic of all components.
    auto EffectiveSampleSizeStatistic() const -> const auto& { return fESS; }
    /// @brief Mean scalar effective sample size over merged chains,
    ///        @f$N / \tau_{\mathrm{tr}}@f$ per chain.
    /// @return Mean scalar ESS
    auto ScalarEffectiveSampleSize() const -> double { return fScalarESS.Mean(); }
    /// @brief Scalar effective sample size statistic.
    auto ScalarEffectiveSampleSizeStatistic() const -> const auto& { return fScalarESS; }

    /// @brief Convert the statistic state to a flat STL representation.
    ///
    /// The counts are `{number of chains}`, and the values are the concatenated
    /// additive sufficient statistics of every internal statistic. Both the counts
    /// and the values must be reduced element-wise (e.g., by an MPI all-reduce)
    /// before being restored with `FromFlat()`.
    ///
    /// @return Pair of counts and values
    /// @see FromFlat
    auto ToFlat() const -> std::pair<std::vector<long long>, std::vector<double>>;
    /// @brief Restore the statistic state from a flat STL representation.
    /// @param data Pair of counts and values as described in `ToFlat()`
    /// @param sampleSize Number of samples in one chain
    /// @param maxLag Maximum lag of the autocorrelation function
    /// @return The restored statistic
    /// @throws std::invalid_argument if the counts or values are inconsistent
    /// @see ToFlat
    static auto FromFlat(const std::pair<std::vector<long long>, std::vector<double>>& data,
                         int sampleSize, int maxLag) -> MarkovChainStatistic;

private:
    /// @brief Construct an empty statistic with the given chain length and maximum lag.
    /// @param sampleSize Number of samples in one chain
    /// @param maxLag Maximum lag of the autocorrelation function
    MarkovChainStatistic(int sampleSize, int maxLag, EmptyTag);

    /// @brief Compute and validate the maximum lag.
    static auto ComputeMaxLag(int sampleSize, double maxLagRatio) -> int;

private:
    std::array<StatisticDiagXD, K> fRho; ///< Per-component ACF statistics
    StatisticDiag<K> fTau;               ///< Per-component IACT statistics
    StatisticDiag<K> fESS;               ///< Per-component ESS statistics
    Statistic1D fScalarTau;              ///< Scalar IACT statistics
    Statistic1D fScalarESS;              ///< Scalar ESS statistics
    int fSampleSize;                     ///< Number of samples in one chain
    int fMaxLag;                         ///< Maximum lag of the autocorrelation function
};

/// @brief Merge two statistics of the same chain length and maximum lag.
/// @param lhs Left-hand statistic
/// @param rhs Right-hand statistic
/// @return The merged statistic
/// @see MarkovChainStatistic::operator+=
template<int K>
    requires(impl::GoodStatisticDimension<K>::value and K != Eigen::Dynamic)
[[nodiscard]] auto operator+(MarkovChainStatistic<K> lhs, const MarkovChainStatistic<K>& rhs) -> MarkovChainStatistic<K>;

} // namespace Mustard::inline Math

#include "Mustard/Math/MarkovChainStatistic.inl"
