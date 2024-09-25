// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
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

namespace Mustard::Math {

constexpr Statistic<1>::Statistic() :
    fSumWX{},
    fSumWX2{},
    fSumWX3{},
    fSumWX4{},
    fSumW{} {}

template<std::ranges::input_range S>
    requires std::convertible_to<std::ranges::range_value_t<S>, double>
constexpr Statistic<1>::Statistic(const S& sample, double weight) :
    Statistic{} {
    Fill(sample, weight);
}

template<std::ranges::input_range S, std::ranges::input_range W>
    requires std::convertible_to<std::ranges::range_value_t<S>, double> and std::convertible_to<std::ranges::range_value_t<W>, double>
constexpr Statistic<1>::Statistic(const S& sample, const W& weight) :
    Statistic{} {
    Fill(sample, weight);
}

constexpr auto Statistic<1>::Fill(double sample, double weight) -> void {
    fSumWX += weight * sample;
    fSumWX2 += weight * muc::pow<2>(sample);
    fSumWX3 += weight * muc::pow<3>(sample);
    fSumWX4 += weight * muc::pow<4>(sample);
    fSumW += weight;
}

template<std::ranges::input_range S>
    requires std::convertible_to<std::ranges::range_value_t<S>, double>
constexpr auto Statistic<1>::Fill(const S& sample, double weight) -> void {
    for (auto&& s : sample) {
        Fill(s, weight);
    }
}

template<std::ranges::input_range S, std::ranges::input_range W>
    requires std::convertible_to<std::ranges::range_value_t<S>, double> and std::convertible_to<std::ranges::range_value_t<W>, double>
constexpr auto Statistic<1>::Fill(const S& sample, const W& weight) -> void {
    if (std::ranges::size(sample) > std::ranges::size(weight)) {
        throw std::invalid_argument{PrettyException("Size of sample > size of weight")};
    }
    auto s = std::ranges::begin(sample);
    auto w = std::ranges::begin(weight);
    const auto sEnd = std::ranges::end(sample);
    while (s != sEnd) {
        Fill(*s++, *w++);
    }
}

template<int K>
    requires(0 <= K and K <= 4)
constexpr auto Statistic<1>::Moment() const -> double {
    if constexpr (K == 0) { return 1; }
    if constexpr (K == 1) { return fSumWX / fSumW; }
    if constexpr (K == 2) { return fSumWX2 / fSumW; }
    if constexpr (K == 3) { return fSumWX3 / fSumW; }
    if constexpr (K == 4) { return fSumWX4 / fSumW; }
}

template<int K>
    requires(0 <= K and K <= 4)
constexpr auto Statistic<1>::CentralMoment() const -> double {
    if constexpr (K == 0) { return 1; }
    if constexpr (K == 1) { return 0; }
    if constexpr (K == 2) { return Moment<2>() - muc::pow<2>(Moment<1>()); }
    if constexpr (K == 3) { return Math::Polynomial({Moment<3>(), -3 * Moment<2>(), 0, 2}, Moment<1>()); }
    if constexpr (K == 4) { return Math::Polynomial({Moment<4>(), -4 * Moment<3>(), 6 * Moment<2>(), 0, -3}, Moment<1>()); }
}

template<int N>
    requires(N > 0)
Statistic<N>::Statistic() :
    fSumWX{},
    fSumWXX{},
    fSumWX3{},
    fSumWX4{},
    fSumW{} {}

template<int N>
    requires(N > 0)
template<std::ranges::input_range S>
    requires Concept::InputVectorAny<std::ranges::range_value_t<S>, N>
Statistic<N>::Statistic(const S& sample, double weight) :
    Statistic{} {
    Fill(sample, weight);
}

template<int N>
    requires(N > 0)
template<std::ranges::input_range S, std::ranges::input_range W>
    requires Concept::InputVectorAny<std::ranges::range_value_t<S>, N> and std::convertible_to<std::ranges::range_value_t<W>, double>
Statistic<N>::Statistic(const S& sample, const W& weight) :
    Statistic{} {
    Fill(sample, weight);
}

template<int N>
    requires(N > 0)
template<Concept::InputVectorAny<N> T>
auto Statistic<N>::Fill(const T& sample, double weight) -> void {
    const auto x{VectorCast<Eigen::Vector<double, N>>(sample)};
    Eigen::Vector<double, N> wx{weight * x};
    fSumWX += wx;
    fSumWXX += wx * x.transpose();
    wx = wx.cwiseProduct(x).cwiseProduct(x).eval();
    fSumWX3 += wx;
    wx = wx.cwiseProduct(x).eval();
    fSumWX4 += wx;
    fSumW += weight;
}

template<int N>
    requires(N > 0)
template<std::ranges::input_range S>
    requires Concept::InputVectorAny<std::ranges::range_value_t<S>, N>
auto Statistic<N>::Fill(const S& sample, double weight) -> void {
    for (auto&& s : sample) {
        Fill(s, weight);
    }
}

template<int N>
    requires(N > 0)
template<std::ranges::input_range S, std::ranges::input_range W>
    requires Concept::InputVectorAny<std::ranges::range_value_t<S>, N> and std::convertible_to<std::ranges::range_value_t<W>, double>
auto Statistic<N>::Fill(const S& sample, const W& weight) -> void {
    if (std::ranges::size(sample) > std::ranges::size(weight)) {
        throw std::invalid_argument{PrettyException("Size of sample > size of weight")};
    }
    auto s = std::ranges::begin(sample);
    auto w = std::ranges::begin(weight);
    const auto sEnd = std::ranges::end(sample);
    while (s != sEnd) {
        Fill(*s++, *w++);
    }
}

template<int N>
    requires(N > 0)
template<int K>
    requires(0 <= K and K <= 4)
auto Statistic<N>::Moment(int i) const -> double {
    if constexpr (K == 0) { return 1; }
    if constexpr (K == 1) { return fSumWX[i] / fSumW; }
    if constexpr (K == 2) { return fSumWXX(i, i) / fSumW; }
    if constexpr (K == 3) { return fSumWX3[i] / fSumW; }
    if constexpr (K == 4) { return fSumWX4[i] / fSumW; }
}

template<int N>
    requires(N > 0)
template<int K>
    requires(0 <= K and K <= 4)
auto Statistic<N>::Moment() const -> Eigen::Vector<double, N> {
    if constexpr (K == 0) { return Eigen::Vector<double, N>::Constant(1); }
    if constexpr (K == 1) { return fSumWX / fSumW; }
    if constexpr (K == 2) { return fSumWXX.diagonal() / fSumW; }
    if constexpr (K == 3) { return fSumWX3 / fSumW; }
    if constexpr (K == 4) { return fSumWX4 / fSumW; }
}

template<int N>
    requires(N > 0)
template<int K>
    requires(0 <= K and K <= 4)
auto Statistic<N>::CentralMoment(int i) const -> double {
    if constexpr (K == 0) { return 1; }
    if constexpr (K == 1) { return 0; }
    if constexpr (K == 2) { return Moment<2>(i) - muc::pow<2>(Moment<1>(i)); }
    if constexpr (K == 3) { return Math::Polynomial({Moment<3>(i), -3 * Moment<2>(i), 0, 2}, Moment<1>(i)); }
    if constexpr (K == 4) { return Math::Polynomial({Moment<4>(i), -4 * Moment<3>(i), 6 * Moment<2>(i), 0, -3}, Moment<1>(i)); }
}

template<int N>
    requires(N > 0)
template<int K>
    requires(0 <= K and K <= 4)
auto Statistic<N>::CentralMoment() const -> Eigen::Vector<double, N> {
    if constexpr (K == 0) {
        return Eigen::Vector<double, N>::Constant(1);
    }
    if constexpr (K == 1) {
        return Eigen::Vector<double, N>::Zeros();
    }
    if constexpr (K == 2) {
        const auto m1 = Moment<1>();
        return Moment<2>() - m1.cwiseProduct(m1);
    }
    if constexpr (K == 3) {
        const auto m1 = Moment<1>();
        return Moment<3>() + m1.cwiseProduct(-3 * Moment<2>() + 2 * m1.cwiseProduct(m1));
    }
    if constexpr (K == 4) {
        const auto m1 = Moment<1>();
        return Moment<4>() + m1.cwiseProduct(-4 * Moment<3>() + m1.cwiseProduct(6 * Moment<2>() - 3 * m1.cwiseProduct(m1)));
    }
}

template<int N>
    requires(N > 0)
auto Statistic<N>::Mixed2ndCentralMoment() const -> Eigen::Matrix<double, N, N> {
    const auto m1 = Moment<1>();
    return Mixed2ndMoment() - m1 * m1.transpose();
}

template<int N>
    requires(N > 0)
auto Statistic<N>::Skewness() const -> Eigen::Vector<double, N> {
    const auto stdDev = StdDev();
    return CentralMoment<3>() / stdDev.cwiseProduct(stdDev).cwiseProduct(stdDev);
}

template<int N>
    requires(N > 0)
auto Statistic<N>::Kurtosis() const -> Eigen::Vector<double, N> {
    const auto variance = Variance();
    return CentralMoment<4>() / variance.cwiseProduct(variance);
}

} // namespace Mustard::Math
