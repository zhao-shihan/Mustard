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

template<std::floating_point T, template<typename> typename AGaussian>
constexpr BasicGaussianSParameter<T, AGaussian>::BasicGaussianSParameter(T mu, T sigma) :
    Base{},
    fMu{mu},
    fSigma{sigma} {}

template<std::floating_point T, template<typename> typename AGaussian>
template<muc::character AChar>
auto BasicGaussianSParameter<T, AGaussian>::StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os) {
    const auto oldPrecision{os.precision(std::numeric_limits<T>::max_digits10)};
    return os << fMu << ' ' << fSigma
              << std::setprecision(oldPrecision);
}

template<std::floating_point T, template<typename> typename AGaussian>
template<muc::character AChar>
auto BasicGaussianSParameter<T, AGaussian>::StreamInput(std::basic_istream<AChar>& is) & -> decltype(is) {
    return is >> fMu >> fSigma;
}

template<std::floating_point T, template<typename> typename AGaussian>
constexpr GaussianBase<T, AGaussian>::GaussianBase(T mu, T sigma) :
    Base{},
    fParameter{mu, sigma} {}

template<std::floating_point T, template<typename> typename AGaussian>
constexpr GaussianBase<T, AGaussian>::GaussianBase(const typename Base::ParameterType& p) :
    Base{},
    fParameter{p} {}

} // namespace impl

#define MUSTARD_MATH_RANDOM_DISTRIBUTION_GAUSSIAN_IMPL_SNIPPET(Suffix)    \
    if ((fCached = not fCached)) {                                        \
        static_assert(Gaussian2D##Suffix<std::array<T, 2>>::Stateless()); \
        auto&& [u, v]{Gaussian2D##Suffix<std::array<T, 2>>{}(g)};         \
        fCachedValue = std::move(v);                                      \
        return p.Sigma() * u + p.Mu();                                    \
    } else {                                                              \
        return p.Sigma() * fCachedValue + p.Mu();                         \
    }

template<std::floating_point T>
auto Gaussian<T>::Impl(auto& g, const GaussianParameter<T>& p) -> T {
    MUSTARD_MATH_RANDOM_DISTRIBUTION_GAUSSIAN_IMPL_SNIPPET()
}

template<std::floating_point T>
auto GaussianS<T>::Impl(auto& g, const GaussianSParameter<T>& p) -> T {
    MUSTARD_MATH_RANDOM_DISTRIBUTION_GAUSSIAN_IMPL_SNIPPET(S)
}

#undef MUSTARD_MATH_RANDOM_DISTRIBUTION_GAUSSIAN_IMPL_SNIPPET

} // namespace Mustard::inline Math::Random::inline Distribution
