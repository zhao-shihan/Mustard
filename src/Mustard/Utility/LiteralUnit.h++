// -*- C++ -*-
//
// Copyright 2020-2025  The Mustard development team
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

#include "CLHEP/Units/SystemOfUnits.h"

#include "muc/math"

#include <numbers>

namespace Mustard::inline Utility::LiteralUnit {

#define MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION_IMPL(FloatType, fullSuffix, unit) \
    constexpr auto operator""_##fullSuffix(long double value)->FloatType {        \
        return value * static_cast<long double>(unit);                            \
    }                                                                             \
    constexpr auto operator""_##fullSuffix(unsigned long long value)->FloatType { \
        return value * static_cast<long double>(unit);                            \
    }
#define MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(suffix, unit)             \
    MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION_IMPL(float, suffix##_f, unit) \
    MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION_IMPL(double, suffix, unit)    \
    // MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION_IMPL(long double, suffix##_l, unit)

// Length [L] symbols
inline namespace Length {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(nm, CLHEP::nm)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(um, CLHEP::um)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mm, CLHEP::mm)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(cm, CLHEP::cm)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(m, CLHEP::m)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(km, CLHEP::km)

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(pc, CLHEP::pc)

} // namespace Length

// Area [L^2] symbols
inline namespace Area {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mm2, CLHEP::mm2)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(cm2, CLHEP::cm2)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(m2, CLHEP::m2)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(km2, CLHEP::km2)

} // namespace Area

// Volume [L^3] symbols
inline namespace Volume {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mm3, CLHEP::mm3)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(cm3, CLHEP::cm3)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(m3, CLHEP::m3)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(km3, CLHEP::km3)

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(L, CLHEP::L)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(dL, CLHEP::dL)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(cL, CLHEP::cL)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mL, CLHEP::mL)

} // namespace Volume

// Angle symbols
inline namespace Angle {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(rad, CLHEP::rad)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mrad, CLHEP::mrad)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(sr, CLHEP::sr)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(deg, CLHEP::deg)

} // namespace Angle

// Time [T] symbols
inline namespace Time {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(ns, CLHEP::ns)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(s, CLHEP::s)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(ms, CLHEP::ms)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(us, CLHEP::us)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(ps, CLHEP::ps)

} // namespace Time

// Energy [E] symbols
inline namespace Energy {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(MeV, CLHEP::MeV)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(eV, CLHEP::eV)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(keV, CLHEP::keV)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(GeV, CLHEP::GeV)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(TeV, CLHEP::TeV)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(PeV, CLHEP::PeV)

} // namespace Energy

// Mass [E][T^2][L^-2] symbols
inline namespace Mass {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(kg, CLHEP::kg)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(g, CLHEP::g)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mg, CLHEP::mg)

} // namespace Mass

// Power [E][T^-1] symbols
inline namespace Power {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(W, CLHEP::watt)

} // namespace Power

// Force [E][L^-1] symbols
inline namespace Force {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(N, CLHEP::newton)

} // namespace Force

// Pressure [E][L^-3] symbols
inline namespace Pressure {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(kPa, CLHEP::hep_pascal * 1'000)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(hPa, CLHEP::hep_pascal * 100)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(Pa, CLHEP::hep_pascal)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(bar, CLHEP::bar)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mbar, CLHEP::bar / 1'000)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(atm, CLHEP::atmosphere)

} // namespace Pressure

// Electric current [Q][T^-1] symbols
inline namespace ElectricCurrent {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(kA, CLHEP::ampere * 1'000)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(A, CLHEP::ampere)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mA, CLHEP::milliampere)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(uA, CLHEP::microampere)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(nA, CLHEP::nanoampere)

} // namespace ElectricCurrent

// Electric potential [E][Q^-1] symbols
inline namespace ElectricPotential {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(kV, CLHEP::kilovolt)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(V, CLHEP::volt)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mV, CLHEP::volt / 1'000)

} // namespace ElectricPotential

// Electric capacitance [Q^2][E^-1] symbols
inline namespace ElectricCapacitance {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(F, CLHEP::farad)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mF, CLHEP::millifarad)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(uF, CLHEP::microfarad)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(nF, CLHEP::nanofarad)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(pF, CLHEP::picofarad)

} // namespace ElectricCapacitance

