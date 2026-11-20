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

template<Concept::NumericVector3FloatingPoint T, template<typename> typename AGaussian3D>
constexpr BasicGaussian3DSParameter<T, AGaussian3D>::BasicGaussian3DSParameter() : // clang-format off
    BasicGaussian3DSParameter{{0, 1}, {0, 1}, {0, 1}} {} // clang-format on

template<Concept::NumericVector3FloatingPoint T, template<typename> typename AGaussian3D>
constexpr BasicGaussian3DSParameter<T, AGaussian3D>::BasicGaussian3DSParameter(std::pair<Scalar, Scalar> pX, std::pair<Scalar, Scalar> pY, std::pair<Scalar, Scalar> pZ) :
    Base{},
    fMuX{pX.first},
    fSigmaX{pX.second},
    fMuY{pY.first},
    fSigmaY{pY.second},
    fMuZ{pZ.first},
    fSigmaZ{pZ.second} {}

template<Concept::NumericVector3FloatingPoint T, template<typename> typename AGaussian3D>
template<muc::character AChar>
auto BasicGaussian3DSParameter<T, AGaussian3D>::StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os) {
    const auto oldPrecision{os.precision(std::numeric_limits<VectorValueType<T>>::max_digits10)};
    return os << fMuX << ' ' << fSigmaX << ' '
              << fMuY << ' ' << fSigmaY << ' '
              << fMuZ << ' ' << fSigmaZ
              << std::setprecision(oldPrecision);
}

template<Concept::NumericVector3FloatingPoint T, template<typename> typename AGaussian3D>
template<muc::character AChar>
auto BasicGaussian3DSParameter<T, AGaussian3D>::StreamInput(std::basic_istream<AChar>& is) & -> decltype(is) {
    return is >>
           fMuX >> fSigmaX >>
           fMuY >> fSigmaY >>
           fMuZ >> fSigmaZ;
}

template<template<typename> typename ADerived, Concept::NumericVector3FloatingPoint T>
constexpr Gaussian3DBase<ADerived, T>::Gaussian3DBase(std::pair<Scalar, Scalar> pX, std::pair<Scalar, Scalar> pY, std::pair<Scalar, Scalar> pZ) :
    Base{},
    fParameter{pX, pY, pZ} {}

template<template<typename> typename ADerived, Concept::NumericVector3FloatingPoint T>
constexpr Gaussian3DBase<ADerived, T>::Gaussian3DBase(const typename Base::ParameterType& p) :
    Base{},
    fParameter{p} {}

} // namespace impl

template<Concept::NumericVector3FloatingPoint T>
auto Gaussian3D<T>::Impl(auto& g, const Gaussian3DParameter<T>& p) -> T {
    // Gaussian4D is vectorized and even faster than caching one value
    static_assert(Gaussian4D<std::array<Scalar, 4>>::Stateless());
    Gaussian4D<std::array<Scalar, 4>> gaussian4D{
        {p.MuX(), p.SigmaX()},
        {p.MuY(), p.SigmaY()},
        {p.MuZ(), p.SigmaZ()},
        {0,       1         }
    };
    const auto [x, y, z, _]{gaussian4D(g)};
    return {x, y, z};
}

template<Concept::NumericVector3FloatingPoint T>
auto Gaussian3DS<T>::Impl(auto& g, const Gaussian3DSParameter<T>& p) -> T {
    static_assert(Gaussian2DS<std::array<Scalar, 2>>::Stateless());
    Gaussian2DS<std::array<Scalar, 2>> gaussian2D;
    const auto [u, v]{gaussian2D(g)};
    if ((fCached = not fCached)) {
        const auto [w, x]{gaussian2D(g)};
        fCachedValue = x;
        return {p.SigmaX() * u + p.MuX(),
                p.SigmaY() * v + p.MuY(),
                p.SigmaZ() * w + p.MuZ()};
    } else {
        return {p.SigmaX() * u + p.MuX(),
                p.SigmaY() * v + p.MuY(),
                p.SigmaZ() * fCachedValue + p.MuZ()};
    }
}

} // namespace Mustard::inline Math::Random::inline Distribution
