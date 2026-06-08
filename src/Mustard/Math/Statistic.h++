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

#include "Mustard/Concept/InputVector.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Math/Estimate.h++"
#include "Mustard/Utility/FunctionAttribute.h++"
#include "Mustard/Utility/VectorCast.h++"

#include "TBase64.h"

#include "Eigen/Core"

#include "mplr/mplr.hpp"

#include "muc/math"

#include "gsl/gsl"

#include "fmt/format.h"

#include <cmath>
#include <cstring>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace Mustard::inline Math {

template<int N>
    requires(0 < N and N * N <= EIGEN_STACK_ALLOCATION_LIMIT / sizeof(double))
class Statistic;

template<int N>
    requires(0 < N and N * N <= EIGEN_STACK_ALLOCATION_LIMIT / sizeof(double))
struct SerializedStatistic;

template<>
class Statistic<1> {
public:
    using MeanType = double;
    using CovarianceType = double;
    using SerializedType = SerializedStatistic<1>;

public:
    MUSTARD_ALWAYS_INLINE Statistic();
    MUSTARD_ALWAYS_INLINE explicit Statistic(const SerializedType& data);

    MUSTARD_ALWAYS_INLINE auto Fill(double x, double w = 1) -> void;

    auto SampleSize() const -> auto { return fN; }
    auto WeightSum() const -> auto { return fW; }
    auto WeightSquareSum() const -> auto { return fW2; }

    auto Sum() const -> auto { return fM; }
    auto Mean() const -> auto { return fM / fW; }
    auto Variance() const -> auto { return Debias() * (fM2 / fW); }
    auto StdDev() const -> auto { return std::sqrt(Variance()); }

    auto EffectiveSampleSize() const -> auto { return muc::pow(fW, 2) / fW2; }
    auto VarianceOfMean() const -> auto { return Variance() / EffectiveSampleSize(); }
    auto StdDevOfMean() const -> auto { return std::sqrt(VarianceOfMean()); }
    auto MeanEstimate() const -> Estimate { return {Mean(), StdDevOfMean()}; }

    MUSTARD_ALWAYS_INLINE auto operator+=(const Statistic& other) -> Statistic&;
    friend auto operator+(Statistic lhs, const Statistic& rhs) -> auto { return lhs += rhs; }

    MUSTARD_ALWAYS_INLINE auto Serialize() const -> SerializedType;
    MUSTARD_ALWAYS_INLINE auto Deserialize(const SerializedType& data) -> void;

private:
    MUSTARD_ALWAYS_INLINE auto Debias() const -> double;

private:
    long long fN;
    double fW;
    double fW2;
    double fM;
    double fM2;
};

template<int N>
    requires(0 < N and N * N <= EIGEN_STACK_ALLOCATION_LIMIT / sizeof(double))
class Statistic {
public:
    using MeanType = Eigen::Vector<double, N>;
    using CovarianceType = Eigen::Matrix<double, N, N>;
    using SerializedType = SerializedStatistic<N>;

public:
    MUSTARD_STRONG_INLINE Statistic();
    MUSTARD_STRONG_INLINE explicit Statistic(const SerializedType& data);

    template<typename T = MeanType>
        requires Concept::InputVectorAny<std::decay_t<T>, N>
    MUSTARD_STRONG_INLINE auto Fill(T&& x, double w = 1) -> void;

    auto SampleSize() const -> auto { return fN; }
    auto WeightSum() const -> auto { return fW; }
    auto WeightSquareSum() const -> auto { return fW2; }

    auto Sum(int i) const -> auto { return fM[i]; }
    auto Mean(int i) const -> auto { return fM[i] / fW; }
    auto Covariance(int i, int j) const -> auto { return Debias() * (fM2(i, j) / fW); }
    auto Variance(int i) const -> auto { return Covariance(i, i); }
    auto StdDev(int i) const -> auto { return std::sqrt(Variance(i)); }
    auto Sum() const -> const auto& { return fM; }
    auto Mean() const -> auto { return (fM / fW).eval(); }
    auto Covariance() const -> auto { return CovarianceExpression().eval(); }
    auto Variance() const -> auto { return VarianceExpression().eval(); }
    auto StdDev() const -> auto { return VarianceExpression().cwiseSqrt().eval(); }

    auto EffectiveSampleSize() const -> auto { return muc::pow(fW, 2) / fW2; }
    auto VarianceOfMean(int i) const -> auto { return Variance(i) / EffectiveSampleSize(); }
    auto CovarianceOfMean(int i, int j) const -> auto { return Covariance(i, j) / EffectiveSampleSize(); }
    auto StdDevOfMean(int i) const -> auto { return std::sqrt(VarianceOfMean(i)); }
    auto MeanEstimate(int i) const -> Estimate { return {Mean(i), StdDevOfMean(i)}; }
    auto VarianceOfMean() const -> auto { return VarianceOfMeanExpression().eval(); }
    auto CovarianceOfMean() const -> auto { return (CovarianceExpression() / EffectiveSampleSize()).eval(); }
    auto StdDevOfMean() const -> auto { return VarianceOfMeanExpression().cwiseSqrt().eval(); }

    MUSTARD_STRONG_INLINE auto operator+=(const Statistic& other) -> Statistic&;
    friend auto operator+(Statistic lhs, const Statistic& rhs) -> auto { return lhs += rhs; }

    MUSTARD_STRONG_INLINE auto Serialize() const -> SerializedType;
    MUSTARD_STRONG_INLINE auto Deserialize(const SerializedType& data) -> void;

private:
    MUSTARD_ALWAYS_INLINE auto Debias() const -> double;
    auto CovarianceExpression() const -> auto { return Debias() * (fM2 / fW); }
    auto VarianceExpression() const -> auto { return Debias() * (fM2.diagonal() / fW); }
    auto VarianceOfMeanExpression() const -> auto { return VarianceExpression() / EffectiveSampleSize(); }

private:
    long long fN;
    double fW;
    double fW2;
    MeanType fM;
    CovarianceType fM2;
};

template<int N>
    requires(0 < N and N * N <= EIGEN_STACK_ALLOCATION_LIMIT / sizeof(double))
struct SerializedStatistic {
public:
    SerializedStatistic();
    SerializedStatistic(const std::string& base64);

    auto EncodeBase64() const -> std::string;
    auto DecodeBase64(const std::string& base64) -> void;

    auto operator+(const SerializedStatistic& other) -> auto { return (Statistic<N>{*this} + Statistic<N>{other}).Serialize(); }

public:
    long long fN;
    double fW;
    double fW2;
    double fM[N];
    double fM2[N * N];
};

} // namespace Mustard::inline Math

MPLR_REFLECTION_TEMPLATE((int N), (Mustard::Math::SerializedStatistic<N>),
                         fN, fW, fW2, fM, fM2)

#include "Mustard/Math/Statistic.inl"