// Magnetic Flux [T][E][Q^-1] symbols
inline namespace MagneticFlux {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(Wb, CLHEP::weber)

} // namespace MagneticFlux

// Magnetic flux density [T][E][Q^-1][L^-2] symbols
inline namespace MagneticFluxDensity {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(T, CLHEP::tesla)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mT, CLHEP::tesla / 1'000)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(uT, CLHEP::tesla / 1'000'000)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(nT, CLHEP::tesla / 1'000'000'000)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(kGs, CLHEP::gauss * 1000)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(Gs, CLHEP::gauss)

} // namespace MagneticFluxDensity

// Inductance [T^2][E][Q^-2] symbols
inline namespace Inductance {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(H, CLHEP::henry)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mH, CLHEP::henry / 1'000)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(uH, CLHEP::henry / 1'000'000)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(nH, CLHEP::henry / 1'000'000'000)

} // namespace Inductance

// Temperature symbols
inline namespace Temperature {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(K, CLHEP::kelvin)

} // namespace Temperature

// Amount of substance symbols
inline namespace AmountOfSubstance {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mol, CLHEP::mole)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(kg_mol, CLHEP::kg / CLHEP::mole)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(g_mol, CLHEP::g / CLHEP::mole)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mg_mol, CLHEP::mg / CLHEP::mole)

} // namespace AmountOfSubstance

// Activity [T^-1] symbols
inline namespace Activity {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(Bq, CLHEP::Bq)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(kBq, CLHEP::kBq)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(MBq, CLHEP::MBq)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(GBq, CLHEP::GBq)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(Ci, CLHEP::Ci)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mCi, CLHEP::mCi)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(uCi, CLHEP::uCi)

} // namespace Activity

// Absorbed dose [L^2][T^-2] symbols
inline namespace AbsorbedDose {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(Gy, CLHEP::gray)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(kGy, CLHEP::kilogray)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mGy, CLHEP::milligray)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(uGy, CLHEP::microgray)

} // namespace AbsorbedDose

// Luminous intensity [I] symbols
inline namespace LuminousIntensity {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(cd, CLHEP::candela)

} // namespace LuminousIntensity

// Luminous flux [I] symbols
inline namespace LuminousFlux {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(lm, CLHEP::lumen)

} // namespace LuminousFlux

// Illuminance [I][L^-2] symbols
inline namespace Illuminance {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(lux, CLHEP::lux)

} // namespace Illuminance

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

// Velocity [L][T^-1] symbols
inline namespace Velocity {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(km_ps, CLHEP::km / CLHEP::ps)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(m_ps, CLHEP::m / CLHEP::ps)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(cm_ps, CLHEP::cm / CLHEP::ps)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mm_ps, CLHEP::mm / CLHEP::ps)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(um_ps, CLHEP::um / CLHEP::ps)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(nm_ps, CLHEP::nm / CLHEP::ps)

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(km_ns, CLHEP::km / CLHEP::ns)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(m_ns, CLHEP::m / CLHEP::ns)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(cm_ns, CLHEP::cm / CLHEP::ns)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mm_ns, CLHEP::mm / CLHEP::ns)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(um_ns, CLHEP::um / CLHEP::ns)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(nm_ns, CLHEP::nm / CLHEP::ns)

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(km_us, CLHEP::km / CLHEP::us)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(m_us, CLHEP::m / CLHEP::us)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(cm_us, CLHEP::cm / CLHEP::us)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mm_us, CLHEP::mm / CLHEP::us)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(um_us, CLHEP::um / CLHEP::us)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(nm_us, CLHEP::nm / CLHEP::us)

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(km_ms, CLHEP::km / CLHEP::ms)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(m_ms, CLHEP::m / CLHEP::ms)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(cm_ms, CLHEP::cm / CLHEP::ms)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mm_ms, CLHEP::mm / CLHEP::ms)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(um_ms, CLHEP::um / CLHEP::ms)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(nm_ms, CLHEP::nm / CLHEP::ms)

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(km_s, CLHEP::km / CLHEP::s)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(m_s, CLHEP::m / CLHEP::s)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(cm_s, CLHEP::cm / CLHEP::s)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mm_s, CLHEP::mm / CLHEP::s)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(um_s, CLHEP::um / CLHEP::s)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(nm_s, CLHEP::nm / CLHEP::s)

} // namespace Velocity

