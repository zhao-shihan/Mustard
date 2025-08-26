// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

template<Concept::NumericVector2Any T, template<typename> typename AUniformDisk>
constexpr BasicUniformDiskParameter<T, AUniformDisk>::BasicUniformDiskParameter() :
    BasicUniformDiskParameter{1} {}

template<Concept::NumericVector2Any T, template<typename> typename AUniformDisk>
constexpr BasicUniformDiskParameter<T, AUniformDisk>::BasicUniformDiskParameter(VT r, VT x0, VT y0) :
    Base{},
    fRadius{r},
    fCenterX{x0},
    fCenterY{y0} {}

template<Concept::NumericVector2Any T, template<typename> typename AUniformDisk>
constexpr BasicUniformDiskParameter<T, AUniformDisk>::BasicUniformDiskParameter(VT radius, T center) :
    Base{},
    fRadius{radius},
    fCenterX{center[0]},
    fCenterY{center[1]} {}

template<Concept::NumericVector2Any T, template<typename> typename AUniformDisk>
constexpr BasicUniformDiskParameter<T, AUniformDisk>::BasicUniformDiskParameter(VT radius) :
    Base{},
    fRadius{radius},
    fCenterX{0},
    fCenterY{0} {}

template<Concept::NumericVector2Any T, template<typename> typename AUniformDisk>
template<muc::character AChar>
auto BasicUniformDiskParameter<T, AUniformDisk>::StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os) {
    const auto oldPrecision{os.precision(std::numeric_limits<VectorValueType<T>>::max_digits10)};
    return os << fRadius << ' ' << fCenter[0] << ' ' << fCenter[1]
              << std::setprecision(oldPrecision);
}

template<Concept::NumericVector2Any T, template<typename> typename AUniformDisk>
template<muc::character AChar>
auto BasicUniformDiskParameter<T, AUniformDisk>::StreamInput(std::basic_istream<AChar>& is) & -> decltype(is) {
    return is >> fRadius >> fCenter[0] >> fCenter[1];
}

template<Concept::NumericVector2Any T, template<typename> typename AUniformDisk>
constexpr UniformDiskBase<T, AUniformDisk>::UniformDiskBase(VT r, VT x0, VT y0) :
    Base{},
    fParameter{r, x0, y0} {}

template<Concept::NumericVector2Any T, template<typename> typename AUniformDisk>
constexpr UniformDiskBase<T, AUniformDisk>::UniformDiskBase(VT radius, T center) :
    Base{},
    fParameter{radius, center} {}

template<Concept::NumericVector2Any T, template<typename> typename AUniformDisk>
constexpr UniformDiskBase<T, AUniformDisk>::UniformDiskBase(VT radius) :
    Base{},
    fParameter{radius} {}

template<Concept::NumericVector2Any T, template<typename> typename AUniformDisk>
constexpr UniformDiskBase<T, AUniformDisk>::UniformDiskBase(const typename Base::ParameterType& p) :
    Base{},
    fParameter{p} {}

} // namespace internal

#define MUSTARD_MATH_RANDOM_DISTRIBUTION_UNIFORM_DISK_GENERATOR(rejection) \
    T r;                                                                   \
    VectorValueType<T> r2;                                                 \
    do {                                                                   \
        r = UniformCompactRectangle<T>({-0.5, 0.5}, {-0.5, 0.5})(g);       \
        r2 = muc::hypot_sq(r[0], r[1]);                                    \
        muc::assume(0 <= r2 and r2 <= 0.5);                                \
    } while (rejection);                                                   \
    if constexpr (Concept::MathVector2Any<T>) {                            \
        r = 2 * p.Radius() * r + p.Center();                               \
    } else {                                                               \
        r[0] = 2 * p.Radius() * r[0] + p.Center()[0];                      \
        r[1] = 2 * p.Radius() * r[1] + p.Center()[1];                      \
    }                                                                      \
    return r;

template<Concept::NumericVector2Any T>
MUSTARD_STRONG_INLINE constexpr auto UniformCompactDisk<T>::operator()(UniformRandomBitGenerator auto& g, const UniformCompactDiskParameter<T>& p) -> T {
    MUSTARD_MATH_RANDOM_DISTRIBUTION_UNIFORM_DISK_GENERATOR(r2 > 0.25)
}

template<Concept::NumericVector2Any T>
MUSTARD_STRONG_INLINE constexpr auto UniformDisk<T>::operator()(UniformRandomBitGenerator auto& g, const UniformDiskParameter<T>& p) -> T {
    MUSTARD_MATH_RANDOM_DISTRIBUTION_UNIFORM_DISK_GENERATOR(r2 >= 0.25)
}

#undef MUSTARD_MATH_RANDOM_DISTRIBUTION_UNIFORM_DISK_GENERATOR

} // namespace Mustard::Math::Random::inline Distribution
