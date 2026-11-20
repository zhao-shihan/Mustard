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

template<Concept::NumericVector4FloatingPoint T, template<typename> typename AGaussian4D>
constexpr BasicGaussian4DSParameter<T, AGaussian4D>::BasicGaussian4DSParameter() : // clang-format off
    BasicGaussian4DSParameter{{0, 1}, {0, 1}, {0, 1}, {0, 1}} {} // clang-format on

template<Concept::NumericVector4FloatingPoint T, template<typename> typename AGaussian4D>
constexpr BasicGaussian4DSParameter<T, AGaussian4D>::BasicGaussian4DSParameter(std::pair<Scalar, Scalar> pX, std::pair<Scalar, Scalar> pY, std::pair<Scalar, Scalar> pZ, std::pair<Scalar, Scalar> pW) :
    Base{},
    fMuX{pX.first},
    fSigmaX{pX.second},
    fMuY{pY.first},
    fSigmaY{pY.second},
    fMuZ{pZ.first},
    fSigmaZ{pZ.second},
    fMuW{pW.first},
    fSigmaW{pW.second} {}

template<Concept::NumericVector4FloatingPoint T, template<typename> typename AGaussian4D>
template<muc::character AChar>
auto BasicGaussian4DSParameter<T, AGaussian4D>::StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os) {
    const auto oldPrecision{os.precision(std::numeric_limits<VectorValueType<T>>::max_digits10)};
    return os << fMuX << ' ' << fSigmaX << ' '
              << fMuY << ' ' << fSigmaY << ' '
              << fMuZ << ' ' << fSigmaZ << ' '
              << fMuW << ' ' << fSigmaW
              << std::setprecision(oldPrecision);
}

template<Concept::NumericVector4FloatingPoint T, template<typename> typename AGaussian4D>
template<muc::character AChar>
auto BasicGaussian4DSParameter<T, AGaussian4D>::StreamInput(std::basic_istream<AChar>& is) & -> decltype(is) {
    return is >>
           fMuX >> fSigmaX >>
           fMuY >> fSigmaY >>
           fMuZ >> fSigmaZ >>
           fMuW >> fSigmaW;
}

template<template<typename> typename ADerived, Concept::NumericVector4FloatingPoint T>
constexpr Gaussian4DBase<ADerived, T>::Gaussian4DBase(std::pair<Scalar, Scalar> pX, std::pair<Scalar, Scalar> pY, std::pair<Scalar, Scalar> pZ, std::pair<Scalar, Scalar> pW) :
    Base{},
    fParameter{pX, pY, pZ, pW} {}

template<template<typename> typename ADerived, Concept::NumericVector4FloatingPoint T>
constexpr Gaussian4DBase<ADerived, T>::Gaussian4DBase(const typename Base::ParameterType& p) :
    Base{},
    fParameter{p} {}

} // namespace impl

template<Concept::NumericVector4FloatingPoint T>
auto Gaussian4D<T>::Impl(auto& g, const Gaussian4DParameter<T>& p) -> T {
    constexpr auto pi{std::numbers::pi_v<Scalar>};
    Uniform<Scalar> u01{};
    muc::array2<Scalar> x{1 - u01(g), 1 - u01(g)};
    x = impl::FastLogOn01<2>(x);
    const auto r1{std::sqrt(-2 * x[0])};
    const auto r2{std::sqrt(-2 * x[1])};
    Uniform<Scalar> uPiPi{-pi, pi};
    x = {uPiPi(g), uPiPi(g)};
    const auto [cosx, sinx]{impl::FastCosSinOnPiPi<2>(x)};
    return {p.SigmaX() * (r1 * cosx[0]) + p.MuX(),
            p.SigmaY() * (r1 * sinx[0]) + p.MuY(),
            p.SigmaZ() * (r2 * cosx[1]) + p.MuZ(),
            p.SigmaW() * (r2 * sinx[1]) + p.MuW()};
}

template<Concept::NumericVector4FloatingPoint T>
auto Gaussian4DS<T>::Impl(auto& g, const Gaussian4DSParameter<T>& p) -> T {
    static_assert(Gaussian2DS<std::array<Scalar, 2>>::Stateless());
    Gaussian2DS<std::array<Scalar, 2>> standardGaussian2D;
    const auto [x, y]{standardGaussian2D(g)};
    const auto [z, w]{standardGaussian2D(g)};
    return {p.SigmaX() * x + p.MuX(),
            p.SigmaY() * y + p.MuY(),
            p.SigmaZ() * z + p.MuZ(),
            p.SigmaW() * w + p.MuW()};
}

} // namespace Mustard::inline Math::Random::inline Distribution
