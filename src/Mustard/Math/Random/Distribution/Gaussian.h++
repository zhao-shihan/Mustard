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

#include "Mustard/Math/Random/Distribution/Gaussian2D.h++"
#include "Mustard/Math/Random/RandomNumberDistributionBase.h++"
#include "Mustard/Utility/ClassSemanticHelper.h++"

#include "TRandom.h"

#include "muc/concepts"
#include "muc/utility"

#include <array>
#include <cmath>
#include <concepts>
#include <limits>
#include <random>

namespace Mustard::inline Math::Random::inline Distribution {

namespace impl {

template<std::floating_point T, template<typename> typename AGaussian>
class BasicGaussianSParameter final : public DistributionParameterBase<BasicGaussianSParameter<T, AGaussian>,
                                                                       AGaussian<T>> {
private:
    using Base = DistributionParameterBase<BasicGaussianSParameter<T, AGaussian>,
                                           AGaussian<T>>;

public:
    constexpr BasicGaussianSParameter() = default;
    constexpr BasicGaussianSParameter(T mu, T sigma);

    constexpr auto Mu() const -> auto { return fMu; }
    constexpr auto Sigma() const -> auto { return fSigma; }

    constexpr auto Mu(T mu) -> void { fMu = mu; }
    constexpr auto Sigma(T sigma) -> void { fSigma = sigma; }

    template<muc::character AChar>
    friend auto operator<<(std::basic_ostream<AChar>& os, const BasicGaussianSParameter& self) -> decltype(os) { return self.StreamOutput(os); }
    template<muc::character AChar>
    friend auto operator>>(std::basic_istream<AChar>& is, BasicGaussianSParameter& self) -> decltype(is) { return self.StreamInput(is); }

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
                                                         BasicGaussianSParameter<T, AGaussian>,
                                                         T> {
private:
    using Base = RandomNumberDistributionBase<AGaussian<T>,
                                              BasicGaussianSParameter<T, AGaussian>,
                                              T>;

public:
    constexpr GaussianBase() = default;
    constexpr GaussianBase(T mu, T sigma);
    constexpr explicit GaussianBase(const typename Base::ParameterType& p);

protected:
    MUSTARD_UTILITY_DECLARE_NONVIRTUAL_PROTECTED_DEFAULT_DESTRUCTOR_IN_DEFAULT_MOVEABLE(GaussianBase)

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

} // namespace impl

/// @brief Generates normal random numbers.
/// Delegates to Gaussian2D and caches the second value for the next call.
/// It is faster than GaussianS by ~1.2x on x86-64 with FMA and AVX2 and they are practically the same.
/// @tparam T The result type.
template<std::floating_point T = double>
class Gaussian;

template<std::floating_point T>
using GaussianParameter = impl::BasicGaussianSParameter<T, Gaussian>;

template<std::floating_point T>
class Gaussian final : public impl::GaussianBase<T, Gaussian> {
public:
    using impl::GaussianBase<T, Gaussian>::GaussianBase;

    constexpr auto Reset() -> void { fCached = false; }

    template<std::uniform_random_bit_generator G>
    auto operator()(G& g) -> auto { return Impl(g, this->fParameter); }
    template<std::uniform_random_bit_generator G>
    auto operator()(G& g, const GaussianParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(CLHEP::HepRandomEngine& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(CLHEP::HepRandomEngine& g, const GaussianParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(TRandom& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(TRandom& g, const GaussianParameter<T>& p) -> auto { return Impl(g, p); }

    constexpr auto Min() const -> auto { return std::numeric_limits<T>::lowest(); }
    constexpr auto Max() const -> auto { return std::numeric_limits<T>::max(); }

private:
    auto Impl(auto& g, const GaussianParameter<T>& p) -> T;

private:
    bool fCached{};
    T fCachedValue{};
};

template<typename T, typename U>
Gaussian(T, U) -> Gaussian<std::common_type_t<T, U>>;

/// @brief Generates normal random numbers.
/// Delegates to Gaussian2DS and caches the second value for the next call. This version is mainly for testing purpose.
/// @tparam T The result type.
template<std::floating_point T = double>
class GaussianS;

template<std::floating_point T>
using GaussianSParameter = impl::BasicGaussianSParameter<T, GaussianS>;

template<std::floating_point T>
class GaussianS final : public impl::GaussianBase<T, GaussianS> {
public:
    using impl::GaussianBase<T, GaussianS>::GaussianBase;

    constexpr auto Reset() -> void { fCached = false; }

    template<std::uniform_random_bit_generator G>
    auto operator()(G& g) -> auto { return Impl(g, this->fParameter); }
    template<std::uniform_random_bit_generator G>
    auto operator()(G& g, const GaussianSParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(CLHEP::HepRandomEngine& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(CLHEP::HepRandomEngine& g, const GaussianSParameter<T>& p) -> auto { return Impl(g, p); }

    auto operator()(TRandom& g) -> auto { return Impl(g, this->fParameter); }
    auto operator()(TRandom& g, const GaussianSParameter<T>& p) -> auto { return Impl(g, p); }

    constexpr auto Min() const -> auto { return std::numeric_limits<T>::lowest(); }
    constexpr auto Max() const -> auto { return std::numeric_limits<T>::max(); }

private:
    auto Impl(auto& g, const GaussianSParameter<T>& p) -> T;

private:
    bool fCached{};
    T fCachedValue{};
};

template<typename T, typename U>
GaussianS(T, U) -> GaussianS<std::common_type_t<T, U>>;

} // namespace Mustard::inline Math::Random::inline Distribution

#include "Mustard/Math/Random/Distribution/Gaussian.inl"
