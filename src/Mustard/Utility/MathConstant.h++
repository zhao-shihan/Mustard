#pragma once

#include <numbers>

namespace Mustard::inline Utility::MathConstant {

using std::numbers::e;
using std::numbers::egamma;
using std::numbers::inv_pi;
using std::numbers::inv_sqrt3;
using std::numbers::inv_sqrtpi;
using std::numbers::ln10;
using std::numbers::ln2;
using std::numbers::log10e;
using std::numbers::log2e;
using std::numbers::phi;
using std::numbers::pi;
using std::numbers::sqrt2;
using std::numbers::sqrt3;

inline constexpr auto e_f = std::numbers::e_v<float>;
inline constexpr auto log2e_f = std::numbers::log2e_v<float>;
inline constexpr auto log10e_f = std::numbers::log10e_v<float>;
inline constexpr auto pi_f = std::numbers::pi_v<float>;
inline constexpr auto inv_pi_f = std::numbers::inv_pi_v<float>;
inline constexpr auto inv_sqrtpi_f = std::numbers::inv_sqrtpi_v<float>;
inline constexpr auto ln2_f = std::numbers::ln2_v<float>;
inline constexpr auto ln10_f = std::numbers::ln10_v<float>;
inline constexpr auto sqrt2_f = std::numbers::sqrt2_v<float>;
inline constexpr auto sqrt3_f = std::numbers::sqrt3_v<float>;
inline constexpr auto inv_sqrt3_f = std::numbers::inv_sqrt3_v<float>;
inline constexpr auto egamma_f = std::numbers::egamma_v<float>;
inline constexpr auto phi_f = std::numbers::phi_v<float>;

inline constexpr auto e_l = std::numbers::e_v<long double>;
inline constexpr auto log2e_l = std::numbers::log2e_v<long double>;
inline constexpr auto log10e_l = std::numbers::log10e_v<long double>;
inline constexpr auto pi_l = std::numbers::pi_v<long double>;
inline constexpr auto inv_pi_l = std::numbers::inv_pi_v<long double>;
inline constexpr auto inv_sqrtpi_l = std::numbers::inv_sqrtpi_v<long double>;
inline constexpr auto ln2_l = std::numbers::ln2_v<long double>;
inline constexpr auto ln10_l = std::numbers::ln10_v<long double>;
inline constexpr auto sqrt2_l = std::numbers::sqrt2_v<long double>;
inline constexpr auto sqrt3_l = std::numbers::sqrt3_v<long double>;
inline constexpr auto inv_sqrt3_l = std::numbers::inv_sqrt3_v<long double>;
inline constexpr auto egamma_l = std::numbers::egamma_v<long double>;
inline constexpr auto phi_l = std::numbers::phi_v<long double>;

} // namespace Mustard::inline Utility::MathConstant
