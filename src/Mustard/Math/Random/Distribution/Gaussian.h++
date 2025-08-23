// -*- C++ -*-
//
// Copyright 2020-2025  The Mustard development team
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

#include "Mustard/Math/Random/Distribution/Gaussian2DDiagnoal.h++"
#include "Mustard/Math/Random/RandomNumberDistributionBase.h++"

#include "muc/concepts"
#include "muc/utility"

#include <array>
#include <cmath>
#include <concepts>
#include <limits>

namespace Mustard::Math::Random::inline Distribution {

namespace internal {

template<std::floating_point T, template<typename> typename AGaussian>
class BasicGaussianParameter final : public DistributionParameterBase<BasicGaussianParameter<T, AGaussian>,
                                                                      AGaussian<T>> {
private:
    using Base = DistributionParameterBase<BasicGaussianParameter<T, AGaussian>,
                                           AGaussian<T>>;

public:
    constexpr BasicGaussianParameter() = default;
    constexpr BasicGaussianParameter(T mu, T sigma);

    constexpr auto Mu() const -> auto { return fMu; }
    constexpr auto Sigma() const -> auto { return fSigma; }

    constexpr auto Mu(T mu) -> void { fMu = mu; }
    constexpr auto Sigma(T sigma) -> void { fSigma = sigma; }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const BasicGaussianParameter& self) -> decltype(os) { return self.StreamOutput(os); }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, BasicGaussianParameter& self) -> decltype(is) { return self.StreamInput(is); }

private:
    template<muc::character AChar>
    auto StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os);
    template<muc::character AChar>
    auto StreamInput(std::basic_istream<AChar>& is) & -> decltype(is);

private:
    T fMu{0};
    T fSigma{1};
};

template<std::floating_point T, template<typename> typename AGaussian>
class GaussianBase : public RandomNumberDistributionBase<AGaussian<T>,
                                                         BasicGaussianParameter<T, AGaussian>,
                                                         T> {
private:
    using Base = RandomNumberDistributionBase<AGaussian<T>,
                                              BasicGaussianParameter<T, AGaussian>,
                                              T>;

public:
    constexpr GaussianBase() = default;
    constexpr GaussianBase(T mu, T sigma);
    constexpr explicit GaussianBase(const typename Base::ParameterType& p);

protected:
    constexpr ~GaussianBase() = default;

public:
    constexpr auto Parameter() const -> auto { return fParameter; }
    constexpr auto Mu() const -> auto { return fParameter.Mu(); }
    constexpr auto Sigma() const -> auto { return fParameter.Sigma(); }

    constexpr auto Parameter(const typename Base::ParameterType& p) -> void { fParameter = p; }
    constexpr auto Mu(T mu) const -> auto { return fParameter.Mu(mu); }
    constexpr auto Sigma(T sigma) const -> auto { return fParameter.Sigma(sigma); }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const GaussianBase& self) -> auto& { return os << self.fParameter; }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, GaussianBase& self) -> auto& { return is >> self.fParameter; }

private:
    template<muc::character AChar>
    auto StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os);
    template<muc::character AChar>
    auto StreamInput(std::basic_istream<AChar>& is) & -> decltype(is);

protected:
    typename Base::ParameterType fParameter;
};

} // namespace internal

/// @brief Reference: Luc Devroye, Non-Uniform Random Variates Generation,
/// Chapter V, Section 4.4 (See http://luc.devroye.org/rnbookindex.html).
/// @tparam T The result type.
template<std::floating_point T = double>
class Gaussian;

template<std::floating_point T>
using GaussianParameter = internal::BasicGaussianParameter<T, Gaussian>;

template<std::floating_point T>
class Gaussian final : public internal::GaussianBase<T, Gaussian> {
public:
    using internal::GaussianBase<T, Gaussian>::GaussianBase;

    constexpr auto Reset() -> void { fSaved = false; }

    MUSTARD_STRONG_INLINE auto operator()(UniformRandomBitGenerator auto& g) -> auto { return (*this)(g, this->fParameter); }
    MUSTARD_STRONG_INLINE auto operator()(UniformRandomBitGenerator auto& g, const GaussianParameter<T>& p) -> T;

    constexpr auto Min() const -> auto { return std::numeric_limits<T>::lowest(); }
    constexpr auto Max() const -> auto { return std::numeric_limits<T>::max(); }

    static constexpr auto Stateless() -> bool { return false; }

private:
    bool fSaved{};
    T fSavedValue;
};

template<typename T, typename U>
Gaussian(T, U) -> Gaussian<std::common_type_t<T, U>>;

/// @brief Reference: Luc Devroye, Non-Uniform Random Variates Generation,
/// Chapter V, Section 4.4 (See http://luc.devroye.org/rnbookindex.html).
/// This version is faster than Gaussian and less accurate for a tiny bit. See
/// note.
/// @note This version uses RA2Log (P2/Q2 rational) instead of full-precision
/// Log. The average truncation error of RA2Log is O(10^-6), and the max
/// truncation error is less than 10^-5. This error will propagate to the
/// generated random numbers.
/// @tparam T The result type.
template<std::floating_point T = double>
class GaussianFast;

template<std::floating_point T>
using GaussianFastParameter = internal::BasicGaussianParameter<T, GaussianFast>;

template<std::floating_point T>
class GaussianFast final : public internal::GaussianBase<T, GaussianFast> {
public:
    using internal::GaussianBase<T, GaussianFast>::GaussianBase;

    constexpr auto Reset() -> void { fSaved = false; }

    MUSTARD_STRONG_INLINE auto operator()(UniformRandomBitGenerator auto& g) -> auto { return (*this)(g, this->fParameter); }
    MUSTARD_STRONG_INLINE auto operator()(UniformRandomBitGenerator auto& g, const GaussianFastParameter<T>& p) -> T;

    constexpr auto Min() const -> auto { return std::numeric_limits<T>::lowest(); }
    constexpr auto Max() const -> auto { return std::numeric_limits<T>::max(); }

    static constexpr auto Stateless() -> bool { return false; }

private:
    bool fSaved{};
    T fSavedValue;
};

template<typename T, typename U>
GaussianFast(T, U) -> GaussianFast<std::common_type_t<T, U>>;

} // namespace Mustard::Math::Random::inline Distribution

#include "Mustard/Math/Random/Distribution/Gaussian.inl"
