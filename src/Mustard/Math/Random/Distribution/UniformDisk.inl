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

template<Concept::NumericVector2FloatingPoint T, template<typename> typename AUniformDisk>
constexpr BasicUniformDiskParameter<T, AUniformDisk>::BasicUniformDiskParameter() :
    BasicUniformDiskParameter{1} {}

template<Concept::NumericVector2FloatingPoint T, template<typename> typename AUniformDisk>
constexpr BasicUniformDiskParameter<T, AUniformDisk>::BasicUniformDiskParameter(Scalar r, Scalar x0, Scalar y0) :
    Base{},
    fRadius{r},
    fCenterX{x0},
    fCenterY{y0} {}

template<Concept::NumericVector2FloatingPoint T, template<typename> typename AUniformDisk>
constexpr BasicUniformDiskParameter<T, AUniformDisk>::BasicUniformDiskParameter(Scalar radius, T center) :
    Base{},
    fRadius{radius},
    fCenterX{center[0]},
    fCenterY{center[1]} {}

template<Concept::NumericVector2FloatingPoint T, template<typename> typename AUniformDisk>
constexpr BasicUniformDiskParameter<T, AUniformDisk>::BasicUniformDiskParameter(Scalar radius) :
    Base{},
    fRadius{radius},
    fCenterX{0},
    fCenterY{0} {}

template<Concept::NumericVector2FloatingPoint T, template<typename> typename AUniformDisk>
template<muc::character AChar>
auto BasicUniformDiskParameter<T, AUniformDisk>::StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os) {
    const auto oldPrecision{os.precision(std::numeric_limits<VectorValueType<T>>::max_digits10)};
    return os << fRadius << ' ' << fCenterX << ' ' << fCenterY
              << std::setprecision(oldPrecision);
}

template<Concept::NumericVector2FloatingPoint T, template<typename> typename AUniformDisk>
template<muc::character AChar>
auto BasicUniformDiskParameter<T, AUniformDisk>::StreamInput(std::basic_istream<AChar>& is) & -> decltype(is) {
    return is >> fRadius >> fCenterX >> fCenterY;
}

template<Concept::NumericVector2FloatingPoint T, template<typename> typename AUniformDisk>
constexpr UniformDiskBase<T, AUniformDisk>::UniformDiskBase(Scalar r, Scalar x0, Scalar y0) :
    Base{},
    fParameter{r, x0, y0} {}

template<Concept::NumericVector2FloatingPoint T, template<typename> typename AUniformDisk>
constexpr UniformDiskBase<T, AUniformDisk>::UniformDiskBase(Scalar radius, T center) :
    Base{},
    fParameter{radius, center} {}

template<Concept::NumericVector2FloatingPoint T, template<typename> typename AUniformDisk>
constexpr UniformDiskBase<T, AUniformDisk>::UniformDiskBase(Scalar radius) :
    Base{},
    fParameter{radius} {}

template<Concept::NumericVector2FloatingPoint T, template<typename> typename AUniformDisk>
constexpr UniformDiskBase<T, AUniformDisk>::UniformDiskBase(const typename Base::ParameterType& p) :
    Base{},
    fParameter{p} {}

} // namespace impl

template<Concept::NumericVector2FloatingPoint T>
auto UniformDisk<T>::Impl(auto& g, const UniformDiskFastParameter<T>& p) -> T {
    constexpr auto pi{std::numbers::pi_v<Scalar>};
    const auto r{p.Radius() * std::sqrt(1 - Uniform<Scalar>{}(g))}; // u in [0,1) so 1-u in (0,1], we don't like coordinate singularity at the center of the disk
    const auto [sinx, cosx]{impl::FastSinCosOnPiPi(Uniform<Scalar>{-pi, pi}(g))};
    return {p.CenterX() + r * cosx,
            p.CenterY() + r * sinx};
}

template<Concept::NumericVector2FloatingPoint T>
constexpr auto UniformDiskS<T>::Impl(auto& g, const UniformDiskParameter<T>& p) -> T {
    T r;
    VectorValueType<T> r2;
    do {
        r = UniformRectangle<T>({-0.5, 0.5}, {-0.5, 0.5})(g);
        r2 = muc::hypot_sq(r[0], r[1]);
    } while (r2 > 0.25);
    if constexpr (Concept::MathVector2Any<T>) {
        r = 2 * p.Radius() * r + p.Center();
    } else {
        r[0] = 2 * p.Radius() * r[0] + p.Center()[0];
        r[1] = 2 * p.Radius() * r[1] + p.Center()[1];
    }
    return r;
}

} // namespace Mustard::inline Math::Random::inline Distribution
