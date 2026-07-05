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

#pragma once

#include "Mustard/Math/Random/UniformRandomBitGenerator.h++"

#include "muc/random"

#include <concepts>
#include <random>
#include <type_traits>

namespace Mustard::inline Math::Random {

/// @brief Concept of uniform pseudo-random number generator.
/// @note This is more constrained than C++ named requirement RandomNumberEngine.
/// Use muc::random_number_engine for a fully standard-compliant interface.
template<typename E>
concept RandomNumberEngine = requires(E& e) {
    // 1. C++ named requirements: RandomNumberEngine.
    requires muc::random_number_engine<E>;
    // 2. Same as the C++ named requirements but in our convention.
    // 2.1. They said: "A random number engine is a function object returning
    // unsigned integer values such that each value in the range of possible
    // results has (ideally) equal probability. Any random number engine is
    // also a UniformRandomBitGenerator."
    requires UniformRandomBitGenerator<E>;
    // 2.2. They said: "E satisfies CopyConstructible." -- OK
    // 2.3. They said: "E satisfies CopyAssignable." -- OK
    // 2.4. They said: "E() must be valid and creates an engine with the same
    // initial state as all other default-constructed engines of type E." -- OK
    // 2.5. They said: "E(x) creates an engine that compares equal to x."
    // -- OK
    // 2.6. They said: "E(s) creates an engine whose initial state is determined
    // by s." -- OK
    // 2.7. They said: "e.seed() must be valid and its value type is void.
    // Postcondition: e == E()."
    { e.Seed() } -> std::same_as<void>;
    // 2.8. They said: "e.seed(s) must be valid and its value type is void.
    // Postcondition: e == E(s)." -- Note: this generally assumes
    // typename E::SeedType (see below) is implicitly convertible to
    // typename E::ResultType, since it is usually decalred as E::Seed(SeedType).
    requires requires(typename E::ResultType s) {
        { e.Seed(s) } -> std::same_as<void>;
    };
    // 2.9. They said: "e.discard(z) must be valid and its value type is void.
    // Expression e.discard(z) advances e's state from e_i to e_{i+z} by any
    // means equivalent to z consecutive calls of e()."
    requires requires(unsigned long long z) {
        { e.Discard(z) } -> std::same_as<void>;
    };
    // 10. They said: "Given x and y, (possibly const) values of type E, given
    // g1 and g2, lvalues of a type satisfying UniformRandomBitGenerator,
    // expression x == y must be valid. It returns true if for all positive
    // integer i, the ith consecutive calls of x(g1) and y(g2) return the
    // same value." Furthermore, they said: "Expression x != y must be
    // valid. It returns a bool such that (x == y) == !(x != y)." -- OK
    // 11. They said: "Given x, a (possibly const) value of E, given os, a
    // lvalue of a specialization of std::basic_ostream, expression os << x
    // must be valid. Expression os << x returns a reference to the type of
    // os. Expression os << x writes a textual representation of the engine's
    // current state to os. The formatting flags and fill character of os are
    // unchanged." -- OK
    // 12. They said: "Given v, a value of type E, given is, a lvalue of a
    // specialization of std::basic_istream, expression is >> v must be valid.
    // Expression is >> v returns a reference to the type of is. Expression
    // is >> v restores the engine's state with data read from is. The
    // formatting flags of is are unchanged. The data must have been written
    // using a stream with the same locale, CharT and Traits stream template
    // parameters, otherwise the behavior is undefined. If bad input is
    // encountered, is.setstate(std::ios::failbit) is called, which may throw
    // std::ios_base::failure. v is unchanged in that case." -- OK
    // 3. Extra requirements.
    // 3.1 Has SeedType. It is an unsigned integral type.
    typename E::SeedType;
    requires std::unsigned_integral<typename E::SeedType>;
    // 3.2 It is constructible from a seed, this constructor is explicit.
    requires std::constructible_from<E, const typename E::SeedType>;
    requires not std::convertible_to<typename E::SeedType&, E>;
    // 3.3 Can be seeded with a seed of SeedType.
    requires requires(const typename E::SeedType seed) {
        { e.Seed(seed) } -> std::same_as<void>;
    };
    // 3.4. It is TriviallyCopyable (a C++ named requirements).
    requires std::is_trivially_copyable_v<E>;
    // 3.5. It is a standard-layout type.
    requires std::is_standard_layout_v<E>;
};

} // namespace Mustard::inline Math::Random
