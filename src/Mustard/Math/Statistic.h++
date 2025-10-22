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

#pragma once

#include "Mustard/Concept/InputVector.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Utility/VectorCast.h++"

#include "Eigen/Core"

#include "muc/math"
#include "muc/numeric"

#include <cmath>
#include <concepts>
#include <initializer_list>
#include <ranges>
#include <stdexcept>

namespace Mustard::Math {

template<int N>
    requires(N > 0)
class Statistic;

template<>
class Statistic<1> {
public:
    constexpr Statistic();
    template<std::ranges::input_range S = std::initializer_list<double>>
        requires std::convertible_to<std::ranges::range_value_t<S>, double>
    constexpr explicit Statistic(const S& sample, double weight = 1);
    template<std::ranges::input_range S = std::initializer_list<double>, std::ranges::input_range W = std::initializer_list<double>>
        requires std::convertible_to<std::ranges::range_value_t<S>, double> and std::convertible_to<std::ranges::range_value_t<W>, double>
    constexpr Statistic(const S& sample, const W& weight);

    constexpr auto Fill(double sample, double weight = 1) -> void;
    template<std::ranges::input_range S = std::initializer_list<double>>
        requires std::convertible_to<std::ranges::range_value_t<S>, double>
    constexpr auto Fill(const S& sample, double weight = 1) -> void;
    template<std::ranges::input_range S = std::initializer_list<double>, std::ranges::input_range W = std::initializer_list<double>>
        requires std::convertible_to<std::ranges::range_value_t<S>, double> and std::convertible_to<std::ranges::range_value_t<W>, double>
    constexpr auto Fill(const S& sample, const W& weight) -> void;

    constexpr auto Sum() const -> const auto& { return fSumWX; }
    constexpr auto SumProduct() const -> const auto& { return fSumWX2; }
    constexpr auto SumCubic() const -> const auto& { return fSumWX3; }
    constexpr auto SumQuartic() const -> const auto& { return fSumWX4; }

    constexpr auto WeightSum() const -> const auto& { return fSumW; }

    template<int K>
        requires(0 <= K and K <= 4)
    constexpr auto Moment() const -> double;
    template<int K>
        requires(0 <= K and K <= 4)
    constexpr auto CentralMoment() const -> double;

    constexpr auto Mean() const -> auto { return Moment<1>(); }
    constexpr auto MeanSquare() const -> auto { return Moment<2>(); }
    constexpr auto MeanCubic() const -> auto { return Moment<3>(); }
    constexpr auto MeanQuartic() const -> auto { return Moment<4>(); }

    constexpr auto Variance() const -> auto { return CentralMoment<2>(); }
    auto StdDev() const -> auto { return std::sqrt(Variance()); }

    auto Skewness() const -> auto { return CentralMoment<3>() / muc::pow(StdDev(), 3); }

    constexpr auto Kurtosis() const -> auto { return CentralMoment<4>() / muc::pow(Variance(), 2); }

    constexpr auto EffectiveN() const -> auto { return muc::pow(fSumW, 2) / fSumW2; }
    constexpr auto VarianceOfMean() const -> auto { return Variance() / EffectiveN(); }
    auto StdDevOfMean() const -> auto { return std::sqrt(VarianceOfMean()); }

private:
    double fSumWX;
    double fSumWX2;
    double fSumWX3;
    double fSumWX4;
    double fSumW;
    double fSumW2;
};

template<int N>
    requires(N > 0)
class Statistic {
public:
    Statistic();
    template<std::ranges::input_range S = std::initializer_list<Eigen::Vector<double, N>>>
        requires Concept::InputVectorAny<std::ranges::range_value_t<S>, N>
    explicit Statistic(const S& sample, double weight = 1);
    template<std::ranges::input_range S = std::initializer_list<Eigen::Vector<double, N>>, std::ranges::input_range W = std::initializer_list<double>>
        requires Concept::InputVectorAny<std::ranges::range_value_t<S>, N> and std::convertible_to<std::ranges::range_value_t<W>, double>
    Statistic(const S& sample, const W& weight);

    template<Concept::InputVectorAny<N> T = Eigen::Vector<double, N>>
    auto Fill(const T& sample, double weight = 1) -> void;
    template<std::ranges::input_range S = std::initializer_list<Eigen::Vector<double, N>>>
        requires Concept::InputVectorAny<std::ranges::range_value_t<S>, N>
    auto Fill(const S& sample, double weight = 1) -> void;
    template<std::ranges::input_range S = std::initializer_list<Eigen::Vector<double, N>>, std::ranges::input_range W = std::initializer_list<double>>
        requires Concept::InputVectorAny<std::ranges::range_value_t<S>, N> and std::convertible_to<std::ranges::range_value_t<W>, double>
    auto Fill(const S& sample, const W& weight) -> void;

