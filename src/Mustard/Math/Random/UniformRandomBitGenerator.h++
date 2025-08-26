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

#pragma once

#include <concepts>
#include <random>
#include <type_traits>

namespace Mustard::Math::Random {

template<typename ADerived, std::unsigned_integral AResult>
class UniformRandomBitGeneratorBase;

/// @brief Concept of uniform random bit generator.
template<typename G>
concept UniformRandomBitGenerator =
    requires(G g) {
        // 1. C++ named requirements: UniformRandomBitGenerator.
        // See also: https://en.cppreference.com/w/cpp/named_req/UniformRandomBitGenerator
        requires std::uniform_random_bit_generator<G>;
        // 2. Same as the C++ named requirements but in our convention.
        // 2.1 Has ResultType. It is an unsigned integral type.
        typename G::ResultType;
        requires std::unsigned_integral<typename G::ResultType>;
        // 2.2 Has static member functions Min() and Max(), and Min() < Max().
        // They are constexpr.
        { G::Min() } -> std::same_as<typename G::ResultType>;
        { G::Max() } -> std::same_as<typename G::ResultType>;
        requires(G::Min() < G::Max());
        // 2.3 Has operator(). It returns a value of ResultType.
        { g() } -> std::same_as<typename G::ResultType>;
        // 3. Extra requirements.
        requires std::derived_from<G, UniformRandomBitGeneratorBase<G, typename G::ResultType>>;
        requires std::is_final_v<G>;
        requires std::default_initializable<G>;
    };

} // namespace Mustard::Math::Random
