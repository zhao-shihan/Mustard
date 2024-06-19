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

namespace Mustard::Math::Random::inline Distribution {

namespace internal {

template<Concept::NumericVector3FloatingPoint T, template<typename> typename AGaussian3DDiagnoal>
constexpr BasicGaussian3DDiagnoalParameter<T, AGaussian3DDiagnoal>::BasicGaussian3DDiagnoalParameter() : // clang-format off
    BasicGaussian3DDiagnoalParameter{{0, 1},
                                     {0, 1},
                                     {0, 1}} {} // clang-format on

template<Concept::NumericVector3FloatingPoint T, template<typename> typename AGaussian3DDiagnoal>
constexpr BasicGaussian3DDiagnoalParameter<T, AGaussian3DDiagnoal>::BasicGaussian3DDiagnoalParameter(std::pair<VT, VT> pX, std::pair<VT, VT> pY, std::pair<VT, VT> pZ) :
    Base{},
    fMuX{pX.first},
    fSigmaX{pX.second},
    fMuY{pY.first},
    fSigmaY{pY.second},
    fMuZ{pZ.first},
    fSigmaZ{pZ.second} {}

template<Concept::NumericVector3FloatingPoint T, template<typename> typename AGaussian3DDiagnoal>
template<Concept::Character AChar>
auto BasicGaussian3DDiagnoalParameter<T, AGaussian3DDiagnoal>::StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os) {
    const auto oldPrecision{os.precision(std::numeric_limits<VectorValueType<T>>::max_digits10)};
    return os << fMuX << ' ' << fSigmaX << ' '
              << fMuY << ' ' << fSigmaY << ' '
              << fMuZ << ' ' << fSigmaZ
              << std::setprecision(oldPrecision);
}

template<Concept::NumericVector3FloatingPoint T, template<typename> typename AGaussian3DDiagnoal>
template<Concept::Character AChar>
auto BasicGaussian3DDiagnoalParameter<T, AGaussian3DDiagnoal>::StreamInput(std::basic_istream<AChar>& is) & -> decltype(is) {
    return is >>
           fMuX >> fSigmaX >>
           fMuY >> fSigmaY >>
           fMuZ >> fSigmaZ;
}

template<template<typename> typename ADerived, Concept::NumericVector3FloatingPoint T>
constexpr Gaussian3DDiagnoalBase<ADerived, T>::Gaussian3DDiagnoalBase(std::pair<VT, VT> pX, std::pair<VT, VT> pY, std::pair<VT, VT> pZ) :
    Base{},
    fParameter{pX, pY, pZ} {}

template<template<typename> typename ADerived, Concept::NumericVector3FloatingPoint T>
constexpr Gaussian3DDiagnoalBase<ADerived, T>::Gaussian3DDiagnoalBase(const typename Base::ParameterType& p) :
    Base{},
    fParameter{p} {}

} // namespace internal

#define MUSTARD_MATH_RANDOM_DISTRIBUTION_GAUSSIAN_3D_DIAGNOAL_GENERATOR_SNIPPET(Suffix) \
    static_assert(Gaussian2DDiagnoal##Suffix<std::array<VT, 2>>::Stateless());          \
    Gaussian2DDiagnoal##Suffix<std::array<VT, 2>> standardGaussian2D;                   \
    auto&& [u, v]{standardGaussian2D(g)};                                               \
    if ((fSaved = not fSaved)) {                                                        \
        auto&& [w, x]{standardGaussian2D(g)};                                           \
        fSavedValue = x;                                                                \
        return {p.SigmaX() * u + p.MuX(),                                               \
                p.SigmaY() * v + p.MuY(),                                               \
                p.SigmaZ() * w + p.MuZ()};                                              \
    } else {                                                                            \
        return {p.SigmaX() * u + p.MuX(),                                               \
                p.SigmaY() * v + p.MuY(),                                               \
                p.SigmaZ() * fSavedValue + p.MuZ()};                                    \
    }

template<Concept::NumericVector3FloatingPoint T>
MUSTARD_STRONG_INLINE auto Gaussian3DDiagnoal<T>::operator()(UniformRandomBitGenerator auto& g, const Gaussian3DDiagnoalParameter<T>& p) -> T {
    MUSTARD_MATH_RANDOM_DISTRIBUTION_GAUSSIAN_3D_DIAGNOAL_GENERATOR_SNIPPET()
}

template<Concept::NumericVector3FloatingPoint T>
MUSTARD_STRONG_INLINE auto Gaussian3DDiagnoalFast<T>::operator()(UniformRandomBitGenerator auto& g, const Gaussian3DDiagnoalFastParameter<T>& p) -> T {
    MUSTARD_MATH_RANDOM_DISTRIBUTION_GAUSSIAN_3D_DIAGNOAL_GENERATOR_SNIPPET(Fast)
}

#undef MUSTARD_MATH_RANDOM_DISTRIBUTION_GAUSSIAN_3D_DIAGNOAL_GENERATOR_SNIPPET

} // namespace Mustard::Math::Random::inline Distribution
