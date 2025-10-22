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

#pragma once

#include "muc/array"
#include "muc/math"

#include <cmath>
#include <numbers>

namespace Mustard::Math {

/// @brief Represents a value with associated uncertainty for error propagation
///
/// The Estimate struct encapsulates a value and its uncertainty,
/// providing mathematical operations that properly propagate uncertainties
/// according to standard error propagation rules.
struct Estimate {
    double value;
    double uncertainty;
};

/// @brief Add two estimates with uncertainty propagation
/// @param a First estimate
/// @param b Second estimate
/// @return Sum with combined uncertainty: √(σₐ² + σ_b²)
inline auto operator+(const Estimate& a, const Estimate& b) -> Estimate {
    return {a.value + b.value, muc::hypot(a.uncertainty, b.uncertainty)};
}

/// @brief Subtract two estimates with uncertainty propagation
/// @param a First estimate
/// @param b Second estimate
/// @return Difference with combined uncertainty: √(σₐ² + σ_b²)
inline auto operator-(const Estimate& a, const Estimate& b) -> Estimate {
    return {a.value - b.value, muc::hypot(a.uncertainty, b.uncertainty)};
}

/// @brief Multiply two estimates with uncertainty propagation
/// @param a First estimate
/// @param b Second estimate
/// @return Product with relative uncertainty: √((b·σₐ)² + (a·σ_b)²)
inline auto operator*(const Estimate& a, const Estimate& b) -> Estimate {
    return {a.value * b.value, muc::hypot(b.value * a.uncertainty, a.value * b.uncertainty)};
}

/// @brief Divide two estimates with uncertainty propagation
/// @param a Numerator estimate
/// @param b Denominator estimate
/// @return Quotient with relative uncertainty: √((b·σₐ)² + (a·σ_b)²)/b²
inline auto operator/(const Estimate& a, const Estimate& b) -> Estimate {
    return {a.value / b.value, muc::hypot(b.value * a.uncertainty, a.value * b.uncertainty) / muc::pow(b.value, 2)};
}

/// @brief Add constant to estimate (uncertainty unchanged)
constexpr auto operator+(const Estimate& a, double x) -> Estimate {
    return {a.value + x, a.uncertainty};
}

/// @brief Add estimate to constant (uncertainty unchanged)
constexpr auto operator+(double x, const Estimate& a) -> Estimate {
    return {x + a.value, a.uncertainty};
}

/// @brief Subtract constant from estimate (uncertainty unchanged)
constexpr auto operator-(const Estimate& a, double x) -> Estimate {
    return {a.value - x, a.uncertainty};
}

/// @brief Subtract estimate from constant (uncertainty unchanged)
constexpr auto operator-(double x, const Estimate& a) -> Estimate {
    return {x - a.value, a.uncertainty};
}

/// @brief Multiply estimate by constant (uncertainty scaled)
constexpr auto operator*(const Estimate& a, double x) -> Estimate {
    return {a.value * x, a.uncertainty * x};
}

/// @brief Multiply constant by estimate (uncertainty scaled)
constexpr auto operator*(double x, const Estimate& a) -> Estimate {
    return {x * a.value, x * a.uncertainty};
}

/// @brief Divide estimate by constant (uncertainty scaled)
constexpr auto operator/(const Estimate& a, double x) -> Estimate {
    return {a.value / x, a.uncertainty / x};
}

/// @brief Divide constant by estimate
/// @return Reciprocal with uncertainty: |x/a²|·σₐ
constexpr auto operator/(double x, const Estimate& a) -> Estimate {
    return {x / a.value, muc::abs(x / muc::pow(a.value, 2)) * a.uncertainty};
}

/// @brief Exponential function with uncertainty propagation
/// @return exp(a) with uncertainty: exp(a)·σₐ
inline auto exp(const Estimate& a) -> Estimate {
    const auto expA{std::exp(a.value)};
    return {expA, expA * a.uncertainty};
}

/// @brief Base-2 exponential with uncertainty propagation
/// @return 2ᵃ with uncertainty: 2ᵃ·ln(2)·σₐ
inline auto exp2(const Estimate& a) -> Estimate {
    const auto exp2A{std::exp2(a.value)};
    return {exp2A, exp2A * std::numbers::ln2 * a.uncertainty};
}

/// @brief Exponential minus one with uncertainty propagation
/// @return exp(a)-1 with uncertainty: exp(a)·σₐ
inline auto expm1(const Estimate& a) -> Estimate {
    const auto expM1A{std::expm1(a.value)};
    return {expM1A, (expM1A + 1) * a.uncertainty};
}

/// @brief Natural logarithm with uncertainty propagation
/// @return ln(a) with uncertainty: σₐ/a
inline auto log(const Estimate& a) -> Estimate {
    return {std::log(a.value), a.uncertainty / a.value};
}

/// @brief Base-10 logarithm with uncertainty propagation
/// @return log₁₀(a) with uncertainty: σₐ/(a·ln(10))
inline auto log10(const Estimate& a) -> Estimate {
    return {std::log10(a.value), a.uncertainty / (a.value * std::numbers::ln10)};
}

/// @brief Base-2 logarithm with uncertainty propagation
/// @return log₂(a) with uncertainty: σₐ/(a·ln(2))
inline auto log2(const Estimate& a) -> Estimate {
    return {std::log2(a.value), a.uncertainty / (a.value * std::numbers::ln2)};
}

/// @brief Natural logarithm of (1+a) with uncertainty propagation
/// @return ln(1+a) with uncertainty: σₐ/(1+a)
inline auto log1p(const Estimate& a) -> Estimate {
    return {std::log1p(a.value), a.uncertainty / (1 + a.value)};
}

/// @brief Power function with integer exponent
/// @return aⁿ with uncertainty: n·aⁿ·σₐ/a
inline auto pow(const Estimate& a, int n) -> Estimate {
    const auto powAN{muc::pow(a.value, n)};
    return {powAN, n * powAN * a.uncertainty / a.value};
}

/// @brief Power function with real exponent
/// @return aˣ with uncertainty: x·aˣ·σₐ/a
inline auto pow(const Estimate& a, double x) -> Estimate {
    const auto powAX{std::pow(a.value, x)};
    return {powAX, x * powAX * a.uncertainty / a.value};
}

/// @brief Square root with uncertainty propagation
/// @return √a with uncertainty: σₐ/(2√a)
inline auto sqrt(const Estimate& a) -> Estimate {
    const auto sqrtA{std::sqrt(a.value)};
    return {sqrtA, a.uncertainty / (2 * sqrtA)};
}

/// @brief Cube root with uncertainty propagation
/// @return ∛a with uncertainty: σₐ/(3(∛a)²)
inline auto cbrt(const Estimate& a) -> Estimate {
    const auto cbrtA{std::cbrt(a.value)};
    return {cbrtA, a.uncertainty / (3 * muc::pow(cbrtA, 2))};
}

/// @brief Sine function with uncertainty propagation
/// @return sin(a) with uncertainty: √(1-sin²(a))·σₐ
inline auto sin(const Estimate& a) -> Estimate {
    const auto sinA{std::sin(a.value)};
    return {sinA, std::sqrt(1 - muc::pow(sinA, 2)) * a.uncertainty};
}

/// @brief Cosine function with uncertainty propagation
/// @return cos(a) with uncertainty: √(1-cos²(a))·σₐ
inline auto cos(const Estimate& a) -> Estimate {
    const auto cosA{std::cos(a.value)};
    return {cosA, std::sqrt(1 - muc::pow(cosA, 2)) * a.uncertainty};
}

/// @brief Tangent function with uncertainty propagation
/// @return tan(a) with uncertainty: σₐ/cos²(a)
inline auto tan(const Estimate& a) -> Estimate {
    return {std::tan(a.value), a.uncertainty / muc::pow(std::cos(a.value), 2)};
}

/// @brief Arcsine function with uncertainty propagation
/// @return arcsin(a) with uncertainty: σₐ/√(1-a²)
inline auto asin(const Estimate& a) -> Estimate {
    return {std::asin(a.value), a.uncertainty / std::sqrt(1 - muc::pow(a.value, 2))};
}

/// @brief Arccosine function with uncertainty propagation
/// @return arccos(a) with uncertainty: σₐ/√(1-a²)
inline auto acos(const Estimate& a) -> Estimate {
    return {std::acos(a.value), a.uncertainty / std::sqrt(1 - muc::pow(a.value, 2))};
}

/// @brief Arctangent function with uncertainty propagation
/// @return arctan(a) with uncertainty: σₐ/(1+a²)
inline auto atan(const Estimate& a) -> Estimate {
    return {std::atan(a.value), a.uncertainty / (1 + muc::pow(a.value, 2))};
}

/// @brief Hyperbolic sine with uncertainty propagation
/// @return sinh(a) with uncertainty: cosh(a)·σₐ
inline auto sinh(const Estimate& a) -> Estimate {
    return {std::sinh(a.value), std::cosh(a.value) * a.uncertainty};
}

/// @brief Hyperbolic cosine with uncertainty propagation
/// @return cosh(a) with uncertainty: sinh(a)·σₐ
inline auto cosh(const Estimate& a) -> Estimate {
    return {std::cosh(a.value), std::sinh(a.value) * a.uncertainty};
}

/// @brief Hyperbolic tangent with uncertainty propagation
/// @return tanh(a) with uncertainty: σₐ/cosh²(a)
inline auto tanh(const Estimate& a) -> Estimate {
    return {std::tanh(a.value), a.uncertainty / muc::pow(std::cosh(a.value), 2)};
}

/// @brief Inverse hyperbolic sine with uncertainty propagation
/// @return asinh(a) with uncertainty: σₐ/√(1+a²)
inline auto asinh(const Estimate& a) -> Estimate {
    return {std::asinh(a.value), a.uncertainty / std::sqrt(1 + muc::pow(a.value, 2))};
}

/// @brief Inverse hyperbolic cosine with uncertainty propagation
/// @return acosh(a) with uncertainty: σₐ/√(a²-1)
inline auto acosh(const Estimate& a) -> Estimate {
    return {std::acosh(a.value), a.uncertainty / std::sqrt(muc::pow(a.value, 2) - 1)};
}

/// @brief Inverse hyperbolic tangent with uncertainty propagation
/// @return atanh(a) with uncertainty: σₐ/(1-a²)
inline auto atanh(const Estimate& a) -> Estimate {
    return {std::atanh(a.value), a.uncertainty / (1 - muc::pow(a.value, 2))};
}

} // namespace Mustard::Math