    auto Sum(int i) const -> auto { return fSumWX[i]; }
    auto SumProduct(int i, int j) const -> auto { return fSumWXX(i, j); }
    auto SumSquare(int i) const -> auto { return SumProduct(i, i); }
    auto SumCubic(int i) const -> auto { return fSumWX3[i]; }
    auto SumQuartic(int i) const -> auto { return fSumWX4[i]; }
    auto Sum() const -> const auto& { return fSumWX; }
    auto SumProduct() const -> const auto& { return fSumWXX; }
    auto SumSquare() const -> auto { return SumProduct().diagonal().eval(); }
    auto SumCubic() const -> const auto& { return fSumWX3; }
    auto SumQuartic() const -> const auto& { return fSumWX4; }

    auto WeightSum() const -> const auto& { return fSumW; }

    template<int K>
        requires(0 <= K and K <= 4)
    auto Moment(int i) const -> double;
    template<int K>
        requires(0 <= K and K <= 4)
    auto Moment() const -> Eigen::Vector<double, N>;

    template<int K>
        requires(0 <= K and K <= 4)
    auto CentralMoment(int i) const -> double;
    template<int K>
        requires(0 <= K and K <= 4)
    auto CentralMoment() const -> Eigen::Vector<double, N>;

    auto Mixed2ndMoment(int i, int j) const -> auto { return fSumWXX(i, j) / fSumW; }
    auto Mixed2ndMoment() const -> auto { return (fSumWXX / fSumW).eval(); }

    auto Mixed2ndCentralMoment(int i, int j) const -> auto { return Mixed2ndMoment(i, j) - Moment<1>(i) * Moment<1>(j); }
    auto Mixed2ndCentralMoment() const -> Eigen::Matrix<double, N, N>;

    auto Mean(int i) const -> auto { return Moment<1>(i); }
    auto MeanSquare(int i) const -> auto { return Moment<2>(i); }
    auto MeanCubic(int i) const -> auto { return Moment<3>(i); }
    auto MeanQuartic(int i) const -> auto { return Moment<4>(i); }
    auto MeanProduct(int i, int j) const -> auto { return Mixed2ndMoment(i, j); }
    auto Mean() const -> auto { return Moment<1>(); }
    auto MeanSquare() const -> auto { return Moment<2>(); }
    auto MeanCubic() const -> auto { return Moment<3>(); }
    auto MeanQuartic() const -> auto { return Moment<4>(); }
    auto MeanProduct() const -> auto { return Mixed2ndMoment(); }

    auto Variance(int i) const -> auto { return CentralMoment<2>(i); }
    auto StdDev(int i) const -> auto { return std::sqrt(Variance(i)); }
    auto Covariance(int i, int j) const -> auto { return Mixed2ndCentralMoment(i, j); }
    auto Variance() const -> auto { return CentralMoment<2>(); }
    auto StdDev() const -> auto { return Variance().cwiseSqrt().eval(); }
    auto Covariance() const -> auto { return Mixed2ndCentralMoment(); }

    auto Skewness(int i) const -> auto { return CentralMoment<3>(i) / muc::pow(StdDev(i), 3); }
    auto Skewness() const -> Eigen::Vector<double, N>;

    auto Kurtosis(int i) const -> auto { return CentralMoment<4>(i) / muc::pow(Variance(i), 2); }
    auto Kurtosis() const -> Eigen::Vector<double, N>;

    constexpr auto EffectiveN() const -> auto { return muc::pow(fSumW, 2) / fSumW2; }
    constexpr auto VarianceOfMean(int i) const -> auto { return Variance(i) / EffectiveN(); }
    constexpr auto VarianceOfMean() const -> auto { return (Variance() / EffectiveN()).eval(); }
    constexpr auto CovarianceOfMean(int i, int j) const -> auto { return Covariance(i, j) / EffectiveN(); }
    constexpr auto CovarianceOfMean() const -> auto { return (Covariance() / EffectiveN()).eval(); }
    auto StdDevOfMean(int i) const -> auto { return std::sqrt(VarianceOfMean(i)); }
    auto StdDevOfMean() const -> auto { return VarianceOfMean().cwiseSqrt().eval(); }

private:
    Eigen::Vector<double, N> fSumWX;
    Eigen::Matrix<double, N, N> fSumWXX;
    Eigen::Vector<double, N> fSumWX3;
    Eigen::Vector<double, N> fSumWX4;
    double fSumW;
    double fSumW2;
};

} // namespace Mustard::Math

#include "Mustard/Math/Statistic.inl"