// Density [E][T^2][L^-5] symbols
inline namespace Density {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(kg_m3, CLHEP::kg / CLHEP::m3)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(g_m3, CLHEP::g / CLHEP::m3)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(g_cm3, CLHEP::g / CLHEP::cm3)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mg_cm3, CLHEP::mg / CLHEP::cm3)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mg_mm3, CLHEP::mg / CLHEP::mm3)

} // namespace Density

// Surface density [E][T^2][L^-4] symbols
inline namespace SurfaceDensity {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(kg_m2, CLHEP::kg / CLHEP::m2)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(g_m2, CLHEP::g / CLHEP::m2)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(g_cm2, CLHEP::g / CLHEP::cm2)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mg_cm2, CLHEP::mg / CLHEP::cm2)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mg_mm2, CLHEP::mg / CLHEP::mm2)

} // namespace SurfaceDensity

// Frequency [T^-1] symbols
inline namespace Frequency {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(Hz, 1 / CLHEP::s)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(kHz, 1 / CLHEP::ms)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(MHz, 1 / CLHEP::us)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(GHz, 1 / CLHEP::ns)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(THz, 1 / CLHEP::ps)

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(s_1, 1 / CLHEP::s)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(ms_1, 1 / CLHEP::ms)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(us_1, 1 / CLHEP::us)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(ns_1, 1 / CLHEP::ns)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(ps_1, 1 / CLHEP::ps)

} // namespace Frequency

// Electric field strength [E][L^-1][Q^-1] symbols
inline namespace ElectricFieldStrength {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(V_m, CLHEP::volt / CLHEP::m)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(V_cm, CLHEP::volt / CLHEP::cm)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(kV_m, CLHEP::kilovolt / CLHEP::m)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(kV_cm, CLHEP::kilovolt / CLHEP::cm)

} // namespace ElectricFieldStrength

// Number density [L^-3] symbols
inline namespace NumberDensity {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mm_3, 1 / CLHEP::mm3)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(cm_3, 1 / CLHEP::cm3)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(m_3, 1 / CLHEP::m3)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(km_3, 1 / CLHEP::km3)

} // namespace NumberDensity

// Surface number density [L^-2] symbols
inline namespace SurfaceNumberDensity {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mm_2, 1 / CLHEP::mm2)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(cm_2, 1 / CLHEP::cm2)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(m_2, 1 / CLHEP::m2)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(km_2, 1 / CLHEP::km2)

} // namespace SurfaceNumberDensity

