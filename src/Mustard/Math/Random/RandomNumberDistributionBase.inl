// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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

namespace Mustard::Math::Random {

template<typename ADerived, typename ADistribution>
constexpr DistributionParameterBase<ADerived, ADistribution>::DistributionParameterBase() {
    static_assert(DistributionParameterOf<ADerived, ADistribution>);
    static_assert(std::is_final_v<ADerived>);
}

template<typename ADerived, typename AParameter, typename T>
    requires(std::is_arithmetic_v<T> or Concept::NumericVectorAny<T>)
constexpr RandomNumberDistributionBase<ADerived, AParameter, T>::RandomNumberDistributionBase() {
    static_assert(RandomNumberDistribution<ADerived>);
    static_assert(std::is_final_v<ADerived>);
}

} // namespace Mustard::Math::Random
