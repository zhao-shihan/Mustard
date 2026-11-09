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

namespace Mustard::inline Math {

template<int K>
    requires(impl::GoodStatisticDimension<K>::value and K != Eigen::Dynamic)
MarkovChainStatistic<K>::MarkovChainStatistic(const Eigen::Matrix<double, Eigen::Dynamic, K>& sample, double maxLagRatio) :
    fRho{},
    fTau{},
    fESS{},
    fScalarTau{},
    fScalarESS{},
    fSampleSize{static_cast<int>(sample.rows())},
    fMaxLag{ComputeMaxLag(static_cast<int>(sample.rows()), maxLagRatio)} {
    for (auto&& rho : fRho) {
        rho = StatisticDiagXD{fMaxLag + 1};
    }

    // Estimate autocorrelation function, integrated autocorrelation time, and effective sample size for each dimension
    const auto sampleMean{sample.colwise().mean().eval()};
    Eigen::FFT<double> fft{{}, Eigen::FFT<double>::HalfSpectrum};
    Eigen::VectorXd signal(2 * fSampleSize);
    Eigen::VectorXcd spectrum(fSampleSize + 1);
    Eigen::Vector<double, K> tau;
    Eigen::Vector<double, K> ess;
    Eigen::VectorXd rho(fMaxLag + 1);
    double varianceSum{};
    double weightedTau{};
    for (int i{}; i < K; ++i) {
        // Compute autocorrelation via Wiener-Khinchin theorem
        signal.head(fSampleSize) = (sample.col(i).array() - sampleMean[i]).matrix();
        signal.tail(fSampleSize).setZero();
        fft.fwd(spectrum, signal);       // Spec(x) = FFT(x)
        spectrum = spectrum.cwiseAbs2(); // Power(x) = |Spec(x)|^2 = |FFT(x)|^2
        fft.inv(signal, spectrum);       // ACF(x) ∝ IFFT(Power(x))
        // Check zero-lag autocovariance
        if (not std::isnormal(signal[0])) {
            Throw<std::runtime_error>(fmt::format("Variance for dimension {} is not normal ({}).", i, signal[0]));
        }
        rho = signal.head(fMaxLag + 1) / signal[0];
        fRho[i].Fill(rho);

        // Estimate integrated autocorrelation time with Geyer's initial-positive/monotone sequence estimator
        double sumPair{};
        double minPair{std::numeric_limits<double>::infinity()};
        for (int k{}; 2 * k + 1 <= fMaxLag; ++k) {
            const auto pair{rho[2 * k] + rho[2 * k + 1]};
            minPair = std::min(minPair, pair);
            if (minPair <= 0) {
                break;
            }
            sumPair += minPair;
        }
        const auto tauI{2 * sumPair - 1};
        if (not std::isfinite(tauI) or tauI <= 0) {
            Throw<std::runtime_error>(fmt::format("Invalid integrated autocorrelation time for dimension {} ({}).", i, tauI));
        }
        tau[i] = tauI;
        ess[i] = fSampleSize / tauI;

        // Variance of this dimension, used for the variance-weighted scalar IACT
        const auto variance{(sample.col(i).array() - sampleMean[i]).square().sum() / (fSampleSize - 1)};
        if (not std::isfinite(variance) or variance <= 0) {
            Throw<std::runtime_error>(fmt::format("Variance for dimension {} is not positive ({}).", i, variance));
        }
        varianceSum += variance;
        weightedTau += variance * tauI;
    }

    if (not std::isfinite(varianceSum) or varianceSum <= 0) {
        Throw<std::runtime_error>(fmt::format("Sum of variances is not positive ({}).", varianceSum));
    }
    const auto scalarTau{weightedTau / varianceSum};
    const auto scalarEss{fSampleSize / scalarTau};
    fTau.Fill(tau);
    fESS.Fill(ess);
    fScalarTau.Fill(scalarTau);
    fScalarESS.Fill(scalarEss);
}

template<int K>
    requires(impl::GoodStatisticDimension<K>::value and K != Eigen::Dynamic)
MarkovChainStatistic<K>::MarkovChainStatistic(int sampleSize, int maxLag, EmptyTag) :
    fRho{},
    fTau{},
    fESS{},
    fScalarTau{},
    fScalarESS{},
    fSampleSize{sampleSize},
    fMaxLag{maxLag} {
    for (auto&& rho : fRho) {
        rho = StatisticDiagXD{maxLag + 1};
    }
}

template<int K>
    requires(impl::GoodStatisticDimension<K>::value and K != Eigen::Dynamic)
auto MarkovChainStatistic<K>::ComputeMaxLag(int sampleSize, double maxLagRatio) -> int {
    if (sampleSize < 100) {
        Throw<std::invalid_argument>(fmt::format("Too few samples ({}), at least 100 required.", sampleSize));
    }
    const auto infMaxLagRatio{1.5 / (sampleSize - 1)};
    if (not std::isfinite(maxLagRatio) or maxLagRatio <= infMaxLagRatio or 1 < maxLagRatio) {
        Throw<std::invalid_argument>(fmt::format("Invalid max lag ratio ({}), must be in ({}, 1].", maxLagRatio, infMaxLagRatio));
    }
    const auto maxLag{static_cast<int>(muc::llround(maxLagRatio * (sampleSize - 1)))};
    if (maxLag < 2) {
        Throw<std::invalid_argument>(fmt::format("Maximum lag is too small ({}), at least 2 required.", maxLag));
    }
    return maxLag;
}

template<int K>
    requires(impl::GoodStatisticDimension<K>::value and K != Eigen::Dynamic)
auto MarkovChainStatistic<K>::operator+=(const MarkovChainStatistic& other) -> MarkovChainStatistic& {
    if (fSampleSize != other.fSampleSize or fMaxLag != other.fMaxLag) {
        Throw<std::invalid_argument>(fmt::format("Cannot merge Markov chain statistics with different sample sizes or maximum lags "
                                                 "(self: N={}, maxLag={}; other: N={}, maxLag={}).",
                                                 fSampleSize, fMaxLag, other.fSampleSize, other.fMaxLag));
    }
    for (int i{}; i < K; ++i) {
        fRho[i] += other.fRho[i];
    }
    fTau += other.fTau;
    fESS += other.fESS;
    fScalarTau += other.fScalarTau;
    fScalarESS += other.fScalarESS;
    return *this;
}

template<int K>
    requires(impl::GoodStatisticDimension<K>::value and K != Eigen::Dynamic)
auto MarkovChainStatistic<K>::AutocorrelationFunction() const -> Eigen::MatrixXd {
    Eigen::MatrixXd acf(fMaxLag + 1, K);
    for (int i{}; i < K; ++i) {
        acf.col(i) = fRho[i].Mean();
    }
    return acf;
}

template<int K>
    requires(impl::GoodStatisticDimension<K>::value and K != Eigen::Dynamic)
auto MarkovChainStatistic<K>::ToFlat() const -> std::pair<std::vector<long long>, std::vector<double>> {
    const auto n{fTau.SampleSize()};
    if (n <= 0) {
        Throw<std::logic_error>("Cannot flatten a Markov chain statistic without any chain.");
    }
    const auto consistent{std::ranges::all_of(fRho, [n](const auto& rho) { return rho.SampleSize() == n; }) and
                          fESS.SampleSize() == n and
                          fScalarTau.SampleSize() == n and
                          fScalarESS.SampleSize() == n};
    if (not consistent) {
        Throw<std::logic_error>("Internal Markov chain statistics have inconsistent chain counts.");
    }
    std::vector<long long> flatCount{n};
    std::vector<double> flatValue;
    const auto appendFlatValue{[&flatValue](const auto& statistic) {
        auto flat{statistic.ToFlat()};
        flatValue.insert(flatValue.end(), flat.second.begin(), flat.second.end());
    }};
    for (auto&& rho : fRho) {
        appendFlatValue(rho);
    }
    appendFlatValue(fTau);
    appendFlatValue(fESS);
    appendFlatValue(fScalarTau);
    appendFlatValue(fScalarESS);
    return {std::move(flatCount), std::move(flatValue)};
}

template<int K>
    requires(impl::GoodStatisticDimension<K>::value and K != Eigen::Dynamic)
auto MarkovChainStatistic<K>::FromFlat(const std::pair<std::vector<long long>, std::vector<double>>& data,
                                       int sampleSize, int maxLag) -> MarkovChainStatistic {
    if (data.first.size() != 1) {
        Throw<std::invalid_argument>(fmt::format("Flat Markov chain statistic counts must contain 1 element, got {}.", data.first.size()));
    }
    if (sampleSize < 100 or maxLag < 2 or sampleSize - 1 < maxLag) {
        Throw<std::invalid_argument>(fmt::format("Invalid Markov chain statistic layout (N={}, maxLag={}).", sampleSize, maxLag));
    }
    const auto n{data.first[0]};
    if (n < 1) {
        Throw<std::invalid_argument>(fmt::format("Flat Markov chain statistic must contain at least one chain, got {}.", n));
    }
    const auto expectedSize{static_cast<std::size_t>(K) * (2 * (maxLag + 1) + 2) + 2 * (2 + 2 * K) + 8};
    if (data.second.size() != expectedSize) {
        Throw<std::invalid_argument>(fmt::format("Flat Markov chain statistic values must contain {} elements, got {}.",
                                                 expectedSize, data.second.size()));
    }
    MarkovChainStatistic result{sampleSize, maxLag, EmptyTag{}};
    const auto& flatValue{data.second};
    auto offset{std::size_t{}};
    const auto read{[&](int dim) {
        const auto length{2 + 2 * dim};
        if (offset + length > flatValue.size()) {
            Throw<std::invalid_argument>(fmt::format("Flat Markov chain statistic values are too short (need at least {}, got {}).",
                                                     offset + length, flatValue.size()));
        }
        std::vector<long long> flatCount{n, dim};
        std::vector<double> segment(flatValue.begin() + offset, flatValue.begin() + offset + length);
        if (segment[0] != static_cast<double>(n) or segment[1] != static_cast<double>(n)) {
            Throw<std::invalid_argument>("Flat Markov chain statistic block has inconsistent weights.");
        }
        offset += length;
        return std::pair{std::move(flatCount), std::move(segment)};
    }};
    for (auto&& rho : result.fRho) {
        rho.FromFlat(read(maxLag + 1));
    }
    result.fTau.FromFlat(read(K));
    result.fESS.FromFlat(read(K));
    result.fScalarTau.FromFlat(read(1));
    result.fScalarESS.FromFlat(read(1));
    if (offset != flatValue.size()) {
        Throw<std::invalid_argument>(fmt::format("Flat Markov chain statistic values have {} extra elements.", flatValue.size() - offset));
    }
    return result;
}

template<int K>
    requires(impl::GoodStatisticDimension<K>::value and K != Eigen::Dynamic)
auto operator+(MarkovChainStatistic<K> lhs, const MarkovChainStatistic<K>& rhs) -> MarkovChainStatistic<K> {
    lhs += rhs;
    return lhs;
}

} // namespace Mustard::inline Math