// Number flux [L^-2][T^-1] symbols
inline namespace NumberFlux {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(km_2_ps_1, muc::pow(CLHEP::km, -2) * muc::pow(CLHEP::ps, -1))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(m_2_ps_1, muc::pow(CLHEP::m, -2) * muc::pow(CLHEP::ps, -1))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(cm_2_ps_1, muc::pow(CLHEP::cm, -2) * muc::pow(CLHEP::ps, -1))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mm_2_ps_1, muc::pow(CLHEP::mm, -2) * muc::pow(CLHEP::ps, -1))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(um_2_ps_1, muc::pow(CLHEP::um, -2) * muc::pow(CLHEP::ps, -1))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(nm_2_ps_1, muc::pow(CLHEP::nm, -2) * muc::pow(CLHEP::ps, -1))

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(km_2_ns_1, muc::pow(CLHEP::km, -2) * muc::pow(CLHEP::ns, -2))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(m_2_ns_1, muc::pow(CLHEP::m, -2) * muc::pow(CLHEP::ns, -2))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(cm_2_ns_1, muc::pow(CLHEP::cm, -2) * muc::pow(CLHEP::ns, -2))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mm_2_ns_1, muc::pow(CLHEP::mm, -2) * muc::pow(CLHEP::ns, -2))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(um_2_ns_1, muc::pow(CLHEP::um, -2) * muc::pow(CLHEP::ns, -2))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(nm_2_ns_1, muc::pow(CLHEP::nm, -2) * muc::pow(CLHEP::ns, -2))

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(km_2_us_1, muc::pow(CLHEP::km, -2) * muc::pow(CLHEP::us, -1))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(m_2_us_1, muc::pow(CLHEP::m, -2) * muc::pow(CLHEP::us, -1))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(cm_2_us_1, muc::pow(CLHEP::cm, -2) * muc::pow(CLHEP::us, -1))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mm_2_us_1, muc::pow(CLHEP::mm, -2) * muc::pow(CLHEP::us, -1))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(um_2_us_1, muc::pow(CLHEP::um, -2) * muc::pow(CLHEP::us, -1))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(nm_2_us_1, muc::pow(CLHEP::nm, -2) * muc::pow(CLHEP::us, -1))

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(km_2_ms_1, muc::pow(CLHEP::km, -2) * muc::pow(CLHEP::ms, -1))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(m_2_ms_1, muc::pow(CLHEP::m, -2) * muc::pow(CLHEP::ms, -1))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(cm_2_ms_1, muc::pow(CLHEP::cm, -2) * muc::pow(CLHEP::ms, -1))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mm_2_ms_1, muc::pow(CLHEP::mm, -2) * muc::pow(CLHEP::ms, -1))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(um_2_ms_1, muc::pow(CLHEP::um, -2) * muc::pow(CLHEP::ms, -1))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(nm_2_ms_1, muc::pow(CLHEP::nm, -2) * muc::pow(CLHEP::ms, -1))

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(km_2_s_1, muc::pow(CLHEP::km, -2) * muc::pow(CLHEP::s, -1))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(m_2_s_1, muc::pow(CLHEP::m, -2) * muc::pow(CLHEP::s, -1))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(cm_2_s_1, muc::pow(CLHEP::cm, -2) * muc::pow(CLHEP::s, -1))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(mm_2_s_1, muc::pow(CLHEP::mm, -2) * muc::pow(CLHEP::s, -1))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(um_2_s_1, muc::pow(CLHEP::um, -2) * muc::pow(CLHEP::s, -1))
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(nm_2_s_1, muc::pow(CLHEP::nm, -2) * muc::pow(CLHEP::s, -1))

} // namespace NumberFlux

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

inline namespace MathConstantSuffix {

MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(log2e, std::numbers::log2e_v<long double>)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(log10e, std::numbers::log10e_v<long double>)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(pi, std::numbers::pi_v<long double>)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(inv_pi, std::numbers::inv_pi_v<long double>)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(inv_sqrtpi, std::numbers::inv_sqrtpi_v<long double>)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(log2, std::numbers::ln2_v<long double>)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(log10, std::numbers::ln10_v<long double>)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(sqrt2, std::numbers::sqrt2_v<long double>)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(sqrt3, std::numbers::sqrt3_v<long double>)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(inv_sqrt3, std::numbers::inv_sqrt3_v<long double>)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(egamma, std::numbers::egamma_v<long double>)
MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION(phi, std::numbers::phi_v<long double>)

} // namespace MathConstantSuffix

#undef MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION
#undef MUSTARD_UTILITY_LITERAL_UNIT_DEFINITION_IMPL

} // namespace Mustard::inline Utility::LiteralUnit
