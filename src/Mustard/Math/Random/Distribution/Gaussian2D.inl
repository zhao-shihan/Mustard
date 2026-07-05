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

namespace Mustard::inline Math::Random::inline Distribution {

namespace impl {

template<Concept::NumericVector2FloatingPoint T, template<typename> typename AGaussian2D>
constexpr BasicGaussian2DSParameter<T, AGaussian2D>::BasicGaussian2DSParameter() : // clang-format off
    BasicGaussian2DSParameter{{0, 1},
                                     {0, 1}} {} // clang-format on

template<Concept::NumericVector2FloatingPoint T, template<typename> typename AGaussian2D>
constexpr BasicGaussian2DSParameter<T, AGaussian2D>::BasicGaussian2DSParameter(std::pair<Scalar, Scalar> pX, std::pair<Scalar, Scalar> pY) :
    Base{},
    fMuX{pX.first},
    fSigmaX{pX.second},
    fMuY{pY.first},
    fSigmaY{pY.second} {}

template<Concept::NumericVector2FloatingPoint T, template<typename> typename AGaussian2D>
template<muc::character AChar>
auto BasicGaussian2DSParameter<T, AGaussian2D>::StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os) {
    const auto oldPrecision{os.precision(std::numeric_limits<VectorValueType<T>>::max_digits10)};
    return os << fMuX << ' ' << fSigmaX << ' ' << fMuY << ' ' << fSigmaY
              << std::setprecision(oldPrecision);
}

template<Concept::NumericVector2FloatingPoint T, template<typename> typename AGaussian2D>
template<muc::character AChar>
auto BasicGaussian2DSParameter<T, AGaussian2D>::StreamInput(std::basic_istream<AChar>& is) & -> decltype(is) {
    return is >> fMuX >> fSigmaX >> fMuY >> fSigmaY;
}

template<template<typename> typename ADerived, Concept::NumericVector2FloatingPoint T>
constexpr Gaussian2DBase<ADerived, T>::Gaussian2DBase(std::pair<Scalar, Scalar> pX, std::pair<Scalar, Scalar> pY) :
    Base{},
    fParameter{pX, pY} {}

template<template<typename> typename ADerived, Concept::NumericVector2FloatingPoint T>
constexpr Gaussian2DBase<ADerived, T>::Gaussian2DBase(const typename Base::ParameterType& p) :
    Base{},
    fParameter{p} {}

} // namespace impl

template<Concept::NumericVector2FloatingPoint T>
auto Gaussian2D<T>::Impl(auto& g, const Gaussian2DParameter<T>& p) -> T {
    constexpr auto pi{std::numbers::pi_v<Scalar>};
    const auto r{std::sqrt(-2 * impl::FastLogOn01(1 - Uniform<Scalar>{}(g)))}; // u in [0,1) so 1-u in (0,1]
    const auto [sinx, cosx]{impl::FastSinCosOnPiPi(Uniform<Scalar>{-pi, pi}(g))};
    return {p.SigmaX() * (r * cosx) + p.MuX(),
            p.SigmaY() * (r * sinx) + p.MuY()};
}

template<Concept::NumericVector2FloatingPoint T>
auto Gaussian2DS<T>::Impl(auto& g, const Gaussian2DSParameter<T>& p) -> T {
    T u;
    Scalar x;
    do {
        u = UniformRectangle<T>({-0.5, 0.5}, {-0.5, 0.5})(g);
        x = muc::hypot_sq(u[0], u[1]);
    } while (x == 0 or x >= 0.25);
    x = std::sqrt(-2 * (std::log(x) + 2 * std::numbers::ln2_v<Scalar>) / x);
    u[0] = p.SigmaX() * (x * u[0]) + p.MuX();
    u[1] = p.SigmaY() * (x * u[1]) + p.MuY();
    return u;
}

} // namespace Mustard::inline Math::Random::inline Distribution
