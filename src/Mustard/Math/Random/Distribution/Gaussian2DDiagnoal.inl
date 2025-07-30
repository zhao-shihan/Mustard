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

template<Concept::NumericVector2FloatingPoint T, template<typename> typename AGaussian2DDiagnoal>
constexpr BasicGaussian2DDiagnoalParameter<T, AGaussian2DDiagnoal>::BasicGaussian2DDiagnoalParameter() : // clang-format off
    BasicGaussian2DDiagnoalParameter{{0, 1},
                                     {0, 1}} {} // clang-format on

template<Concept::NumericVector2FloatingPoint T, template<typename> typename AGaussian2DDiagnoal>
constexpr BasicGaussian2DDiagnoalParameter<T, AGaussian2DDiagnoal>::BasicGaussian2DDiagnoalParameter(std::pair<VT, VT> pX, std::pair<VT, VT> pY) :
    Base{},
    fMuX{pX.first},
    fSigmaX{pX.second},
    fMuY{pY.first},
    fSigmaY{pY.second} {}

template<Concept::NumericVector2FloatingPoint T, template<typename> typename AGaussian2DDiagnoal>
template<Concept::Character AChar>
auto BasicGaussian2DDiagnoalParameter<T, AGaussian2DDiagnoal>::StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os) {
    const auto oldPrecision{os.precision(std::numeric_limits<VectorValueType<T>>::max_digits10)};
    return os << fMuX << ' ' << fSigmaX << ' ' << fMuY << ' ' << fSigmaY
              << std::setprecision(oldPrecision);
}

template<Concept::NumericVector2FloatingPoint T, template<typename> typename AGaussian2DDiagnoal>
template<Concept::Character AChar>
auto BasicGaussian2DDiagnoalParameter<T, AGaussian2DDiagnoal>::StreamInput(std::basic_istream<AChar>& is) & -> decltype(is) {
    return is >> fMuX >> fSigmaX >> fMuY >> fSigmaY;
}

template<template<typename> typename ADerived, Concept::NumericVector2FloatingPoint T>
constexpr Gaussian2DDiagnoalBase<ADerived, T>::Gaussian2DDiagnoalBase(std::pair<VT, VT> pX, std::pair<VT, VT> pY) :
    Base{},
    fParameter{pX, pY} {}

template<template<typename> typename ADerived, Concept::NumericVector2FloatingPoint T>
constexpr Gaussian2DDiagnoalBase<ADerived, T>::Gaussian2DDiagnoalBase(const typename Base::ParameterType& p) :
    Base{},
    fParameter{p} {}

} // namespace internal

#define MUSTARD_MATH_RANDOM_DISTRIBUTION_GAUSSIAN_2D_DIAGNOAL_GENERATOR_SNIPPET(TheLog) \
    T u;                                                                                \
    VT x;                                                                               \
    do {                                                                                \
        static_assert(UniformCompactRectangle<T>::Stateless());                         \
        u = UniformCompactRectangle<T>({-0.5, 0.5}, {-0.5, 0.5})(g);                    \
        x = muc::hypot_sq(u[0], u[1]);                                                  \
        muc::assume(0 <= x and x < 0.5);                                                \
    } while (x == 0 or x > 0.25);                                                       \
    x = std::sqrt(-2 * (TheLog(x) + 2 * std::numbers::ln2_v<VT>) / x);                  \
    u[0] = p.SigmaX() * (x * u[0]) + p.MuX();                                           \
    u[1] = p.SigmaY() * (x * u[1]) + p.MuY();                                           \
    return u;

template<Concept::NumericVector2FloatingPoint T>
MUSTARD_STRONG_INLINE auto Gaussian2DDiagnoal<T>::operator()(UniformRandomBitGenerator auto& g, const Gaussian2DDiagnoalParameter<T>& p) -> T {
    MUSTARD_MATH_RANDOM_DISTRIBUTION_GAUSSIAN_2D_DIAGNOAL_GENERATOR_SNIPPET(std::log)
}

template<Concept::NumericVector2FloatingPoint T>
MUSTARD_STRONG_INLINE auto Gaussian2DDiagnoalFast<T>::operator()(UniformRandomBitGenerator auto& g, const Gaussian2DDiagnoalFastParameter<T>& p) -> T {
    MUSTARD_MATH_RANDOM_DISTRIBUTION_GAUSSIAN_2D_DIAGNOAL_GENERATOR_SNIPPET(internal::FastLogForOpen01)
}

#undef MUSTARD_MATH_RANDOM_DISTRIBUTION_GAUSSIAN_2D_DIAGNOAL_GENERATOR_SNIPPET

} // namespace Mustard::Math::Random::inline Distribution
