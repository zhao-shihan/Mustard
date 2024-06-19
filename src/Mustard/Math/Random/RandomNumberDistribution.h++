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

#pragma once

#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Concept/StreamIOable.h++"
#include "Mustard/Math/Random/Generator/Xoshiro512SS.h++"

#include <concepts>
#include <istream>
#include <ostream>
#include <type_traits>

namespace Mustard::Math::Random {

/// @brief Sub-requirements on D::param_type of RandomNumberDistribution (a C++
/// named requirements).
/// See also: https://en.cppreference.com/w/cpp/named_req/RandomNumberDistribution
template<typename P>
concept STDDistributionParameter =
    requires {
        // 1. They said: "D::param_type (aka P) satisfies CopyConstructible."
        // 2. They said: "D::param_type (aka P) satisfies CopyAssignable."
        requires std::copyable<P>;
        // 3. They said: "D::param_type (aka P) satisfies EqualityComparable."
        requires std::equality_comparable<P>;
        // 4. They said: "D::param_type (aka P) has a constructor taking identical
        // arguments as each of the constructors of D that take arguments
        // corresponding to the distribution parameters."
        // -- Semantic only
        // 5. They said: "D::param_type (aka P) has a member function with the
        // identical name, type, and semantics, as every member function of D that
        // returns a parameter of the distribution."
        // -- Semantic only
        // 6. They said: "D::param_type (aka P) declares a member typedef:
        // using distribution_type = D;"
        typename P::distribution_type;
    };

/// @brief Sub-requirements on D::param_type of RandomNumberDistribution (a C++
/// named requirements).
/// See also: https://en.cppreference.com/w/cpp/named_req/RandomNumberDistribution
template<typename P, typename D>
concept STDDistributionParameterOf =
    requires {
        // 1. Previous requirements.
        requires STDDistributionParameter<P>;
        // 2. The member typedef distribution_type is same as D.
        requires std::same_as<typename P::distribution_type, D>;
        // 3. D satisfies RandomNumberDistribution, where they said: "D::param_type
        // must be valid.". It's clear that D::param_type should be the same as P.
        typename D::param_type;
        requires std::same_as<P, typename D::param_type>;
    };

/// @brief C++ named requirements: RandomNumberDistribution.
/// See also: https://en.cppreference.com/w/cpp/named_req/RandomNumberDistribution
template<typename D>
concept STDRandomNumberDistribution =
    requires(D d, const D x) {
        // 1. They said: "D satisfies CopyConstructible."
        // 2. They said: "D satisfies CopyAssignable."
        requires std::copyable<D>;
        // 3. They said: "D::result_type must be valid, it is an arithmetic type."
        // -- Weakened, allow D::result_type to be a numeric vector.
        typename D::result_type;
        requires(std::is_arithmetic_v<typename D::result_type> or
                 Concept::NumericVectorAny<typename D::result_type>);
        // 4. They said: "D::param_type must be valid, it satisfies 'some
        // requirements'."
        typename D::param_type;
        requires STDDistributionParameterOf<typename D::param_type, D>;
        // 5. They said: "D() must be valid and creates a distribution
        // indistinguishable from any other default-constructed D."
        requires std::default_initializable<D>;
        // 6. They said: "Given p, a (possibly const) value of type D::param_type,
        // D(p) must be valid and creates a distribution indistinguishable from D
        // constructed directly from the values used to construct p."
        requires std::constructible_from<const typename D::param_type>;
        // 7. They said: "Given d, a value of D, expression d.reset() must be
        // valid and its value type is void. Expression d.reset() resets the
        // internal state of the distribution. The next call to operator() on
        // d will not depend on values produced by any engine prior to reset()."
        { d.reset() } -> std::same_as<void>;
        // 8. They said: "Given x, a (possibly const) value of D, expression
        // x.param() must be valid and its value type is D::param_type. Expression
        // x.param() returns p such that D(p).param() == p.
        { x.param() } -> std::same_as<typename D::param_type>;
        // 9. They said: "Given d, a value of D, and given p, a (possibly const)
        // value of type D::param_type, expression d.param(p) must be
        // valid and its value type is void. The postcondition of d.param(p) is
        // d.param() == p.
        requires requires(const typename D::param_type p) {
            { d.param(p) } -> std::same_as<void>;
        };
        // 10. They said: "Given g, lvalues of a type satisfying
        // UniformRandomBitGenerator, expression d(g) must be valid. The result of
        // expression d(g) has type of D::result_type. The sequence of numbers
        // returned by successive invocations of expression d(g) with the same g
        // are randomly distributed according to the distribution parametrized by
        // d.param().
        requires requires(Xoshiro512SS g) {
            { d(g) } -> std::same_as<typename D::result_type>;
        };
        // 11. They said: "Given g, lvalues of a type satisfying
        // UniformRandomBitGenerator, and given p, a (possibly const) value of type
        // D::param_type, expression d(g, p) must be valid. The sequence of numbers
        // returned by successive invocations of d(g, p) with the same g are
        // randomly distributed according to the distribution parametrized by p.
        requires requires(Xoshiro512SS g, const typename D::param_type p) {
            { d(g, p) } -> std::same_as<typename D::result_type>;
        };
        // 12. They said: "Given x, a (possibly const) value of D, expression
        // x.min() must be valid. It returns the greatest lower bound on the values
        // potentially returned by x’s operator(), as determined by the current
        // values of x’s parameters.
        { x.min() } -> std::same_as<typename D::result_type>;
        // 13. They said: "Given x, a (possibly const) value of D, expression
        // x.max() must be valid. It returns the least upper bound on the values
        // potentially returned by x’s operator(), as determined by the current
        // values of x’s parameters.
        { x.max() } -> std::same_as<typename D::result_type>;
        // 14. They said: "Given x and y, (possibly const) values of type D, given
        // g1 and g2, lvalues of a type satisfying UniformRandomBitGenerator,
        // expression x == y must be valid. It Establishes an equivalence relation.
        // It returns true if x.param() == y.param() and future infinite sequences
        // of values that would be generated by repeated invocations of x(g1) and
        // y(g2) would be equal as long as g1 == g2. Futhermore, they said: "Given
        // x and y, (possibly const) values of type D, expression x != y must be
        // valid. It returns a bool such that (x == y) == !(x != y)."
        requires std::equality_comparable<D>;
        // 15. They said: "Given x, a (possibly const) value of D, given os, a
        // lvalue of a specialization of std::basic_ostream, expression os << x
        // must be valid. Expression os << x returns a reference to the type of
        // os. Expression os << x writes a textual representation of the
        // distribution parameters and internal state to os. The formatting flags
        // and fill character of os are unchanged."
        // 16. They said: "Given d, a value of type D, given is, a lvalue of a
        // specialization of std::basic_istream, expression is >> d must be valid.
        // Expression is >> d returns a reference to the type of is. Expression
        // is >> d restores the distribution parameters and internal state with
        // data read from is. The formatting flags of is are unchanged. The data
        // must have been written using a stream with the same locale, CharT and
        // Traits stream template parameters, otherwise the behavior is undefined.
        // If bad input is encountered, is.setstate(std::ios::failbit) is called,
        // which may throw std::ios_base::failure. d is unchanged in that case."
        requires Concept::StreamIOable<D>;
    };

template<typename ADerived, typename ADistribution>
class DistributionParameterBase;

template<typename P>
concept DistributionParameter =
    requires {
        // 1. Same as the sub-requirements on D::param_type of
        // RandomNumberDistribution (a C++ named requirements).
        requires STDDistributionParameter<P>;
        // 2. Same as the C++ requirements but in our convention:
        // They said: "D::param_type (aka P) declares a member typedef:
        // using distribution_type = D;"
        typename P::DistributionType;
        // 3. It is TriviallyCopyable (a C++ named requirements).
        requires std::is_trivially_copyable_v<P>;
        // 4. It can be input and output.
        requires Concept::StreamIOable<P>;
        // 5. Extra requirements.
        requires std::derived_from<P, DistributionParameterBase<P, typename P::DistributionType>>;
        requires std::is_final_v<P>;
    };

template<typename P, typename D>
concept DistributionParameterOf =
    requires {
        // 1. Same as the sub-requirements on D::param_type of
        // RandomNumberDistribution (a C++ named requirements).
        requires STDDistributionParameterOf<P, D>;
        // 2. Previous requirements.
        requires DistributionParameter<P>;
        // 3. Same as the C++ requirements but in our convention.
        // 3.1 They said: "D::param_type (aka P) declares a member typedef:
        // using distribution_type = D;"
        requires std::same_as<typename P::DistributionType, D>;
        // 3.2. D satisfies RandomNumberDistribution, where they said:
        // "D::param_type must be valid.". It's clear that D::param_type should
        // be the same as P.
        typename D::ParameterType;
        requires std::same_as<P, typename D::ParameterType>;
    };

template<typename ADerived, typename AParameter, typename T>
    requires(std::is_arithmetic_v<T> or Concept::NumericVectorAny<T>)
class RandomNumberDistributionBase;

template<typename D>
concept RandomNumberDistribution =
    requires(D d, const D x) {
        // 1. C++ named requirements: RandomNumberDistribution.
        requires STDRandomNumberDistribution<D>;
        // 2. Same as the C++ named requirements but in our convention.
        // 2.1. D satisfies CopyConstructible. -- OK
        // 2.2. D satisfies CopyAssignable. -- OK
        // 2.3. D::ResultType must be valid, it is same as D::result_type.
        typename D::ResultType;
        requires std::same_as<typename D::ResultType, typename D::result_type>;
        // 2.4. D::ParameterType must be valid, it is same as D::param_type.
        typename D::ParameterType;
        requires std::same_as<typename D::ParameterType, typename D::param_type>;
        requires DistributionParameterOf<typename D::ParameterType, D>;
        // 2.5. D() must be valid and creates a distribution
        // indistinguishable from any other default-constructed D. -- OK
        // 2.6. Given p, a (possibly const) value of type D::ParameterType,
        // D(p) must be valid and creates a distribution indistinguishable from D
        // constructed directly from the values used to construct p. -- OK
        // 2.7. Given d, a value of D, expression d.Reset() must be
        // valid and its value type is void. Expression d.Reset() resets the
        // internal state of the distribution. The next call to operator() on
        // d will not depend on values produced by any engine prior to reset().
        { d.Reset() } -> std::same_as<void>;
        // 2.8. Given x, a (possibly const) value of D, expression
        // x.Parameter() must be valid and its value type is D::ParameterType. Expression
        // x.Parameter() returns p such that D(p).Parameter() == p.
        { x.Parameter() } -> std::same_as<typename D::ParameterType>;
        // 2.9. Given d, a value of D, and given p, a (possibly const)
        // value of type D::ParameterType, expression d.Parameter(p) must be
        // valid and its value type is void. The postcondition of d.Parameter(p) is
        // d.Parameter() == p.
        requires requires(const typename D::ParameterType p) {
            { d.Parameter(p) } -> std::same_as<void>;
        };
        // 2.10. Given g, lvalues of a type satisfying
        // UniformRandomBitGenerator, expression d(g) must be valid. The result of
        // expression d(g) has type of D::ResultType. The sequence of numbers
        // returned by successive invocations of expression d(g) with the same g
        // are randomly distributed according to the distribution parametrized by
        // d.Parameter(). -- OK
        // 2.11. Given g, lvalues of a type satisfying
        // UniformRandomBitGenerator, and given p, a (possibly const) value of type
        // D::ParameterType, expression d(g, p) must be valid. The sequence of numbers
        // returned by successive invocations of d(g, p) with the same g are
        // randomly distributed according to the distribution parametrized by p. -- OK
        // 2.12. Given x, a (possibly const) value of D, expression
        // x.Min() must be valid. It returns the greatest lower bound on the values
        // potentially returned by x’s operator(), as determined by the current
        // values of x’s parameters.
        { x.Min() } -> std::same_as<typename D::ResultType>;
        // 2.13. Given x, a (possibly const) value of D, expression
        // x.Max() must be valid. It returns the least upper bound on the values
        // potentially returned by x’s operator(), as determined by the current
        // values of x’s parameters.
        { x.Max() } -> std::same_as<typename D::ResultType>;
        // 2.14. Given x and y, (possibly const) values of type D, given
        // g1 and g2, lvalues of a type satisfying UniformRandomBitGenerator,
        // expression x == y must be valid. It Establishes an equivalence relation.
        // It returns true if x.Parameter() == y.Parameter() and future infinite
        // sequences of values that would be generated by repeated invocations of
        // x(g1) and y(g2) would be equal as long as g1 == g2. Futhermore, Given x
        // and y, (possibly const) values of type D, expression x == y must be
        // valid. It returns a bool such that (x == y) == !(x != y). -- OK
        // 2.16. Given x, a (possibly const) value of D, given os, a
        // lvalue of a specialization of std::basic_ostream, expression os << x
        // must be valid. Expression os << x returns a reference to the type of
        // os. Expression os << x writes a textual representation of the
        // distribution parameters and internal state to os. The formatting flags
        // and fill character of os are unchanged. -- OK
        // 2.17. Given d, a value of type D, given is, a lvalue of a
        // specialization of std::basic_istream, expression is >> d must be valid.
        // Expression is >> x returns a reference to the type of is. Expression
        // is << x restores the distribution parameters and internal state with
        // data read from is. The formatting flags of is are unchanged. The data
        // must have been written using a stream with the same locale, CharT and
        // Traits stream template parameters, otherwise the behavior is undefined.
        // If bad input is encountered, is.setstate(std::ios::failbit) is called,
        // which may throw std::ios_base::failure. d is unchanged in that case.
        // -- OK
        // 3. It has static member function Stateless(), indicates whether the
        // distribution implementation is stateless. Stateless() is constexpr.
        // If D::Stateless() returns true, a call of D::Reset() has no observable
        // effect on the distribution object. Otherwise, D::Stateless() returns
        // false, i.e. a call of D::Reset() has observable effects on the
        // distribution object.
        { D::Stateless() } -> std::same_as<bool>;
        requires std::integral<std::conditional_t<D::Stateless(), int, long>>;
        // 4. It is TriviallyCopyable (a C++ named requirements).
        requires std::is_trivially_copyable_v<D>;
        // 5. D::ParameterType cannot implicitly convertible to D.
        requires not std::convertible_to<typename D::ParameterType, D>;
        // 6. Extra requirements.
        requires std::derived_from<D, RandomNumberDistributionBase<D, typename D::ParameterType, typename D::ResultType>>;
        requires std::is_final_v<D>;
    };

} // namespace Mustard::Math::Random
