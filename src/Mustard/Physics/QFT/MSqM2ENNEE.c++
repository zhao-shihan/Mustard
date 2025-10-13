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

#include "Mustard/Physics/QFT/MSqM2ENNEE.h++"
#include "Mustard/Utility/MathConstant.h++"
#include "Mustard/Utility/PhysicalConstant.h++"

#include "CLHEP/Vector/LorentzVector.h"

#include "muc/math"

#include <cmath>

namespace Mustard::inline Physics::QFT {

using namespace PhysicalConstant;
using namespace MathConstant;

MSqM2ENNEE::MSqM2ENNEE(Ver ver) :
    PolarizedMatrixElement{},
    fVersion{ver} {}

auto MSqM2ENNEE::operator()(const InitialStateMomenta& pI, const FinalStateMomenta& pF) const -> double {
    switch (fVersion) {
    case Ver::McMule0Av:
        return MSqMcMule0Av(pI, pF);
    case Ver::McMuleLegacy:
        return MSqMcMuleLegacy(pI, pF);
    default:
        Throw<std::invalid_argument>("No such version");
    }
}

auto MSqM2ENNEE::MSqMcMule0Av(const InitialStateMomenta& pI, const FinalStateMomenta& pF) const -> double {
    const auto& p1{pI};
    const auto& [p2, _1, _2, p3, p4]{pF};

    // Adapt from McMule v0.5.1, mudecrare/mudecrare_mat_el.f95, FUNCTION PM2ENNEEav
    //
    // Copyright (C) 2020-2025  Yannick Ulrich and others (The McMule development team)
    //

    constexpr auto s{[](auto&& a, auto&& b) {
        return 2 * (a * b);
    }};

    const auto s13{s(p1, p3)};
    const auto m12{p1.m2()};
    const auto m32{p3.m2()};

    const auto polarized{InitialStatePolarization() != CLHEP::Hep3Vector{}};
    const auto sqrtM12{polarized ? std::sqrt(m12) : 0};

    double pm2enneeav{};
    const auto m2enneeavImpl{[&](const auto& p2, const auto& p4) {
        const auto s12{s(p1, p2)};
        const auto s14{s(p1, p4)};
        const auto s23{s(p2, p3)};
        const auto s24{s(p2, p4)};
        const auto s34{s(p3, p4)};
        const auto m22{p2.m2()};
        pm2enneeav += 2 * OneBorn(s12, s13, s14, s23, s24, s34, m12, m22, m32) +
                      TwoBorn(s12, s13, s14, s23, s24, s34, m12, m22, m32);
        if (polarized) {
            const CLHEP::HepLorentzVector n{-InitialStatePolarization()};
            const auto s2n{s(p2, n)};
            const auto s3n{s(p3, n)};
            const auto s4n{s(p4, n)};
            pm2enneeav += sqrtM12 *
                          (2 * OneBornPol(s12, s13, s14, s23, s24, s34, m12, m22, m32, s2n, s3n, s4n) +
                           TwoBornPol(s12, s13, s14, s23, s24, s34, m12, m22, m32, s2n, s3n, s4n));
        }
    }};
    m2enneeavImpl(p2, p4);
    m2enneeavImpl(p4, p2);

    return muc::pow(2 * pi * fine_structure_const * reduced_fermi_constant, 2) * pm2enneeav;
}

MUSTARD_OPTIMIZE_FAST auto MSqM2ENNEE::OneBorn(double s12, double s13, double s14, double s23, double s24, double s34,
                                               double m12, double m22, double) -> double {
    using muc::pow;

    // Adapt from McMule v0.5.1, mudecrare/mudecrare_1l_onetrace.opt.f95, FUNCTION bornPol
    //
    // Copyright (C) 2020-2025  Yannick Ulrich and others (The McMule development team)
    //

    const auto tmp3{2 * m22};
    const auto tmp7{-s13};
    const auto tmp17{pow(m12, 2)};
    const auto tmp18{pow(m22, 4)};
    const auto tmp20{pow(m22, 3)};
    const auto tmp25{pow(s12, 2)};
    const auto tmp29{pow(s12, 3)};
    const auto tmp28{pow(m22, 2)};
    const auto tmp23{pow(m22, 5)};
    const auto tmp41{pow(s13, 2)};
    const auto tmp47{pow(s13, 3)};
    const auto tmp70{pow(s14, 2)};
    const auto tmp79{pow(s14, 3)};
    const auto tmp32{pow(s12, 4)};
    const auto tmp51{pow(s13, 4)};
    const auto tmp84{pow(s14, 4)};
    const auto tmp142{pow(s23, 2)};
    const auto tmp187{pow(s23, 3)};
    const auto tmp211{pow(s23, 4)};
    const auto tmp368{pow(s24, 2)};
    const auto tmp451{pow(s24, 3)};
    const auto tmp478{pow(s24, 4)};
    const auto tmp777{pow(s34, 2)};
    const auto tmp926{pow(s34, 3)};
    const auto tmp982{pow(s34, 4)};
    return (-32 *
            (60 * m12 * m22 * s12 * s13 * s14 * s23 + 2 * s14 * pow(s23, 5) -
             48 * m12 * m22 * s12 * s13 * s14 * s24 +
             40 * m12 * m22 * s12 * s13 * s23 * s24 +
             44 * m12 * m22 * s13 * s14 * s23 * s24 -
             6 * m12 * s12 * s13 * s14 * s23 * s24 -
             54 * m22 * s12 * s13 * s14 * s23 * s24 +
             60 * m12 * m22 * s12 * s13 * s14 * s34 -
             56 * m12 * m22 * s12 * s13 * s23 * s34 -
             104 * m12 * m22 * s12 * s14 * s23 * s34 -
             56 * m12 * m22 * s13 * s14 * s23 * s34 +
             12 * m12 * s12 * s13 * s14 * s23 * s34 +
             284 * m22 * s12 * s13 * s14 * s23 * s34 +
             44 * m12 * m22 * s12 * s13 * s24 * s34 +
             40 * m12 * m22 * s13 * s14 * s24 * s34 -
             6 * m12 * s12 * s13 * s14 * s24 * s34 -
             54 * m22 * s12 * s13 * s14 * s24 * s34 -
             176 * m12 * m22 * s12 * s23 * s24 * s34 -
             100 * m12 * m22 * s13 * s23 * s24 * s34 +
             6 * m12 * s12 * s13 * s23 * s24 * s34 +
             62 * m22 * s12 * s13 * s23 * s24 * s34 -
             176 * m12 * m22 * s14 * s23 * s24 * s34 +
             16 * m12 * s12 * s14 * s23 * s24 * s34 -
             16 * m22 * s12 * s14 * s23 * s24 * s34 +
             6 * m12 * s13 * s14 * s23 * s24 * s34 +
             62 * m22 * s13 * s14 * s23 * s24 * s34 -
             16 * s12 * s13 * s14 * s23 * s24 * s34 + 2 * s12 * pow(s34, 5) -
             16 * m12 * m22 * s12 * s13 * tmp142 -
             64 * m12 * m22 * s12 * s14 * tmp142 -
             40 * m12 * m22 * s13 * s14 * tmp142 +
             6 * m12 * s12 * s13 * s14 * tmp142 +
             150 * m22 * s12 * s13 * s14 * tmp142 -
             68 * m12 * m22 * s12 * s24 * tmp142 -
             44 * m12 * m22 * s13 * s24 * tmp142 +
             4 * m12 * s12 * s13 * s24 * tmp142 +
             36 * m22 * s12 * s13 * s24 * tmp142 -
             46 * m12 * m22 * s14 * s24 * tmp142 +
             2 * m12 * s12 * s14 * s24 * tmp142 -
             58 * m22 * s12 * s14 * s24 * tmp142 +
             2 * m12 * s13 * s14 * s24 * tmp142 -
             10 * m22 * s13 * s14 * s24 * tmp142 -
             4 * s12 * s13 * s14 * s24 * tmp142 -
             108 * m12 * m22 * s12 * s34 * tmp142 -
             40 * m12 * m22 * s13 * s34 * tmp142 -
             2 * m12 * s12 * s13 * s34 * tmp142 -
             122 * m22 * s12 * s13 * s34 * tmp142 -
             74 * m12 * m22 * s14 * s34 * tmp142 -
             172 * m22 * s12 * s14 * s34 * tmp142 -
             4 * m12 * s13 * s14 * s34 * tmp142 -
             176 * m22 * s13 * s14 * s34 * tmp142 +
             28 * s12 * s13 * s14 * s34 * tmp142 +
             256 * m12 * m22 * s24 * s34 * tmp142 -
             22 * m12 * s12 * s24 * s34 * tmp142 -
             58 * m22 * s12 * s24 * s34 * tmp142 +
             36 * m22 * s13 * s24 * s34 * tmp142 +
             8 * s12 * s13 * s24 * s34 * tmp142 -
             24 * m12 * s14 * s24 * s34 * tmp142 +
             14 * s12 * s14 * s24 * s34 * tmp142 -
             2 * s13 * s14 * s24 * s34 * tmp142 +
             36 * m22 * s13 * s23 * s24 * tmp17 -
             24 * m22 * s12 * s23 * s34 * tmp17 +
             12 * m22 * s13 * s23 * s34 * tmp17 -
             24 * m22 * s14 * s23 * s34 * tmp17 +
             36 * m22 * s13 * s24 * s34 * tmp17 +
             72 * m22 * s23 * s24 * s34 * tmp17 +
             6 * s13 * s23 * s24 * s34 * tmp17 -
             6 * m22 * s12 * tmp142 * tmp17 + 6 * m22 * s13 * tmp142 * tmp17 -
             18 * m22 * s14 * tmp142 * tmp17 + 32 * m22 * s24 * tmp142 * tmp17 +
             3 * s13 * s24 * tmp142 * tmp17 + 104 * m22 * s34 * tmp142 * tmp17 -
             3 * s12 * s34 * tmp142 * tmp17 - 6 * s14 * s34 * tmp142 * tmp17 +
             4 * s24 * s34 * tmp142 * tmp17 + 160 * m12 * s12 * tmp18 +
             32 * m12 * s13 * tmp18 - 400 * s12 * s13 * tmp18 +
             160 * m12 * s14 * tmp18 - 880 * s12 * s14 * tmp18 -
             400 * s13 * s14 * tmp18 + 288 * m12 * s23 * tmp18 +
             472 * s12 * s23 * tmp18 + 440 * s13 * s23 * tmp18 +
             700 * s14 * s23 * tmp18 + 640 * m12 * s24 * tmp18 +
             16 * s12 * s24 * tmp18 - 196 * s13 * s24 * tmp18 +
             16 * s14 * s24 * tmp18 + 288 * m12 * s34 * tmp18 +
             700 * s12 * s34 * tmp18 + 440 * s13 * s34 * tmp18 +
             472 * s14 * s34 * tmp18 + 192 * tmp17 * tmp18 -
             32 * m12 * m22 * s12 * tmp187 - 12 * m12 * m22 * s13 * tmp187 -
             24 * m22 * s12 * s13 * tmp187 + 2 * m12 * m22 * s14 * tmp187 -
             4 * m12 * s12 * s14 * tmp187 - 80 * m22 * s12 * s14 * tmp187 -
             2 * m12 * s13 * s14 * tmp187 - 78 * m22 * s13 * s14 * tmp187 +
             12 * s12 * s13 * s14 * tmp187 + 64 * m12 * m22 * s24 * tmp187 -
             16 * m22 * s12 * s24 * tmp187 + 4 * m22 * s13 * s24 * tmp187 +
             6 * s12 * s13 * s24 * tmp187 - 2 * m12 * s14 * s24 * tmp187 +
             42 * m22 * s14 * s24 * tmp187 - 6 * s12 * s14 * s24 * tmp187 -
             4 * s13 * s14 * s24 * tmp187 + 128 * m12 * m22 * s34 * tmp187 -
             12 * m12 * s12 * s34 * tmp187 - 8 * m22 * s12 * s34 * tmp187 -
             4 * m12 * s13 * s34 * tmp187 + 68 * m22 * s13 * s34 * tmp187 -
             6 * s12 * s13 * s34 * tmp187 - 12 * m12 * s14 * s34 * tmp187 +
             56 * m22 * s14 * s34 * tmp187 - 8 * s12 * s14 * s34 * tmp187 -
             18 * s13 * s14 * s34 * tmp187 + 16 * m12 * s24 * s34 * tmp187 -
             8 * s12 * s24 * s34 * tmp187 + 4 * s13 * s24 * s34 * tmp187 +
             24 * m22 * tmp17 * tmp187 - 3 * s14 * tmp17 * tmp187 +
             8 * s34 * tmp17 * tmp187 - 128 * m12 * s12 * s13 * tmp20 -
             352 * m12 * s12 * s14 * tmp20 - 128 * m12 * s13 * s14 * tmp20 +
             544 * s12 * s13 * s14 * tmp20 - 32 * m12 * s12 * s23 * tmp20 -
             16 * m12 * s13 * s23 * tmp20 - 480 * s12 * s13 * s23 * tmp20 +
             168 * m12 * s14 * s23 * tmp20 - 1056 * s12 * s14 * s23 * tmp20 -
             616 * s13 * s14 * s23 * tmp20 - 240 * m12 * s12 * s24 * tmp20 -
             440 * m12 * s13 * s24 * tmp20 + 384 * s12 * s13 * s24 * tmp20 -
             240 * m12 * s14 * s24 * tmp20 - 184 * s12 * s14 * s24 * tmp20 +
             384 * s13 * s14 * s24 * tmp20 + 880 * m12 * s23 * s24 * tmp20 -
             64 * s12 * s23 * s24 * tmp20 - 88 * s13 * s23 * s24 * tmp20 +
             148 * s14 * s23 * s24 * tmp20 + 168 * m12 * s12 * s34 * tmp20 -
             16 * m12 * s13 * s34 * tmp20 - 616 * s12 * s13 * s34 * tmp20 -
             32 * m12 * s14 * s34 * tmp20 - 1056 * s12 * s14 * s34 * tmp20 -
             480 * s13 * s14 * s34 * tmp20 + 720 * m12 * s23 * s34 * tmp20 +
             612 * s12 * s23 * s34 * tmp20 + 636 * s13 * s23 * s34 * tmp20 +
             612 * s14 * s23 * s34 * tmp20 + 880 * m12 * s24 * s34 * tmp20 +
             148 * s12 * s24 * s34 * tmp20 - 88 * s13 * s24 * s34 * tmp20 -
             64 * s14 * s24 * s34 * tmp20 + 384 * m12 * tmp142 * tmp20 +
             150 * s12 * tmp142 * tmp20 + 314 * s13 * tmp142 * tmp20 +
             474 * s14 * tmp142 * tmp20 - 24 * s12 * tmp17 * tmp20 +
             24 * s13 * tmp17 * tmp20 - 24 * s14 * tmp17 * tmp20 +
             288 * s23 * tmp17 * tmp20 + 128 * s24 * tmp17 * tmp20 +
             288 * s34 * tmp17 * tmp20 + 24 * m12 * m22 * tmp211 -
             4 * m22 * s12 * tmp211 + 12 * m22 * s13 * tmp211 +
             28 * m22 * s14 * tmp211 - 6 * s12 * s14 * tmp211 -
             6 * s13 * s14 * tmp211 - 2 * s13 * s24 * tmp211 +
             4 * s14 * s24 * tmp211 + 8 * m12 * s34 * tmp211 -
             2 * s12 * s34 * tmp211 + 4 * s13 * s34 * tmp211 +
             4 * s14 * s34 * tmp211 + 408 * s12 * tmp23 + 232 * s13 * tmp23 +
             408 * s14 * tmp23 + 30 * m12 * m22 * s14 * s23 * tmp25 -
             60 * m22 * s13 * s14 * s23 * tmp25 -
             30 * m12 * m22 * s13 * s24 * tmp25 -
             12 * m12 * m22 * s14 * s24 * tmp25 +
             60 * m22 * s13 * s14 * s24 * tmp25 +
             28 * m12 * m22 * s23 * s24 * tmp25 -
             3 * m12 * s13 * s23 * s24 * tmp25 -
             67 * m22 * s13 * s23 * s24 * tmp25 -
             3 * m12 * s14 * s23 * s24 * tmp25 -
             3 * m22 * s14 * s23 * s24 * tmp25 +
             8 * s13 * s14 * s23 * s24 * tmp25 +
             24 * m12 * m22 * s14 * s34 * tmp25 -
             56 * m22 * s13 * s14 * s34 * tmp25 +
             40 * m12 * m22 * s23 * s34 * tmp25 +
             32 * m22 * s13 * s23 * s34 * tmp25 +
             3 * m12 * s14 * s23 * s34 * tmp25 +
             107 * m22 * s14 * s23 * s34 * tmp25 -
             10 * s13 * s14 * s23 * s34 * tmp25 +
             56 * m12 * m22 * s24 * s34 * tmp25 -
             3 * m12 * s13 * s24 * s34 * tmp25 -
             91 * m22 * s13 * s24 * s34 * tmp25 -
             6 * m12 * s14 * s24 * s34 * tmp25 -
             30 * m22 * s14 * s24 * s34 * tmp25 +
             10 * s13 * s14 * s24 * s34 * tmp25 +
             14 * m12 * s23 * s24 * s34 * tmp25 +
             118 * m22 * s23 * s24 * s34 * tmp25 -
             16 * s13 * s23 * s24 * s34 * tmp25 -
             20 * s14 * s23 * s24 * s34 * tmp25 +
             16 * m12 * m22 * tmp142 * tmp25 + 8 * m22 * s13 * tmp142 * tmp25 +
             3 * m12 * s14 * tmp142 * tmp25 + 67 * m22 * s14 * tmp142 * tmp25 -
             6 * s13 * s14 * tmp142 * tmp25 + 36 * m22 * s24 * tmp142 * tmp25 -
             6 * s13 * s24 * tmp142 * tmp25 + 8 * m12 * s34 * tmp142 * tmp25 +
             36 * m22 * s34 * tmp142 * tmp25 + 4 * s14 * s34 * tmp142 * tmp25 +
             18 * s24 * s34 * tmp142 * tmp25 - 128 * tmp18 * tmp25 +
             12 * m22 * tmp187 * tmp25 + 6 * s14 * tmp187 * tmp25 +
             6 * s34 * tmp187 * tmp25 + 16 * m12 * tmp20 * tmp25 +
             64 * s13 * tmp20 * tmp25 + 304 * s14 * tmp20 * tmp25 -
             48 * s23 * tmp20 * tmp25 + 184 * s24 * tmp20 * tmp25 -
             184 * s34 * tmp20 * tmp25 + 96 * m12 * s12 * s13 * s14 * tmp28 -
             96 * m12 * s12 * s13 * s23 * tmp28 -
             288 * m12 * s12 * s14 * s23 * tmp28 -
             136 * m12 * s13 * s14 * s23 * tmp28 +
             524 * s12 * s13 * s14 * s23 * tmp28 +
             160 * m12 * s12 * s13 * s24 * tmp28 -
             8 * m12 * s12 * s14 * s24 * tmp28 +
             160 * m12 * s13 * s14 * s24 * tmp28 -
             224 * s12 * s13 * s14 * s24 * tmp28 -
             256 * m12 * s12 * s23 * s24 * tmp28 -
             276 * m12 * s13 * s23 * s24 * tmp28 +
             200 * s12 * s13 * s23 * s24 * tmp28 -
             204 * m12 * s14 * s23 * s24 * tmp28 -
             184 * s12 * s14 * s23 * s24 * tmp28 +
             164 * s13 * s14 * s23 * s24 * tmp28 -
             136 * m12 * s12 * s13 * s34 * tmp28 -
             288 * m12 * s12 * s14 * s34 * tmp28 -
             96 * m12 * s13 * s14 * s34 * tmp28 +
             524 * s12 * s13 * s14 * s34 * tmp28 -
             116 * m12 * s12 * s23 * s34 * tmp28 -
             72 * m12 * s13 * s23 * s34 * tmp28 -
             520 * s12 * s13 * s23 * s34 * tmp28 -
             116 * m12 * s14 * s23 * s34 * tmp28 -
             816 * s12 * s14 * s23 * s34 * tmp28 -
             520 * s13 * s14 * s23 * s34 * tmp28 -
             204 * m12 * s12 * s24 * s34 * tmp28 -
             276 * m12 * s13 * s24 * s34 * tmp28 +
             164 * s12 * s13 * s24 * s34 * tmp28 -
             256 * m12 * s14 * s24 * s34 * tmp28 -
             184 * s12 * s14 * s24 * s34 * tmp28 +
             200 * s13 * s14 * s24 * s34 * tmp28 +
             888 * m12 * s23 * s24 * s34 * tmp28 -
             32 * s12 * s23 * s24 * s34 * tmp28 +
             14 * s13 * s23 * s24 * s34 * tmp28 -
             32 * s14 * s23 * s24 * s34 * tmp28 -
             120 * m12 * s12 * tmp142 * tmp28 -
             40 * m12 * s13 * tmp142 * tmp28 -
             188 * s12 * s13 * tmp142 * tmp28 +
             48 * m12 * s14 * tmp142 * tmp28 -
             444 * s12 * s14 * tmp142 * tmp28 -
             340 * s13 * s14 * tmp142 * tmp28 +
             408 * m12 * s24 * tmp142 * tmp28 -
             68 * s12 * s24 * tmp142 * tmp28 + 21 * s13 * s24 * tmp142 * tmp28 +
             138 * s14 * s24 * tmp142 * tmp28 +
             512 * m12 * s34 * tmp142 * tmp28 +
             123 * s12 * s34 * tmp142 * tmp28 +
             328 * s13 * s34 * tmp142 * tmp28 +
             284 * s14 * s34 * tmp142 * tmp28 - 24 * s12 * s23 * tmp17 * tmp28 +
             24 * s13 * s23 * tmp17 * tmp28 - 36 * s14 * s23 * tmp17 * tmp28 +
             60 * s13 * s24 * tmp17 * tmp28 + 128 * s23 * s24 * tmp17 * tmp28 -
             36 * s12 * s34 * tmp17 * tmp28 + 24 * s13 * s34 * tmp17 * tmp28 -
             24 * s14 * s34 * tmp17 * tmp28 + 320 * s23 * s34 * tmp17 * tmp28 +
             128 * s24 * s34 * tmp17 * tmp28 + 144 * tmp142 * tmp17 * tmp28 +
             168 * m12 * tmp187 * tmp28 + 100 * s13 * tmp187 * tmp28 +
             161 * s14 * tmp187 * tmp28 + 48 * m12 * s14 * tmp25 * tmp28 -
             96 * s13 * s14 * tmp25 * tmp28 + 40 * m12 * s23 * tmp25 * tmp28 +
             48 * s13 * s23 * tmp25 * tmp28 + 262 * s14 * s23 * tmp25 * tmp28 +
             80 * m12 * s24 * tmp25 * tmp28 - 214 * s13 * s24 * tmp25 * tmp28 -
             28 * s14 * s24 * tmp25 * tmp28 + 156 * s23 * s24 * tmp25 * tmp28 +
             24 * m12 * s34 * tmp25 * tmp28 + 80 * s13 * s34 * tmp25 * tmp28 +
             264 * s14 * s34 * tmp25 * tmp28 - 28 * s23 * s34 * tmp25 * tmp28 +
             148 * s24 * s34 * tmp25 * tmp28 + 32 * tmp142 * tmp25 * tmp28 -
             6 * m12 * m22 * s23 * tmp29 + 4 * m22 * s13 * s23 * tmp29 -
             16 * m22 * s14 * s23 * tmp29 - 6 * m12 * m22 * s24 * tmp29 +
             24 * m22 * s13 * s24 * tmp29 + 12 * m22 * s14 * s24 * tmp29 -
             24 * m22 * s23 * s24 * tmp29 + 2 * s13 * s23 * s24 * tmp29 +
             2 * s14 * s23 * s24 * tmp29 - 12 * m12 * m22 * s34 * tmp29 +
             8 * m22 * s13 * s34 * tmp29 - 8 * m22 * s14 * s34 * tmp29 -
             3 * m12 * s23 * s34 * tmp29 - 35 * m22 * s23 * s34 * tmp29 +
             2 * s13 * s23 * s34 * tmp29 - 3 * m12 * s24 * s34 * tmp29 -
             51 * m22 * s24 * s34 * tmp29 + 4 * s13 * s24 * s34 * tmp29 +
             6 * s14 * s24 * s34 * tmp29 - 12 * s23 * s24 * s34 * tmp29 -
             12 * m22 * tmp142 * tmp29 - 2 * s14 * tmp142 * tmp29 -
             6 * s34 * tmp142 * tmp29 - 28 * tmp20 * tmp29 -
             12 * m12 * tmp28 * tmp29 + 8 * s13 * tmp28 * tmp29 -
             24 * s14 * tmp28 * tmp29 - 38 * s23 * tmp28 * tmp29 -
             70 * s24 * tmp28 * tmp29 - 36 * s34 * tmp28 * tmp29 +
             4 * m22 * s23 * tmp32 + 4 * m22 * s24 * tmp32 +
             8 * m22 * s34 * tmp32 + 2 * s23 * s34 * tmp32 +
             2 * s24 * s34 * tmp32 + 8 * tmp28 * tmp32 -
             20 * m12 * m22 * s12 * s13 * tmp368 -
             12 * m12 * m22 * s12 * s14 * tmp368 -
             20 * m12 * m22 * s13 * s14 * tmp368 +
             6 * m12 * s12 * s13 * s14 * tmp368 +
             6 * m22 * s12 * s13 * s14 * tmp368 -
             30 * m12 * m22 * s12 * s23 * tmp368 -
             4 * m12 * m22 * s13 * s23 * tmp368 -
             2 * m12 * s12 * s13 * s23 * tmp368 +
             18 * m22 * s12 * s13 * s23 * tmp368 -
             28 * m12 * m22 * s14 * s23 * tmp368 -
             20 * m22 * s12 * s14 * s23 * tmp368 -
             8 * m12 * s13 * s14 * s23 * tmp368 +
             8 * m22 * s13 * s14 * s23 * tmp368 +
             4 * s12 * s13 * s14 * s23 * tmp368 -
             28 * m12 * m22 * s12 * s34 * tmp368 -
             4 * m12 * m22 * s13 * s34 * tmp368 -
             8 * m12 * s12 * s13 * s34 * tmp368 +
             8 * m22 * s12 * s13 * s34 * tmp368 -
             30 * m12 * m22 * s14 * s34 * tmp368 -
             20 * m22 * s12 * s14 * s34 * tmp368 -
             2 * m12 * s13 * s14 * s34 * tmp368 +
             18 * m22 * s13 * s14 * s34 * tmp368 +
             4 * s12 * s13 * s14 * s34 * tmp368 +
             152 * m12 * m22 * s23 * s34 * tmp368 -
             3 * m12 * s12 * s23 * s34 * tmp368 -
             23 * m22 * s12 * s23 * s34 * tmp368 +
             14 * m12 * s13 * s23 * s34 * tmp368 -
             2 * m22 * s13 * s23 * s34 * tmp368 -
             4 * s12 * s13 * s23 * s34 * tmp368 -
             3 * m12 * s14 * s23 * s34 * tmp368 -
             23 * m22 * s14 * s23 * s34 * tmp368 +
             8 * s12 * s14 * s23 * s34 * tmp368 -
             4 * s13 * s14 * s23 * s34 * tmp368 +
             64 * m12 * m22 * tmp142 * tmp368 -
             12 * m22 * s12 * tmp142 * tmp368 +
             2 * m12 * s13 * tmp142 * tmp368 -
             10 * m22 * s13 * tmp142 * tmp368 +
             6 * s12 * s13 * tmp142 * tmp368 - m12 * s14 * tmp142 * tmp368 +
             19 * m22 * s14 * tmp142 * tmp368 -
             2 * s13 * s14 * tmp142 * tmp368 + 8 * m12 * s34 * tmp142 * tmp368 -
             6 * s12 * s34 * tmp142 * tmp368 + 4 * s13 * s34 * tmp142 * tmp368 -
             4 * s14 * s34 * tmp142 * tmp368 + 6 * m22 * s12 * tmp17 * tmp368 +
             18 * m22 * s13 * tmp17 * tmp368 + 6 * m22 * s14 * tmp17 * tmp368 +
             16 * m22 * s23 * tmp17 * tmp368 + 3 * s14 * s23 * tmp17 * tmp368 +
             16 * m22 * s34 * tmp17 * tmp368 + 3 * s12 * s34 * tmp17 * tmp368 -
             4 * s23 * s34 * tmp17 * tmp368 - 4 * s13 * tmp187 * tmp368 +
             2 * s14 * tmp187 * tmp368 + 336 * m12 * tmp20 * tmp368 -
             2 * s12 * tmp20 * tmp368 - 94 * s13 * tmp20 * tmp368 -
             2 * s14 * tmp20 * tmp368 + 3 * m12 * s13 * tmp25 * tmp368 -
             5 * m22 * s13 * tmp25 * tmp368 - 6 * s13 * s14 * tmp25 * tmp368 +
             16 * m22 * s23 * tmp25 * tmp368 - 2 * s14 * s23 * tmp25 * tmp368 +
             28 * m22 * s34 * tmp25 * tmp368 + 4 * s13 * s34 * tmp25 * tmp368 -
             4 * s14 * s34 * tmp25 * tmp368 + 8 * s23 * s34 * tmp25 * tmp368 -
             52 * m12 * s12 * tmp28 * tmp368 - 52 * m12 * s13 * tmp28 * tmp368 +
             72 * s12 * s13 * tmp28 * tmp368 - 52 * m12 * s14 * tmp28 * tmp368 -
             56 * s12 * s14 * tmp28 * tmp368 + 72 * s13 * s14 * tmp28 * tmp368 +
             296 * m12 * s23 * tmp28 * tmp368 -
             30 * s12 * s23 * tmp28 * tmp368 - 44 * s13 * s23 * tmp28 * tmp368 +
             29 * s14 * s23 * tmp28 * tmp368 +
             296 * m12 * s34 * tmp28 * tmp368 +
             29 * s12 * s34 * tmp28 * tmp368 - 44 * s13 * s34 * tmp28 * tmp368 -
             30 * s14 * s34 * tmp28 * tmp368 + 32 * tmp17 * tmp28 * tmp368 +
             40 * tmp25 * tmp28 * tmp368 - 4 * m22 * tmp29 * tmp368 -
             2 * s13 * tmp29 * tmp368 - 2 * s34 * tmp29 * tmp368 -
             6 * m12 * m22 * s12 * s23 * tmp41 -
             6 * m12 * m22 * s14 * s23 * tmp41 -
             32 * m22 * s12 * s14 * s23 * tmp41 -
             66 * m12 * m22 * s12 * s24 * tmp41 -
             66 * m12 * m22 * s14 * s24 * tmp41 +
             120 * m22 * s12 * s14 * s24 * tmp41 +
             76 * m12 * m22 * s23 * s24 * tmp41 -
             6 * m12 * s12 * s23 * s24 * tmp41 -
             142 * m22 * s12 * s23 * s24 * tmp41 -
             6 * m12 * s14 * s23 * s24 * tmp41 -
             150 * m22 * s14 * s23 * s24 * tmp41 +
             12 * s12 * s14 * s23 * s24 * tmp41 -
             6 * m12 * m22 * s12 * s34 * tmp41 -
             6 * m12 * m22 * s14 * s34 * tmp41 -
             32 * m22 * s12 * s14 * s34 * tmp41 +
             24 * m12 * m22 * s23 * s34 * tmp41 +
             24 * m22 * s12 * s23 * s34 * tmp41 +
             24 * m22 * s14 * s23 * s34 * tmp41 -
             8 * s12 * s14 * s23 * s34 * tmp41 +
             76 * m12 * m22 * s24 * s34 * tmp41 -
             6 * m12 * s12 * s24 * s34 * tmp41 -
             150 * m22 * s12 * s24 * s34 * tmp41 -
             6 * m12 * s14 * s24 * s34 * tmp41 -
             142 * m22 * s14 * s24 * s34 * tmp41 +
             12 * s12 * s14 * s24 * s34 * tmp41 +
             4 * m12 * s23 * s24 * s34 * tmp41 +
             164 * m22 * s23 * s24 * s34 * tmp41 -
             20 * s12 * s23 * s24 * s34 * tmp41 -
             20 * s14 * s23 * s24 * s34 * tmp41 +
             12 * m12 * m22 * tmp142 * tmp41 + 4 * m22 * s12 * tmp142 * tmp41 +
             20 * m22 * s14 * tmp142 * tmp41 - 4 * s12 * s14 * tmp142 * tmp41 +
             2 * m12 * s24 * tmp142 * tmp41 + 78 * m22 * s24 * tmp142 * tmp41 -
             12 * s12 * s24 * tmp142 * tmp41 - 8 * s14 * s24 * tmp142 * tmp41 -
             20 * m22 * s34 * tmp142 * tmp41 + 4 * s12 * s34 * tmp142 * tmp41 +
             8 * s14 * s34 * tmp142 * tmp41 + 10 * s24 * s34 * tmp142 * tmp41 +
             144 * tmp18 * tmp41 - 4 * m22 * tmp187 * tmp41 +
             4 * s14 * tmp187 * tmp41 + 6 * s24 * tmp187 * tmp41 -
             4 * s34 * tmp187 * tmp41 + 96 * m12 * tmp20 * tmp41 -
             28 * s12 * tmp20 * tmp41 - 28 * s14 * tmp20 * tmp41 +
             112 * s23 * tmp20 * tmp41 + 632 * s24 * tmp20 * tmp41 +
             112 * s34 * tmp20 * tmp41 + 4 * m22 * s23 * tmp25 * tmp41 +
             64 * m22 * s24 * tmp25 * tmp41 + 6 * s23 * s24 * tmp25 * tmp41 +
             4 * m22 * s34 * tmp25 * tmp41 + 6 * s24 * s34 * tmp25 * tmp41 -
             12 * m12 * s12 * tmp28 * tmp41 - 12 * m12 * s14 * tmp28 * tmp41 -
             48 * s12 * s14 * tmp28 * tmp41 + 72 * m12 * s23 * tmp28 * tmp41 -
             6 * s12 * s23 * tmp28 * tmp41 + 10 * s14 * s23 * tmp28 * tmp41 +
             248 * m12 * s24 * tmp28 * tmp41 - 402 * s12 * s24 * tmp28 * tmp41 -
             402 * s14 * s24 * tmp28 * tmp41 + 416 * s23 * s24 * tmp28 * tmp41 +
             72 * m12 * s34 * tmp28 * tmp41 + 10 * s12 * s34 * tmp28 * tmp41 -
             6 * s14 * s34 * tmp28 * tmp41 + 24 * s23 * s34 * tmp28 * tmp41 +
             416 * s24 * s34 * tmp28 * tmp41 + 12 * tmp142 * tmp28 * tmp41 +
             8 * tmp25 * tmp28 * tmp41 + 4 * m12 * m22 * tmp368 * tmp41 +
             6 * m12 * s12 * tmp368 * tmp41 - 30 * m22 * s12 * tmp368 * tmp41 +
             6 * m12 * s14 * tmp368 * tmp41 - 30 * m22 * s14 * tmp368 * tmp41 -
             12 * s12 * s14 * tmp368 * tmp41 - 4 * m12 * s23 * tmp368 * tmp41 +
             64 * m22 * s23 * tmp368 * tmp41 + 4 * s14 * s23 * tmp368 * tmp41 -
             4 * m12 * s34 * tmp368 * tmp41 + 64 * m22 * s34 * tmp368 * tmp41 +
             4 * s12 * s34 * tmp368 * tmp41 + 4 * s23 * s34 * tmp368 * tmp41 +
             6 * tmp142 * tmp368 * tmp41 - 6 * tmp25 * tmp368 * tmp41 +
             176 * tmp28 * tmp368 * tmp41 - 6 * m12 * m22 * s12 * tmp451 +
             2 * m12 * m22 * s13 * tmp451 - 8 * m22 * s12 * s13 * tmp451 -
             6 * m12 * m22 * s14 * tmp451 - 8 * m22 * s12 * s14 * tmp451 -
             8 * m22 * s13 * s14 * tmp451 + 4 * s12 * s13 * s14 * tmp451 +
             32 * m12 * m22 * s23 * tmp451 + m12 * s13 * s23 * tmp451 -
             3 * m22 * s13 * s23 * tmp451 + m12 * s14 * s23 * tmp451 +
             5 * m22 * s14 * s23 * tmp451 - 4 * s13 * s14 * s23 * tmp451 +
             32 * m12 * m22 * s34 * tmp451 + m12 * s12 * s34 * tmp451 +
             5 * m22 * s12 * s34 * tmp451 + m12 * s13 * s34 * tmp451 -
             3 * m22 * s13 * s34 * tmp451 - 4 * s12 * s13 * s34 * tmp451 +
             4 * s13 * s23 * s34 * tmp451 - 2 * s13 * tmp142 * tmp451 +
             8 * m22 * tmp17 * tmp451 - 3 * s13 * tmp17 * tmp451 +
             2 * s13 * tmp25 * tmp451 + 72 * m12 * tmp28 * tmp451 +
             10 * s12 * tmp28 * tmp451 - 7 * s13 * tmp28 * tmp451 +
             10 * s14 * tmp28 * tmp451 + 8 * m22 * tmp41 * tmp451 +
             4 * s12 * tmp41 * tmp451 + 4 * s14 * tmp41 * tmp451 -
             12 * m12 * m22 * s23 * tmp47 + 12 * m22 * s12 * s23 * tmp47 +
             12 * m22 * s14 * s23 * tmp47 - 36 * m12 * m22 * s24 * tmp47 +
             68 * m22 * s12 * s24 * tmp47 + 68 * m22 * s14 * s24 * tmp47 -
             88 * m22 * s23 * s24 * tmp47 + 4 * s12 * s23 * s24 * tmp47 +
             4 * s14 * s23 * s24 * tmp47 - 12 * m12 * m22 * s34 * tmp47 +
             12 * m22 * s12 * s34 * tmp47 + 12 * m22 * s14 * s34 * tmp47 -
             32 * m22 * s23 * s34 * tmp47 - 88 * m22 * s24 * s34 * tmp47 +
             4 * s12 * s24 * s34 * tmp47 + 4 * s14 * s24 * s34 * tmp47 -
             8 * s23 * s24 * s34 * tmp47 - 16 * m22 * tmp142 * tmp47 -
             4 * s24 * tmp142 * tmp47 - 120 * tmp20 * tmp47 -
             24 * m12 * tmp28 * tmp47 + 24 * s12 * tmp28 * tmp47 +
             24 * s14 * tmp28 * tmp47 - 92 * s23 * tmp28 * tmp47 -
             244 * s24 * tmp28 * tmp47 - 92 * s34 * tmp28 * tmp47 +
             6 * m12 * tmp368 * tmp47 - 18 * m22 * tmp368 * tmp47 -
             8 * s12 * tmp368 * tmp47 - 8 * s14 * tmp368 * tmp47 +
             4 * s23 * tmp368 * tmp47 + 4 * s34 * tmp368 * tmp47 +
             4 * tmp451 * tmp47 + 8 * m12 * m22 * tmp478 - m12 * s13 * tmp478 -
             m22 * s13 * tmp478 + 8 * m22 * s23 * tmp51 +
             24 * m22 * s24 * tmp51 + 8 * m22 * s34 * tmp51 +
             16 * tmp28 * tmp51 - 4 * tmp368 * tmp51 +
             24 * m12 * m22 * s12 * s23 * tmp70 -
             56 * m22 * s12 * s13 * s23 * tmp70 -
             12 * m12 * m22 * s12 * s24 * tmp70 -
             30 * m12 * m22 * s13 * s24 * tmp70 +
             60 * m22 * s12 * s13 * s24 * tmp70 +
             56 * m12 * m22 * s23 * s24 * tmp70 -
             6 * m12 * s12 * s23 * s24 * tmp70 -
             30 * m22 * s12 * s23 * s24 * tmp70 -
             3 * m12 * s13 * s23 * s24 * tmp70 -
             91 * m22 * s13 * s23 * s24 * tmp70 +
             10 * s12 * s13 * s23 * s24 * tmp70 +
             30 * m12 * m22 * s12 * s34 * tmp70 -
             60 * m22 * s12 * s13 * s34 * tmp70 +
             40 * m12 * m22 * s23 * s34 * tmp70 +
             3 * m12 * s12 * s23 * s34 * tmp70 +
             107 * m22 * s12 * s23 * s34 * tmp70 +
             32 * m22 * s13 * s23 * s34 * tmp70 -
             10 * s12 * s13 * s23 * s34 * tmp70 +
             28 * m12 * m22 * s24 * s34 * tmp70 -
             3 * m12 * s12 * s24 * s34 * tmp70 -
             3 * m22 * s12 * s24 * s34 * tmp70 -
             3 * m12 * s13 * s24 * s34 * tmp70 -
             67 * m22 * s13 * s24 * s34 * tmp70 +
             8 * s12 * s13 * s24 * s34 * tmp70 +
             14 * m12 * s23 * s24 * s34 * tmp70 +
             118 * m22 * s23 * s24 * s34 * tmp70 -
             20 * s12 * s23 * s24 * s34 * tmp70 -
             16 * s13 * s23 * s24 * s34 * tmp70 +
             12 * m12 * m22 * tmp142 * tmp70 + 64 * m22 * s12 * tmp142 * tmp70 +
             32 * m22 * s13 * tmp142 * tmp70 - 4 * s12 * s13 * tmp142 * tmp70 +
             8 * m12 * s24 * tmp142 * tmp70 + 32 * m22 * s24 * tmp142 * tmp70 -
             8 * s12 * s24 * tmp142 * tmp70 - 6 * s13 * s24 * tmp142 * tmp70 +
             10 * m12 * s34 * tmp142 * tmp70 + 2 * m22 * s34 * tmp142 * tmp70 +
             2 * s12 * s34 * tmp142 * tmp70 + 4 * s13 * s34 * tmp142 * tmp70 +
             20 * s24 * s34 * tmp142 * tmp70 - 128 * tmp18 * tmp70 +
             2 * m12 * tmp187 * tmp70 - 22 * m22 * tmp187 * tmp70 +
             4 * s12 * tmp187 * tmp70 + 4 * s13 * tmp187 * tmp70 +
             2 * s24 * tmp187 * tmp70 + 2 * s34 * tmp187 * tmp70 +
             16 * m12 * tmp20 * tmp70 + 304 * s12 * tmp20 * tmp70 +
             64 * s13 * tmp20 * tmp70 - 184 * s23 * tmp20 * tmp70 +
             184 * s24 * tmp20 * tmp70 - 48 * s34 * tmp20 * tmp70 -
             2 * tmp211 * tmp70 - 36 * m22 * s23 * tmp25 * tmp70 +
             16 * m22 * s24 * tmp25 * tmp70 + 6 * s23 * s24 * tmp25 * tmp70 -
             36 * m22 * s34 * tmp25 * tmp70 - 4 * s23 * s34 * tmp25 * tmp70 +
             6 * s24 * s34 * tmp25 * tmp70 - 2 * tmp142 * tmp25 * tmp70 +
             48 * m12 * s12 * tmp28 * tmp70 - 96 * s12 * s13 * tmp28 * tmp70 +
             24 * m12 * s23 * tmp28 * tmp70 + 264 * s12 * s23 * tmp28 * tmp70 +
             80 * s13 * s23 * tmp28 * tmp70 + 80 * m12 * s24 * tmp28 * tmp70 -
             28 * s12 * s24 * tmp28 * tmp70 - 214 * s13 * s24 * tmp28 * tmp70 +
             148 * s23 * s24 * tmp28 * tmp70 + 40 * m12 * s34 * tmp28 * tmp70 +
             262 * s12 * s34 * tmp28 * tmp70 + 48 * s13 * s34 * tmp28 * tmp70 -
             28 * s23 * s34 * tmp28 * tmp70 + 156 * s24 * s34 * tmp28 * tmp70 -
             96 * tmp142 * tmp28 * tmp70 - 64 * tmp25 * tmp28 * tmp70 +
             3 * m12 * s13 * tmp368 * tmp70 - 5 * m22 * s13 * tmp368 * tmp70 -
             6 * s12 * s13 * tmp368 * tmp70 + 28 * m22 * s23 * tmp368 * tmp70 -
             4 * s12 * s23 * tmp368 * tmp70 + 4 * s13 * s23 * tmp368 * tmp70 +
             16 * m22 * s34 * tmp368 * tmp70 - 2 * s12 * s34 * tmp368 * tmp70 +
             8 * s23 * s34 * tmp368 * tmp70 + 4 * tmp142 * tmp368 * tmp70 +
             40 * tmp28 * tmp368 * tmp70 + 4 * m22 * s23 * tmp41 * tmp70 +
             64 * m22 * s24 * tmp41 * tmp70 + 6 * s23 * s24 * tmp41 * tmp70 +
             4 * m22 * s34 * tmp41 * tmp70 + 6 * s24 * s34 * tmp41 * tmp70 +
             8 * tmp28 * tmp41 * tmp70 - 6 * tmp368 * tmp41 * tmp70 +
             2 * s13 * tmp451 * tmp70 - 40 * m12 * m22 * s12 * s13 * tmp777 -
             64 * m12 * m22 * s12 * s14 * tmp777 -
             16 * m12 * m22 * s13 * s14 * tmp777 +
             6 * m12 * s12 * s13 * s14 * tmp777 +
             150 * m22 * s12 * s13 * s14 * tmp777 -
             74 * m12 * m22 * s12 * s23 * tmp777 -
             40 * m12 * m22 * s13 * s23 * tmp777 -
             4 * m12 * s12 * s13 * s23 * tmp777 -
             176 * m22 * s12 * s13 * s23 * tmp777 -
             108 * m12 * m22 * s14 * s23 * tmp777 -
             172 * m22 * s12 * s14 * s23 * tmp777 -
             2 * m12 * s13 * s14 * s23 * tmp777 -
             122 * m22 * s13 * s14 * s23 * tmp777 +
             28 * s12 * s13 * s14 * s23 * tmp777 -
             46 * m12 * m22 * s12 * s24 * tmp777 -
             44 * m12 * m22 * s13 * s24 * tmp777 +
             2 * m12 * s12 * s13 * s24 * tmp777 -
             10 * m22 * s12 * s13 * s24 * tmp777 -
             68 * m12 * m22 * s14 * s24 * tmp777 +
             2 * m12 * s12 * s14 * s24 * tmp777 -
             58 * m22 * s12 * s14 * s24 * tmp777 +
             4 * m12 * s13 * s14 * s24 * tmp777 +
             36 * m22 * s13 * s14 * s24 * tmp777 -
             4 * s12 * s13 * s14 * s24 * tmp777 +
             256 * m12 * m22 * s23 * s24 * tmp777 -
             24 * m12 * s12 * s23 * s24 * tmp777 +
             36 * m22 * s13 * s23 * s24 * tmp777 -
             2 * s12 * s13 * s23 * s24 * tmp777 -
             22 * m12 * s14 * s23 * s24 * tmp777 -
             58 * m22 * s14 * s23 * s24 * tmp777 +
             14 * s12 * s14 * s23 * s24 * tmp777 +
             8 * s13 * s14 * s23 * s24 * tmp777 +
             208 * m12 * m22 * tmp142 * tmp777 -
             24 * m12 * s12 * tmp142 * tmp777 +
             24 * m22 * s12 * tmp142 * tmp777 -
             8 * m12 * s13 * tmp142 * tmp777 +
             112 * m22 * s13 * tmp142 * tmp777 -
             18 * s12 * s13 * tmp142 * tmp777 -
             24 * m12 * s14 * tmp142 * tmp777 +
             24 * m22 * s14 * tmp142 * tmp777 -
             4 * s12 * s14 * tmp142 * tmp777 -
             18 * s13 * s14 * tmp142 * tmp777 +
             32 * m12 * s24 * tmp142 * tmp777 -
             12 * s12 * s24 * tmp142 * tmp777 +
             12 * s13 * s24 * tmp142 * tmp777 -
             12 * s14 * s24 * tmp142 * tmp777 -
             18 * m22 * s12 * tmp17 * tmp777 + 6 * m22 * s13 * tmp17 * tmp777 -
             6 * m22 * s14 * tmp17 * tmp777 + 104 * m22 * s23 * tmp17 * tmp777 -
             6 * s12 * s23 * tmp17 * tmp777 - 3 * s14 * s23 * tmp17 * tmp777 +
             32 * m22 * s24 * tmp17 * tmp777 + 3 * s13 * s24 * tmp17 * tmp777 +
             4 * s23 * s24 * tmp17 * tmp777 + 16 * tmp142 * tmp17 * tmp777 +
             24 * m12 * tmp187 * tmp777 - 4 * s12 * tmp187 * tmp777 +
             12 * s13 * tmp187 * tmp777 + 384 * m12 * tmp20 * tmp777 +
             474 * s12 * tmp20 * tmp777 + 314 * s13 * tmp20 * tmp777 +
             150 * s14 * tmp20 * tmp777 + 12 * m12 * m22 * tmp25 * tmp777 +
             32 * m22 * s13 * tmp25 * tmp777 + 64 * m22 * s14 * tmp25 * tmp777 -
             4 * s13 * s14 * tmp25 * tmp777 + 10 * m12 * s23 * tmp25 * tmp777 +
             2 * m22 * s23 * tmp25 * tmp777 + 4 * s13 * s23 * tmp25 * tmp777 +
             2 * s14 * s23 * tmp25 * tmp777 + 8 * m12 * s24 * tmp25 * tmp777 +
             32 * m22 * s24 * tmp25 * tmp777 - 6 * s13 * s24 * tmp25 * tmp777 -
             8 * s14 * s24 * tmp25 * tmp777 + 20 * s23 * s24 * tmp25 * tmp777 +
             10 * tmp142 * tmp25 * tmp777 + 48 * m12 * s12 * tmp28 * tmp777 -
             40 * m12 * s13 * tmp28 * tmp777 -
             340 * s12 * s13 * tmp28 * tmp777 -
             120 * m12 * s14 * tmp28 * tmp777 -
             444 * s12 * s14 * tmp28 * tmp777 -
             188 * s13 * s14 * tmp28 * tmp777 +
             512 * m12 * s23 * tmp28 * tmp777 +
             284 * s12 * s23 * tmp28 * tmp777 +
             328 * s13 * s23 * tmp28 * tmp777 +
             123 * s14 * s23 * tmp28 * tmp777 +
             408 * m12 * s24 * tmp28 * tmp777 +
             138 * s12 * s24 * tmp28 * tmp777 +
             21 * s13 * s24 * tmp28 * tmp777 - 68 * s14 * s24 * tmp28 * tmp777 +
             144 * tmp17 * tmp28 * tmp777 - 96 * tmp25 * tmp28 * tmp777 -
             3 * m12 * tmp29 * tmp777 - 15 * m22 * tmp29 * tmp777 +
             2 * s13 * tmp29 * tmp777 + 2 * s14 * tmp29 * tmp777 -
             8 * s23 * tmp29 * tmp777 - 8 * s24 * tmp29 * tmp777 +
             2 * tmp32 * tmp777 + 64 * m12 * m22 * tmp368 * tmp777 -
             m12 * s12 * tmp368 * tmp777 + 19 * m22 * s12 * tmp368 * tmp777 +
             2 * m12 * s13 * tmp368 * tmp777 -
             10 * m22 * s13 * tmp368 * tmp777 -
             2 * s12 * s13 * tmp368 * tmp777 -
             12 * m22 * s14 * tmp368 * tmp777 +
             6 * s13 * s14 * tmp368 * tmp777 + 8 * m12 * s23 * tmp368 * tmp777 -
             4 * s12 * s23 * tmp368 * tmp777 + 4 * s13 * s23 * tmp368 * tmp777 -
             6 * s14 * s23 * tmp368 * tmp777 + 4 * tmp25 * tmp368 * tmp777 +
             12 * m12 * m22 * tmp41 * tmp777 + 20 * m22 * s12 * tmp41 * tmp777 +
             4 * m22 * s14 * tmp41 * tmp777 - 4 * s12 * s14 * tmp41 * tmp777 -
             20 * m22 * s23 * tmp41 * tmp777 + 8 * s12 * s23 * tmp41 * tmp777 +
             4 * s14 * s23 * tmp41 * tmp777 + 2 * m12 * s24 * tmp41 * tmp777 +
             78 * m22 * s24 * tmp41 * tmp777 - 8 * s12 * s24 * tmp41 * tmp777 -
             12 * s14 * s24 * tmp41 * tmp777 + 10 * s23 * s24 * tmp41 * tmp777 -
             8 * tmp142 * tmp41 * tmp777 + 12 * tmp28 * tmp41 * tmp777 +
             6 * tmp368 * tmp41 * tmp777 - 2 * s13 * tmp451 * tmp777 -
             16 * m22 * tmp47 * tmp777 - 4 * s24 * tmp47 * tmp777 +
             16 * m12 * m22 * tmp70 * tmp777 + 3 * m12 * s12 * tmp70 * tmp777 +
             67 * m22 * s12 * tmp70 * tmp777 + 8 * m22 * s13 * tmp70 * tmp777 -
             6 * s12 * s13 * tmp70 * tmp777 + 8 * m12 * s23 * tmp70 * tmp777 +
             36 * m22 * s23 * tmp70 * tmp777 + 4 * s12 * s23 * tmp70 * tmp777 +
             36 * m22 * s24 * tmp70 * tmp777 - 6 * s13 * s24 * tmp70 * tmp777 +
             18 * s23 * s24 * tmp70 * tmp777 + 10 * tmp142 * tmp70 * tmp777 -
             2 * tmp25 * tmp70 * tmp777 + 32 * tmp28 * tmp70 * tmp777 -
             12 * m12 * m22 * s23 * tmp79 - 8 * m22 * s12 * s23 * tmp79 +
             8 * m22 * s13 * s23 * tmp79 - 6 * m12 * m22 * s24 * tmp79 +
             12 * m22 * s12 * s24 * tmp79 + 24 * m22 * s13 * s24 * tmp79 -
             3 * m12 * s23 * s24 * tmp79 - 51 * m22 * s23 * s24 * tmp79 +
             6 * s12 * s23 * s24 * tmp79 + 4 * s13 * s23 * s24 * tmp79 -
             6 * m12 * m22 * s34 * tmp79 - 16 * m22 * s12 * s34 * tmp79 +
             4 * m22 * s13 * s34 * tmp79 - 3 * m12 * s23 * s34 * tmp79 -
             35 * m22 * s23 * s34 * tmp79 + 2 * s13 * s23 * s34 * tmp79 -
             24 * m22 * s24 * s34 * tmp79 + 2 * s12 * s24 * s34 * tmp79 +
             2 * s13 * s24 * s34 * tmp79 - 12 * s23 * s24 * s34 * tmp79 -
             3 * m12 * tmp142 * tmp79 - 15 * m22 * tmp142 * tmp79 +
             2 * s12 * tmp142 * tmp79 + 2 * s13 * tmp142 * tmp79 -
             8 * s24 * tmp142 * tmp79 - 8 * s34 * tmp142 * tmp79 -
             2 * tmp187 * tmp79 - 28 * tmp20 * tmp79 -
             12 * m12 * tmp28 * tmp79 - 24 * s12 * tmp28 * tmp79 +
             8 * s13 * tmp28 * tmp79 - 36 * s23 * tmp28 * tmp79 -
             70 * s24 * tmp28 * tmp79 - 38 * s34 * tmp28 * tmp79 -
             4 * m22 * tmp368 * tmp79 - 2 * s13 * tmp368 * tmp79 -
             2 * s23 * tmp368 * tmp79 - 12 * m22 * tmp777 * tmp79 -
             2 * s12 * tmp777 * tmp79 - 6 * s23 * tmp777 * tmp79 +
             8 * m22 * s23 * tmp84 + 4 * m22 * s24 * tmp84 +
             2 * s23 * s24 * tmp84 + 4 * m22 * s34 * tmp84 +
             2 * s23 * s34 * tmp84 + 2 * tmp142 * tmp84 + 8 * tmp28 * tmp84 +
             2 * m12 * m22 * s12 * tmp926 - 12 * m12 * m22 * s13 * tmp926 -
             2 * m12 * s12 * s13 * tmp926 - 78 * m22 * s12 * s13 * tmp926 -
             32 * m12 * m22 * s14 * tmp926 - 4 * m12 * s12 * s14 * tmp926 -
             80 * m22 * s12 * s14 * tmp926 - 24 * m22 * s13 * s14 * tmp926 +
             12 * s12 * s13 * s14 * tmp926 + 128 * m12 * m22 * s23 * tmp926 -
             12 * m12 * s12 * s23 * tmp926 + 56 * m22 * s12 * s23 * tmp926 -
             4 * m12 * s13 * s23 * tmp926 + 68 * m22 * s13 * s23 * tmp926 -
             18 * s12 * s13 * s23 * tmp926 - 12 * m12 * s14 * s23 * tmp926 -
             8 * m22 * s14 * s23 * tmp926 - 8 * s12 * s14 * s23 * tmp926 -
             6 * s13 * s14 * s23 * tmp926 + 64 * m12 * m22 * s24 * tmp926 -
             2 * m12 * s12 * s24 * tmp926 + 42 * m22 * s12 * s24 * tmp926 +
             4 * m22 * s13 * s24 * tmp926 - 4 * s12 * s13 * s24 * tmp926 -
             16 * m22 * s14 * s24 * tmp926 - 6 * s12 * s14 * s24 * tmp926 +
             6 * s13 * s14 * s24 * tmp926 + 16 * m12 * s23 * s24 * tmp926 +
             4 * s13 * s23 * s24 * tmp926 - 8 * s14 * s23 * s24 * tmp926 +
             24 * m12 * tmp142 * tmp926 + 12 * s13 * tmp142 * tmp926 -
             4 * s14 * tmp142 * tmp926 + 24 * m22 * tmp17 * tmp926 -
             3 * s12 * tmp17 * tmp926 + 8 * s23 * tmp17 * tmp926 +
             2 * m12 * tmp25 * tmp926 - 22 * m22 * tmp25 * tmp926 +
             4 * s13 * tmp25 * tmp926 + 4 * s14 * tmp25 * tmp926 +
             2 * s23 * tmp25 * tmp926 + 2 * s24 * tmp25 * tmp926 +
             168 * m12 * tmp28 * tmp926 + 161 * s12 * tmp28 * tmp926 +
             100 * s13 * tmp28 * tmp926 - 2 * tmp29 * tmp926 +
             2 * s12 * tmp368 * tmp926 - 4 * s13 * tmp368 * tmp926 -
             4 * m22 * tmp41 * tmp926 + 4 * s12 * tmp41 * tmp926 -
             4 * s23 * tmp41 * tmp926 + 6 * s24 * tmp41 * tmp926 +
             12 * m22 * tmp70 * tmp926 + 6 * s12 * tmp70 * tmp926 +
             6 * s23 * tmp70 * tmp926 + 24 * m12 * m22 * tmp982 +
             28 * m22 * s12 * tmp982 + 12 * m22 * s13 * tmp982 -
             6 * s12 * s13 * tmp982 - 4 * m22 * s14 * tmp982 -
             6 * s12 * s14 * tmp982 + 8 * m12 * s23 * tmp982 +
             4 * s12 * s23 * tmp982 + 4 * s13 * s23 * tmp982 -
             2 * s14 * s23 * tmp982 + 4 * s12 * s24 * tmp982 -
             2 * s13 * s24 * tmp982 - 2 * tmp25 * tmp982)) /
           (3. * (s23 + tmp3) * (s34 + tmp3) * pow(s23 + s24 + s34 + tmp3, 2) *
            (-s12 + s23 + tmp3 + tmp7) * (-s14 + s34 + tmp3 + tmp7));
}

MUSTARD_OPTIMIZE_FAST auto MSqM2ENNEE::OneBornPol(double s12, double s13, double s14, double s23, double s24, double s34,
                                                  double m12, double m22, double,
                                                  double s2n, double s3n, double s4n) -> double {
    using muc::pow;

    // Adapt from McMule v0.5.1, mudecrare/mudecrare_1l_onetrace.opt.f95, FUNCTION bornPol
    //
    // Copyright (C) 2020-2025  Yannick Ulrich and others (The McMule development team)
    //

    const auto tmp3{2 * m22};
    const auto tmp7{-s13};
    const auto tmp58{pow(s13, 2)};
    const auto tmp61{pow(s14, 2)};
    const auto tmp65{pow(s23, 2)};
    const auto tmp70{pow(s24, 2)};
    const auto tmp76{pow(s34, 2)};
    const auto tmp23{pow(m12, 2)};
    const auto tmp27{-(s24 * s3n)};
    const auto tmp164{pow(s23, 3)};
    const auto tmp177{pow(s34, 3)};
    const auto tmp93{pow(s12, 2)};
    const auto tmp24{s23 + s24 + s34};
    const auto tmp215{s2n + s4n};
    const auto tmp227{s3n + s4n};
    const auto tmp114{s2n + s3n + s4n};
    const auto tmp226{s2n * tmp76};
    const auto tmp219{2 * s3n};
    const auto tmp263{s2n + s3n};
    const auto tmp236{s4n * tmp65};
    const auto tmp230{tmp114 * tmp70};
    const auto tmp233{s23 + s34};
    const auto tmp217{s23 * tmp215};
    const auto tmp218{s34 * tmp215};
    const auto tmp220{s2n + s4n + tmp219};
    const auto tmp221{-(s24 * tmp220)};
    const auto tmp222{tmp217 + tmp218 + tmp221};
    const auto tmp163{s2n * tmp61 * tmp65};
    const auto tmp166{s23 * s24 * s2n * tmp61};
    const auto tmp167{-(s14 * s24 * s2n * tmp65)};
    const auto tmp168{s23 * s2n * s34 * tmp61};
    const auto tmp282{pow(s24, 3)};
    const auto tmp200{s4n * tmp61 * tmp65};
    const auto tmp203{s23 * s24 * s4n * tmp61};
    const auto tmp206{s23 * s34 * s4n * tmp61};
    const auto tmp216{s34 * tmp215 * tmp24 * tmp93};
    const auto tmp223{s24 * tmp222 * tmp58};
    const auto tmp250{-(s23 * s3n * tmp70)};
    const auto tmp257{s24 * s4n * tmp65};
    const auto tmp264{-(s23 * s24 * tmp263)};
    const auto tmp265{-(s24 * s34 * tmp263)};
    const auto tmp266{2 * s23 * s34 * s4n};
    const auto tmp267{s4n * tmp76};
    const auto tmp268{tmp230 + tmp236 + tmp264 + tmp265 + tmp266 + tmp267};
    const auto tmp224{s2n * tmp65};
    const auto tmp225{2 * s23 * s2n * s34};
    const auto tmp228{-(s23 * s24 * tmp227)};
    const auto tmp229{-(s24 * s34 * tmp227)};
    const auto tmp231{tmp224 + tmp225 + tmp226 + tmp228 + tmp229 + tmp230};
    const auto tmp234{s2n + tmp219};
    const auto tmp237{tmp215 * tmp70};
    const auto tmp239{s4n + tmp219};
    const auto tmp362{pow(s13, 3)};
    const auto tmp366{pow(s14, 3)};
    const auto tmp479{2 * s2n};
    const auto tmp480{2 * s4n};
    const auto tmp481{s3n + tmp479 + tmp480};
    const auto tmp118{5 * s24 * s2n};
    const auto tmp125{5 * s24 * s4n};
    const auto tmp527{-4 * s3n};
    const auto tmp131{5 * s4n};
    const auto tmp473{-2 * s4n};
    const auto tmp94{7 * s2n};
    const auto tmp19{29 * s3n};
    const auto tmp201{pow(s23, 4)};
    const auto tmp181{pow(s34, 4)};
    const auto tmp297{pow(s24, 4)};
    const auto tmp207{-2 * s14 * s34 * s4n * tmp65};
    const auto tmp465{pow(s12, 3)};
    const auto tmp581{-s3n};
    const auto tmp488{3 * s4n};
    const auto tmp543{5 * s2n};
    const auto tmp544{tmp131 + tmp219 + tmp543};
    const auto tmp328{4 * s24 * s2n * tmp76};
    const auto tmp781{2 * s34};
    const auto tmp782{s23 + s24 + tmp781};
    const auto tmp786{3 * s2n};
    const auto tmp47{-2 * s3n};
    const auto tmp784{s2n + s4n + tmp581};
    const auto tmp130{-4 * s2n};
    const auto tmp534{3 * s24 * s3n};
    const auto tmp525{8 * s34 * s3n};
    const auto tmp352{4 * s3n};
    const auto tmp353{s2n + s4n + tmp352};
    const auto tmp923{s3n + tmp488 + tmp786};
    const auto tmp820{-2 * s24 * tmp220};
    const auto tmp808{-3 * s4n};
    const auto tmp965{s2n + tmp219 + tmp808};
    const auto tmp968{11 * s2n};
    const auto tmp976{42 * s3n};
    const auto tmp983{32 * s3n};
    const auto tmp52{55 * s4n};
    const auto tmp880{5 * s2n * tmp70};
    const auto tmp577{4 * s2n};
    const auto tmp578{4 * s4n};
    const auto tmp1014{s3n + tmp131 + tmp543};
    return (-32 *
            (8 * pow(m22, 5) * (51 * s2n + 51 * s4n + tmp19) -
             tmp23 * pow(tmp24, 2) * (s2n * s34 + s23 * s4n + tmp27) -
             4 * pow(m22, 4) *
                 (54 * s13 * s2n + 110 * s14 * s2n - 118 * s23 * s2n -
                  4 * s24 * s2n - 175 * s2n * s34 - 36 * s13 * s3n +
                  46 * s14 * s3n - 110 * s23 * s3n + 49 * s24 * s3n -
                  110 * s34 * s3n + 54 * s13 * s4n + 32 * s14 * s4n -
                  175 * s23 * s4n - 4 * s24 * s4n - 118 * s34 * s4n +
                  4 * m12 * (s2n + s4n + tmp47) +
                  2 * s12 * (16 * s2n + 23 * s3n + tmp52)) +
             2 * (s12 + s13 + s14 - s23 - s24 - s34) *
                 (-2 * s14 * s23 * s24 * s2n * s34 -
                  s14 * s23 * s24 * s34 * s3n -
                  4 * s14 * s23 * s24 * s34 * s4n + tmp163 +
                  s14 * s2n * tmp164 + s2n * s34 * tmp164 + s14 * s3n * tmp164 +
                  s24 * s3n * tmp164 - 2 * s34 * s3n * tmp164 -
                  s24 * s4n * tmp164 - s34 * s4n * tmp164 + tmp166 + tmp167 +
                  tmp168 + s14 * s2n * tmp177 - s23 * s2n * tmp177 -
                  s24 * s2n * tmp177 - 2 * s23 * s3n * tmp177 +
                  s24 * s3n * tmp177 + s23 * s4n * tmp177 - s2n * tmp181 +
                  tmp200 - s4n * tmp201 + tmp203 + tmp206 + tmp207 + tmp216 +
                  tmp223 +
                  s13 * (-(s14 * tmp231) +
                         tmp233 *
                             (s23 * s34 * tmp215 + tmp226 - s23 * s24 * tmp234 +
                              tmp236 + tmp237 - s24 * s34 * tmp239)) +
                  s14 * s2n * s34 * tmp65 + 3 * s24 * s2n * s34 * tmp65 +
                  2 * s14 * s34 * s3n * tmp65 - s24 * s34 * s3n * tmp65 -
                  2 * s14 * s24 * s4n * tmp65 + 2 * s24 * s34 * s4n * tmp65 +
                  s14 * s23 * s3n * tmp70 - s14 * s34 * s3n * tmp70 -
                  2 * s23 * s34 * s3n * tmp70 + s3n * tmp65 * tmp70 +
                  s14 * s23 * s2n * tmp76 + s14 * s24 * s2n * tmp76 +
                  2 * s23 * s24 * s2n * tmp76 + s14 * s23 * s3n * tmp76 -
                  s14 * s24 * s3n * tmp76 - s23 * s24 * s3n * tmp76 -
                  2 * s14 * s23 * s4n * tmp76 + 3 * s23 * s24 * s4n * tmp76 +
                  s2n * tmp65 * tmp76 - 4 * s3n * tmp65 * tmp76 +
                  s4n * tmp65 * tmp76 + s3n * tmp70 * tmp76 +
                  s12 * (-4 * s23 * s24 * s2n * s34 - s23 * s24 * s34 * s3n -
                         2 * s23 * s24 * s34 * s4n + s4n * tmp164 +
                         s3n * tmp177 + s4n * tmp177 - s14 * tmp222 * tmp233 +
                         tmp250 + tmp257 - s13 * tmp268 -
                         2 * s2n * s34 * tmp65 - s24 * s3n * tmp65 +
                         s34 * s3n * tmp65 + s34 * s4n * tmp65 +
                         s34 * s3n * tmp70 - 2 * s23 * s2n * tmp76 -
                         2 * s24 * s2n * tmp76 + 2 * s23 * s3n * tmp76 +
                         s23 * s4n * tmp76 - s24 * s4n * tmp76)) -
             m12 *
                 (-4 * s14 * s23 * s24 * s2n * s34 +
                  3 * s14 * s23 * s24 * s34 * s3n -
                  6 * s14 * s23 * s24 * s34 * s4n + tmp163 +
                  4 * s2n * s34 * tmp164 + 2 * s14 * s3n * tmp164 -
                  4 * s34 * s3n * tmp164 - 2 * s14 * s4n * tmp164 +
                  2 * s24 * s4n * tmp164 + 4 * s34 * s4n * tmp164 + tmp166 +
                  tmp167 + tmp168 + 4 * s23 * s2n * tmp177 +
                  2 * s24 * s2n * tmp177 - 4 * s23 * s3n * tmp177 +
                  4 * s23 * s4n * tmp177 + tmp200 + tmp203 + tmp206 + tmp216 +
                  tmp223 + s2n * s34 * tmp282 + s14 * s3n * tmp282 -
                  3 * s23 * s3n * tmp282 - 3 * s34 * s3n * tmp282 +
                  s23 * s4n * tmp282 - s3n * tmp297 -
                  2 * s14 * s2n * s34 * tmp65 + 6 * s24 * s2n * s34 * tmp65 +
                  3 * s14 * s24 * s3n * tmp65 + 4 * s14 * s34 * s3n * tmp65 -
                  8 * s24 * s34 * s3n * tmp65 - 4 * s14 * s24 * s4n * tmp65 -
                  6 * s14 * s34 * s4n * tmp65 + 8 * s24 * s34 * s4n * tmp65 +
                  s14 * s23 * s2n * tmp70 - s14 * s2n * s34 * tmp70 +
                  s23 * s2n * s34 * tmp70 + 4 * s14 * s23 * s3n * tmp70 +
                  s14 * s34 * s3n * tmp70 - 10 * s23 * s34 * s3n * tmp70 +
                  s23 * s34 * s4n * tmp70 - 2 * s3n * tmp65 * tmp70 +
                  3 * s4n * tmp65 * tmp70 - 2 * s14 * s23 * s2n * tmp76 -
                  s14 * s24 * s2n * tmp76 + 8 * s23 * s24 * s2n * tmp76 +
                  2 * s14 * s23 * s3n * tmp76 - 8 * s23 * s24 * s3n * tmp76 -
                  4 * s14 * s23 * s4n * tmp76 + 6 * s23 * s24 * s4n * tmp76 +
                  8 * s2n * tmp65 * tmp76 - 8 * s3n * tmp65 * tmp76 +
                  8 * s4n * tmp65 * tmp76 + 3 * s2n * tmp70 * tmp76 -
                  2 * s3n * tmp70 * tmp76 -
                  s12 * (6 * s23 * s24 * s2n * s34 - 3 * s23 * s24 * s34 * s3n +
                         4 * s23 * s24 * s34 * s4n + 2 * s2n * tmp177 -
                         2 * s3n * tmp177 + s14 * tmp222 * tmp233 + tmp250 +
                         tmp257 + s13 * tmp268 - s3n * tmp282 + tmp328 +
                         4 * s2n * s34 * tmp65 - 2 * s34 * s3n * tmp65 +
                         2 * s34 * s4n * tmp65 - 4 * s34 * s3n * tmp70 +
                         s23 * s4n * tmp70 - s34 * s4n * tmp70 +
                         6 * s23 * s2n * tmp76 - 4 * s23 * s3n * tmp76 -
                         3 * s24 * s3n * tmp76 + 2 * s23 * s4n * tmp76 +
                         s24 * s4n * tmp76) -
                  s13 * (s14 * tmp231 +
                         s24 * (-(s23 * s24 * s2n) - s24 * s34 * s4n + tmp237 +
                                s23 * s34 * tmp353 + tmp239 * tmp65 +
                                tmp234 * tmp76))) -
             2 * pow(m22, 3) *
                 (-106 * s13 * s14 * s2n + 112 * s13 * s23 * s2n +
                  276 * s14 * s23 * s2n - 118 * s13 * s24 * s2n +
                  46 * s14 * s24 * s2n + 32 * s23 * s24 * s2n +
                  164 * s13 * s2n * s34 + 252 * s14 * s2n * s34 -
                  306 * s23 * s2n * s34 - 74 * s24 * s2n * s34 +
                  16 * s13 * s14 * s3n - 56 * s13 * s23 * s3n +
                  144 * s14 * s23 * s3n - 316 * s13 * s24 * s3n -
                  74 * s14 * s24 * s3n + 44 * s23 * s24 * s3n -
                  56 * s13 * s34 * s3n + 128 * s14 * s34 * s3n -
                  318 * s23 * s34 * s3n + 44 * s24 * s34 * s3n -
                  16 * s13 * s14 * s4n + 164 * s13 * s23 * s4n +
                  92 * s14 * s23 * s4n - 118 * s13 * s24 * s4n -
                  92 * s14 * s24 * s4n - 74 * s23 * s24 * s4n +
                  112 * s13 * s34 * s4n + 24 * s14 * s34 * s4n -
                  306 * s23 * s34 * s4n + 32 * s24 * s34 * s4n +
                  2 * s12 *
                      (12 * s23 * s2n - 46 * s24 * s2n + 46 * s2n * s34 +
                       64 * s23 * s3n - 37 * s24 * s3n + 72 * s34 * s3n +
                       s13 * (-8 * s2n + 8 * s3n - 53 * s4n) + 126 * s23 * s4n +
                       23 * s24 * s4n + 138 * s34 * s4n -
                       s14 * (59 * s2n + 30 * s3n + 59 * s4n)) +
                  4 * tmp114 * tmp23 +
                  4 * m12 *
                      (5 * s14 * s2n + 6 * s23 * s2n + 2 * s2n * s34 +
                       s14 * s3n - 10 * s23 * s3n - 10 * s34 * s3n -
                       4 * s14 * s4n + 2 * s23 * s4n + 6 * s34 * s4n +
                       s13 * (s2n - 6 * s3n + s4n) + tmp118 + tmp125 +
                       s12 * (s3n + tmp130 + tmp131) + tmp27) -
                  2 * s2n * tmp58 + 60 * s3n * tmp58 - 2 * s4n * tmp58 -
                  34 * s2n * tmp61 - 16 * s3n * tmp61 + 14 * s4n * tmp61 -
                  75 * s2n * tmp65 - 157 * s3n * tmp65 - 237 * s4n * tmp65 +
                  s2n * tmp70 + 47 * s3n * tmp70 + s4n * tmp70 -
                  237 * s2n * tmp76 - 157 * s3n * tmp76 - 75 * s4n * tmp76 +
                  2 * tmp93 * (-8 * s3n - 17 * s4n + tmp94)) -
             pow(m22, 2) *
                 (-216 * s13 * s14 * s23 * s2n + 60 * s13 * s14 * s24 * s2n -
                  154 * s13 * s23 * s24 * s2n + 72 * s14 * s23 * s24 * s2n -
                  192 * s13 * s14 * s2n * s34 + 240 * s13 * s23 * s2n * s34 +
                  408 * s14 * s23 * s2n * s34 - 84 * s13 * s24 * s2n * s34 +
                  112 * s14 * s24 * s2n * s34 + 32 * s23 * s24 * s2n * s34 +
                  8 * s13 * s14 * s23 * s3n + 246 * s13 * s14 * s24 * s3n -
                  416 * s13 * s23 * s24 * s3n - 80 * s14 * s23 * s24 * s3n +
                  16 * s13 * s14 * s34 * s3n - 24 * s13 * s23 * s34 * s3n +
                  280 * s14 * s23 * s34 * s3n - 416 * s13 * s24 * s34 * s3n -
                  46 * s14 * s24 * s34 * s3n - 14 * s23 * s24 * s34 * s3n -
                  40 * s13 * s14 * s23 * s4n + 182 * s13 * s14 * s24 * s4n -
                  84 * s13 * s23 * s24 * s4n - 148 * s14 * s23 * s24 * s4n -
                  16 * s13 * s14 * s34 * s4n + 240 * s13 * s23 * s34 * s4n +
                  28 * s14 * s23 * s34 * s4n - 154 * s13 * s24 * s34 * s4n -
                  156 * s14 * s24 * s34 * s4n + 32 * s23 * s24 * s34 * s4n -
                  100 * s3n * tmp164 - 161 * s4n * tmp164 - 161 * s2n * tmp177 -
                  100 * s3n * tmp177 - 10 * s2n * tmp282 + 7 * s3n * tmp282 -
                  10 * s4n * tmp282 - 8 * s2n * tmp362 - 16 * s3n * tmp362 -
                  8 * s4n * tmp362 - 8 * s2n * tmp366 - 8 * s4n * tmp366 -
                  8 * tmp215 * tmp465 +
                  4 * tmp23 *
                      (3 * s2n * s34 + 2 * s34 * tmp227 + s24 * tmp481 +
                       s23 * (tmp219 + tmp479 + tmp488)) +
                  16 * s14 * s2n * tmp58 - 10 * s23 * s2n * tmp58 +
                  156 * s24 * s2n * tmp58 - 18 * s2n * s34 * tmp58 -
                  16 * s14 * s3n * tmp58 + 92 * s23 * s3n * tmp58 +
                  244 * s24 * s3n * tmp58 + 92 * s34 * s3n * tmp58 -
                  8 * s14 * s4n * tmp58 - 18 * s23 * s4n * tmp58 +
                  156 * s24 * s4n * tmp58 - 10 * s34 * s4n * tmp58 +
                  16 * s13 * s2n * tmp61 - 68 * s23 * s2n * tmp61 +
                  14 * s24 * s2n * tmp61 - 50 * s2n * s34 * tmp61 -
                  40 * s23 * s3n * tmp61 + 32 * s24 * s3n * tmp61 -
                  32 * s34 * s3n * tmp61 - 8 * s13 * s4n * tmp61 +
                  36 * s23 * s4n * tmp61 + 70 * s24 * s4n * tmp61 +
                  38 * s34 * s4n * tmp61 + 66 * s13 * s2n * tmp65 +
                  230 * s14 * s2n * tmp65 + 68 * s24 * s2n * tmp65 -
                  123 * s2n * s34 * tmp65 - 12 * s13 * s3n * tmp65 +
                  158 * s14 * s3n * tmp65 - 21 * s24 * s3n * tmp65 -
                  328 * s34 * s3n * tmp65 + 182 * s13 * s4n * tmp65 +
                  96 * s14 * s4n * tmp65 - 138 * s24 * s4n * tmp65 -
                  284 * s34 * s4n * tmp65 - 34 * s13 * s2n * tmp70 +
                  28 * s14 * s2n * tmp70 + 30 * s23 * s2n * tmp70 -
                  29 * s2n * s34 * tmp70 - 176 * s13 * s3n * tmp70 -
                  38 * s14 * s3n * tmp70 + 44 * s23 * s3n * tmp70 +
                  44 * s34 * s3n * tmp70 - 34 * s13 * s4n * tmp70 -
                  40 * s14 * s4n * tmp70 - 29 * s23 * s4n * tmp70 +
                  30 * s34 * s4n * tmp70 + 182 * s13 * s2n * tmp76 +
                  214 * s14 * s2n * tmp76 - 284 * s23 * s2n * tmp76 -
                  138 * s24 * s2n * tmp76 - 12 * s13 * s3n * tmp76 +
                  122 * s14 * s3n * tmp76 - 328 * s23 * s3n * tmp76 -
                  21 * s24 * s3n * tmp76 + 66 * s13 * s4n * tmp76 -
                  32 * s14 * s4n * tmp76 - 123 * s23 * s4n * tmp76 +
                  68 * s24 * s4n * tmp76 +
                  2 * (19 * s23 * s2n + 35 * s24 * s2n + 18 * s2n * s34 - 16 * s23 * s3n + 16 * s24 * s3n - 20 * s34 * s3n - 25 * s23 * s4n + 7 * s24 * s4n - 34 * s34 * s4n - 4 * s13 * (s2n + tmp473) + 8 * s14 * tmp481) *
                      tmp93 +
                  4 * m12 *
                      (10 * s14 * s23 * s2n + s14 * s24 * s2n +
                       11 * s23 * s24 * s2n + 6 * s14 * s2n * s34 +
                       15 * s23 * s2n * s34 + 7 * s24 * s2n * s34 +
                       4 * s14 * s23 * s3n - 3 * s14 * s24 * s3n -
                       10 * s23 * s24 * s3n + 4 * s14 * s34 * s3n -
                       26 * s23 * s34 * s3n - 10 * s24 * s34 * s3n -
                       10 * s14 * s23 * s4n - 10 * s14 * s24 * s4n +
                       7 * s23 * s24 * s4n - 8 * s14 * s34 * s4n +
                       15 * s23 * s34 * s4n + 11 * s24 * s34 * s4n + tmp226 +
                       tmp236 -
                       s13 * (3 * s14 * s2n - 2 * s2n * s34 + 18 * s24 * s3n -
                              4 * s34 * s4n + tmp118 + tmp125 + tmp525 -
                              2 * s23 * (s4n + tmp479 + tmp527)) -
                       s12 * (10 * s24 * s2n + 10 * s2n * s34 - 4 * s34 * s3n +
                              3 * s13 * s4n - s24 * s4n - 10 * s34 * s4n +
                              s23 * (8 * s2n - 6 * s4n + tmp527) + tmp534 +
                              s14 * tmp544) +
                       tmp220 * tmp58 + s2n * tmp61 + s4n * tmp61 +
                       9 * s2n * tmp65 - 14 * s3n * tmp65 - s2n * tmp70 -
                       7 * s3n * tmp70 - s4n * tmp70 - 14 * s3n * tmp76 +
                       9 * s4n * tmp76 + tmp215 * tmp93) -
                  2 * s12 *
                      (78 * s23 * s24 * s2n - 14 * s23 * s2n * s34 +
                       74 * s24 * s2n * s34 + 23 * s23 * s24 * s3n -
                       140 * s23 * s34 * s3n + 40 * s24 * s34 * s3n -
                       56 * s23 * s24 * s4n - 204 * s23 * s34 * s4n -
                       36 * s24 * s34 * s4n +
                       4 * (s2n + tmp219 + tmp473) * tmp58 -
                       s13 *
                           (91 * s24 * s2n - 20 * s2n * s34 + 123 * s24 * s3n +
                            4 * s34 * s3n + 30 * s24 * s4n - 108 * s34 * s4n +
                            8 * s14 * (s3n + tmp577 + tmp578) -
                            8 * s23 * (s2n + 12 * s4n + tmp581)) -
                       8 * tmp481 * tmp61 + 16 * s2n * tmp65 -
                       61 * s3n * tmp65 - 107 * s4n * tmp65 + 20 * s2n * tmp70 +
                       19 * s3n * tmp70 - 14 * s4n * tmp70 - 48 * s2n * tmp76 -
                       79 * s3n * tmp76 - 115 * s4n * tmp76 +
                       s14 * (2 * s23 * (53 * s2n + 49 * s4n + tmp19) +
                              2 * s34 * (49 * s2n + 53 * s4n + tmp19) -
                              s24 * (52 * s3n + 7 * s4n + tmp94)))) -
             m22 *
                 (12 * s13 * s14 * s23 * s24 * s2n -
                  110 * s13 * s14 * s23 * s2n * s34 -
                  47 * s13 * s23 * s24 * s2n * s34 +
                  8 * s14 * s23 * s24 * s2n * s34 +
                  99 * s13 * s14 * s23 * s24 * s3n -
                  8 * s13 * s14 * s23 * s34 * s3n +
                  87 * s13 * s14 * s24 * s34 * s3n -
                  164 * s13 * s23 * s24 * s34 * s3n -
                  15 * s14 * s23 * s24 * s34 * s3n +
                  75 * s13 * s14 * s23 * s24 * s4n -
                  8 * s13 * s14 * s23 * s34 * s4n +
                  63 * s13 * s14 * s24 * s34 * s4n -
                  47 * s13 * s23 * s24 * s34 * s4n -
                  118 * s14 * s23 * s24 * s34 * s4n + 4 * s13 * s2n * tmp164 +
                  36 * s14 * s2n * tmp164 + 16 * s24 * s2n * tmp164 +
                  8 * s2n * s34 * tmp164 + 4 * s13 * s3n * tmp164 +
                  34 * s14 * s3n * tmp164 - 4 * s24 * s3n * tmp164 -
                  68 * s34 * s3n * tmp164 + 44 * s13 * s4n * tmp164 +
                  22 * s14 * s4n * tmp164 - 42 * s24 * s4n * tmp164 -
                  56 * s34 * s4n * tmp164 + 44 * s13 * s2n * tmp177 +
                  44 * s14 * s2n * tmp177 - 56 * s23 * s2n * tmp177 -
                  42 * s24 * s2n * tmp177 + 4 * s13 * s3n * tmp177 +
                  20 * s14 * s3n * tmp177 - 68 * s23 * s3n * tmp177 -
                  4 * s24 * s3n * tmp177 + 4 * s13 * s4n * tmp177 -
                  12 * s14 * s4n * tmp177 + 8 * s23 * s4n * tmp177 +
                  16 * s24 * s4n * tmp177 - 28 * s2n * tmp181 -
                  12 * s3n * tmp181 + 4 * s4n * tmp181 + 4 * s2n * tmp201 -
                  12 * s3n * tmp201 - 28 * s4n * tmp201 + tmp207 +
                  9 * s13 * s2n * tmp282 + 4 * s14 * s2n * tmp282 -
                  5 * s2n * s34 * tmp282 - 8 * s13 * s3n * tmp282 -
                  s14 * s3n * tmp282 + 3 * s23 * s3n * tmp282 +
                  3 * s34 * s3n * tmp282 + 9 * s13 * s4n * tmp282 -
                  5 * s23 * s4n * tmp282 + s3n * tmp297 -
                  4 * s23 * s2n * tmp362 - 24 * s24 * s2n * tmp362 -
                  4 * s2n * s34 * tmp362 - 8 * s23 * s3n * tmp362 -
                  24 * s24 * s3n * tmp362 - 8 * s34 * s3n * tmp362 -
                  4 * s23 * s4n * tmp362 - 24 * s24 * s4n * tmp362 -
                  4 * s34 * s4n * tmp362 - 8 * s23 * s2n * tmp366 -
                  4 * s24 * s2n * tmp366 - 4 * s2n * s34 * tmp366 -
                  8 * s23 * s4n * tmp366 - 4 * s24 * s4n * tmp366 -
                  4 * s34 * s4n * tmp366 + 12 * s14 * s23 * s2n * tmp58 -
                  32 * s14 * s24 * s2n * tmp58 + 55 * s23 * s24 * s2n * tmp58 +
                  12 * s14 * s2n * s34 * tmp58 - 16 * s23 * s2n * s34 * tmp58 +
                  51 * s24 * s2n * s34 * tmp58 - 8 * s14 * s23 * s3n * tmp58 -
                  44 * s14 * s24 * s3n * tmp58 + 88 * s23 * s24 * s3n * tmp58 -
                  8 * s14 * s34 * s3n * tmp58 + 32 * s23 * s34 * s3n * tmp58 +
                  88 * s24 * s34 * s3n * tmp58 - 4 * s14 * s23 * s4n * tmp58 -
                  44 * s14 * s24 * s4n * tmp58 + 51 * s23 * s24 * s4n * tmp58 -
                  4 * s14 * s34 * s4n * tmp58 - 16 * s23 * s34 * s4n * tmp58 +
                  55 * s24 * s34 * s4n * tmp58 + 8 * s13 * s23 * s2n * tmp61 -
                  12 * s13 * s24 * s2n * tmp61 + 19 * s23 * s24 * s2n * tmp61 +
                  12 * s13 * s2n * s34 * tmp61 - 21 * s23 * s2n * s34 * tmp61 -
                  20 * s13 * s24 * s3n * tmp61 + 16 * s23 * s24 * s3n * tmp61 -
                  24 * s23 * s34 * s3n * tmp61 + 4 * s24 * s34 * s3n * tmp61 -
                  8 * s13 * s23 * s4n * tmp61 - 24 * s13 * s24 * s4n * tmp61 +
                  51 * s23 * s24 * s4n * tmp61 - 4 * s13 * s34 * s4n * tmp61 +
                  35 * s23 * s34 * s4n * tmp61 + 24 * s24 * s34 * s4n * tmp61 -
                  63 * s13 * s14 * s2n * tmp65 - 32 * s13 * s24 * s2n * tmp65 +
                  13 * s14 * s24 * s2n * tmp65 + 40 * s13 * s2n * s34 * tmp65 +
                  82 * s14 * s2n * s34 * tmp65 + 58 * s24 * s2n * s34 * tmp65 -
                  8 * s13 * s14 * s3n * tmp65 - 78 * s13 * s24 * s3n * tmp65 -
                  3 * s14 * s24 * s3n * tmp65 + 20 * s13 * s34 * s3n * tmp65 +
                  96 * s14 * s34 * s3n * tmp65 - 36 * s24 * s34 * s3n * tmp65 -
                  16 * s13 * s14 * s4n * tmp65 + 13 * s13 * s24 * s4n * tmp65 -
                  32 * s14 * s24 * s4n * tmp65 + 80 * s13 * s34 * s4n * tmp65 -
                  4 * s2n * tmp58 * tmp65 + 16 * s3n * tmp58 * tmp65 -
                  12 * s4n * tmp58 * tmp65 - 17 * s2n * tmp61 * tmp65 -
                  16 * s3n * tmp61 * tmp65 + 15 * s4n * tmp61 * tmp65 -
                  11 * s13 * s14 * s2n * tmp70 - 5 * s13 * s23 * s2n * tmp70 +
                  3 * s14 * s23 * s2n * tmp70 + 4 * s13 * s2n * s34 * tmp70 +
                  17 * s14 * s2n * s34 * tmp70 + 23 * s23 * s2n * s34 * tmp70 +
                  21 * s13 * s14 * s3n * tmp70 - 64 * s13 * s23 * s3n * tmp70 -
                  12 * s14 * s23 * s3n * tmp70 - 64 * s13 * s34 * s3n * tmp70 -
                  13 * s14 * s34 * s3n * tmp70 + 2 * s23 * s34 * s3n * tmp70 +
                  5 * s13 * s14 * s4n * tmp70 + 4 * s13 * s23 * s4n * tmp70 -
                  28 * s14 * s23 * s4n * tmp70 - 5 * s13 * s34 * s4n * tmp70 -
                  16 * s14 * s34 * s4n * tmp70 + 23 * s23 * s34 * s4n * tmp70 +
                  9 * s2n * tmp58 * tmp70 + 18 * s3n * tmp58 * tmp70 +
                  9 * s4n * tmp58 * tmp70 + 4 * s4n * tmp61 * tmp70 +
                  12 * s2n * tmp65 * tmp70 + 10 * s3n * tmp65 * tmp70 -
                  19 * s4n * tmp65 * tmp70 - 55 * s13 * s14 * s2n * tmp76 +
                  80 * s13 * s23 * s2n * tmp76 + 90 * s14 * s23 * s2n * tmp76 +
                  13 * s13 * s24 * s2n * tmp76 + 45 * s14 * s24 * s2n * tmp76 +
                  20 * s13 * s23 * s3n * tmp76 + 82 * s14 * s23 * s3n * tmp76 -
                  78 * s13 * s24 * s3n * tmp76 - 4 * s14 * s24 * s3n * tmp76 -
                  36 * s23 * s24 * s3n * tmp76 + 40 * s13 * s23 * s4n * tmp76 -
                  36 * s14 * s23 * s4n * tmp76 - 32 * s13 * s24 * s4n * tmp76 -
                  36 * s14 * s24 * s4n * tmp76 + 58 * s23 * s24 * s4n * tmp76 -
                  12 * s2n * tmp58 * tmp76 + 16 * s3n * tmp58 * tmp76 -
                  4 * s4n * tmp58 * tmp76 - 12 * s2n * tmp61 * tmp76 -
                  8 * s3n * tmp61 * tmp76 + 12 * s4n * tmp61 * tmp76 -
                  24 * s2n * tmp65 * tmp76 - 112 * s3n * tmp65 * tmp76 -
                  24 * s4n * tmp65 * tmp76 - 19 * s2n * tmp70 * tmp76 +
                  10 * s3n * tmp70 * tmp76 + 12 * s4n * tmp70 * tmp76 -
                  4 * tmp215 * tmp465 * tmp782 +
                  2 * tmp23 * tmp24 *
                      (s23 * (s2n + s3n + tmp488) + s24 * tmp784 +
                       s34 * (s3n + s4n + tmp786)) +
                  (24 * s23 * s24 * s2n + 35 * s23 * s2n * s34 +
                   51 * s24 * s2n * s34 + 4 * s23 * s24 * s3n -
                   24 * s23 * s34 * s3n + 16 * s24 * s34 * s3n -
                   21 * s23 * s34 * s4n + 19 * s24 * s34 * s4n +
                   12 * s2n * tmp65 - 8 * s3n * tmp65 - 12 * s4n * tmp65 +
                   4 * s2n * tmp70 + 15 * s2n * tmp76 - 16 * s3n * tmp76 -
                   17 * s4n * tmp76 -
                   4 * s13 *
                       (2 * s34 * (s2n - s4n) +
                        s24 * (6 * s2n + 5 * s3n + tmp488) +
                        s23 * (s2n + tmp808)) +
                   4 * s14 * (2 * s34 * tmp481 + s23 * tmp544 + tmp820)) *
                      tmp93 +
                  2 * m12 *
                      (4 * s14 * s23 * s2n * s34 + 19 * s23 * s24 * s2n * s34 +
                       8 * s14 * s23 * s34 * s3n + 3 * s14 * s24 * s34 * s3n -
                       16 * s23 * s24 * s34 * s3n - 14 * s14 * s23 * s24 * s4n -
                       14 * s14 * s23 * s34 * s4n - 6 * s14 * s24 * s34 * s4n +
                       19 * s23 * s24 * s34 * s4n + 4 * s2n * tmp164 -
                       6 * s3n * tmp164 - 6 * s3n * tmp177 + 4 * s4n * tmp177 +
                       s2n * tmp282 - 3 * s3n * tmp282 + s4n * tmp282 + tmp328 +
                       (6 * s24 * tmp114 + s23 * tmp220 + s34 * tmp220) *
                           tmp58 +
                       2 * s23 * s2n * tmp61 + s24 * s2n * tmp61 +
                       s2n * s34 * tmp61 + 2 * s23 * s4n * tmp61 +
                       s24 * s4n * tmp61 + s34 * s4n * tmp61 +
                       5 * s14 * s2n * tmp65 + 6 * s24 * s2n * tmp65 +
                       17 * s2n * s34 * tmp65 + 5 * s14 * s3n * tmp65 -
                       9 * s24 * s3n * tmp65 - 20 * s34 * s3n * tmp65 -
                       8 * s14 * s4n * tmp65 + 4 * s24 * s4n * tmp65 +
                       13 * s34 * s4n * tmp65 + 2 * s14 * s2n * tmp70 +
                       s23 * s2n * tmp70 + 2 * s2n * s34 * tmp70 +
                       3 * s14 * s3n * tmp70 - 9 * s23 * s3n * tmp70 -
                       9 * s34 * s3n * tmp70 + 2 * s23 * s4n * tmp70 +
                       s34 * s4n * tmp70 + s14 * s2n * tmp76 +
                       13 * s23 * s2n * tmp76 + 3 * s14 * s3n * tmp76 -
                       20 * s23 * s3n * tmp76 - 9 * s24 * s3n * tmp76 -
                       4 * s14 * s4n * tmp76 + 17 * s23 * s4n * tmp76 +
                       6 * s24 * s4n * tmp76 +
                       s13 * (-2 * s24 * s2n * s34 - 12 * s24 * s34 * s3n +
                              s24 * s34 * s4n +
                              s23 * s24 * (s2n - 2 * (6 * s3n + s4n)) +
                              s14 * (-4 * s23 * s2n + 2 * s24 * s2n -
                                     4 * s2n * s34 + 5 * s24 * s3n + tmp125) +
                              tmp226 + 5 * s4n * tmp70 - 2 * s3n * tmp76 +
                              3 * s4n * tmp76 + 4 * s23 * s34 * tmp784 +
                              tmp65 * (s4n + tmp47 + tmp786) + tmp880) +
                       s12 *
                           (5 * s13 * s24 * s2n - 14 * s24 * s2n * s34 +
                            5 * s13 * s24 * s3n + 2 * s13 * s24 * s4n -
                            4 * s13 * s34 * s4n +
                            s23 * (-6 * s24 * s2n - 14 * s2n * s34 -
                                   4 * s13 * s4n + 4 * s34 * s4n + tmp525 +
                                   tmp534) +
                            (3 * s3n + s4n + tmp130) * tmp65 + 3 * s3n * tmp70 +
                            2 * s4n * tmp70 - 8 * s2n * tmp76 +
                            5 * s3n * tmp76 + 5 * s4n * tmp76 +
                            s14 * (s24 * tmp353 - 2 * s23 * tmp923 -
                                   2 * s34 * tmp923)) +
                       tmp215 * tmp782 * tmp93) -
                  s12 *
                      (118 * s23 * s24 * s2n * s34 +
                       15 * s23 * s24 * s34 * s3n - 8 * s23 * s24 * s34 * s4n +
                       12 * s2n * tmp164 - 20 * s3n * tmp164 -
                       44 * s4n * tmp164 - 22 * s2n * tmp177 -
                       34 * s3n * tmp177 - 36 * s4n * tmp177 + s3n * tmp282 -
                       4 * s4n * tmp282 + 36 * s24 * s2n * tmp65 +
                       36 * s2n * s34 * tmp65 + 4 * s24 * s3n * tmp65 -
                       82 * s34 * s3n * tmp65 - 45 * s24 * s4n * tmp65 -
                       90 * s34 * s4n * tmp65 + 16 * s23 * s2n * tmp70 +
                       28 * s2n * s34 * tmp70 + 13 * s23 * s3n * tmp70 +
                       12 * s34 * s3n * tmp70 - 17 * s23 * s4n * tmp70 -
                       3 * s34 * s4n * tmp70 + 2 * s23 * s2n * tmp76 +
                       32 * s24 * s2n * tmp76 - 96 * s23 * s3n * tmp76 +
                       3 * s24 * s3n * tmp76 - 82 * s23 * s4n * tmp76 -
                       13 * s24 * s4n * tmp76 -
                       4 * tmp61 * (2 * s23 * tmp481 + s34 * tmp544 + tmp820) -
                       s13 * (75 * s24 * s2n * s34 + 99 * s24 * s34 * s3n +
                              s23 * s24 * (63 * s2n + 87 * s3n) +
                              12 * s24 * s34 * s4n -
                              2 * s23 * s34 * (tmp352 + tmp52 + tmp577) +
                              8 * s14 *
                                  (s23 * tmp1014 + s34 * tmp1014 -
                                   s24 * (7 * s3n + tmp577 + tmp578)) -
                              55 * s4n * tmp65 + 21 * s3n * tmp70 -
                              11 * s4n * tmp70 - 16 * s2n * tmp76 -
                              8 * s3n * tmp76 - 63 * s4n * tmp76 + tmp880) +
                       4 * tmp58 *
                           (s23 * tmp965 + s34 * tmp965 +
                            s24 * (11 * s3n + 8 * s4n + tmp968)) +
                       s14 * (-16 * s3n * tmp70 -
                              s23 * s24 * (11 * s4n + tmp786 + tmp976) -
                              s24 * s34 * (tmp488 + tmp968 + tmp976) +
                              2 * s23 * s34 * (43 * s2n + 43 * s4n + tmp983) +
                              tmp65 * (55 * s2n + 47 * s4n + tmp983) +
                              tmp76 * (47 * s2n + tmp52 + tmp983)))))) /
           (3. * (s23 + tmp3) * (s34 + tmp3) * pow(s23 + s24 + s34 + tmp3, 2) *
            (-s12 + s23 + tmp3 + tmp7) * (-s14 + s34 + tmp3 + tmp7));
}

MUSTARD_OPTIMIZE_FAST auto MSqM2ENNEE::TwoBorn(double s12, double s13, double s14, double s23, double s24, double s34,
                                               double m12, double m22, double m32) -> double {
    using muc::pow;

    // Adapt from McMule v0.5.1, mudecrare/mudecrare_1l_twotrace.opt.f95, FUNCTION born
    //
    // Copyright (C) 2020-2025  Yannick Ulrich and others (The McMule development team)
    //

    const auto tmp1{2 * m32};
    const auto tmp12{pow(m32, 3)};
    const auto tmp11{pow(m12, 2)};
    const auto tmp14{pow(m22, 2)};
    const auto tmp16{pow(m32, 4)};
    const auto tmp25{pow(s12, 2)};
    const auto tmp31{pow(m32, 2)};
    const auto tmp29{pow(s12, 3)};
    const auto tmp45{pow(s13, 2)};
    const auto tmp56{pow(s13, 3)};
    const auto tmp93{pow(s14, 2)};
    const auto tmp110{pow(s14, 3)};
    const auto tmp61{pow(s13, 4)};
    const auto tmp202{pow(s23, 2)};
    const auto tmp116{pow(s14, 4)};
    const auto tmp257{pow(s23, 3)};
    const auto tmp275{pow(s23, 4)};
    const auto tmp451{pow(s24, 2)};
    const auto tmp537{pow(s24, 3)};
    const auto tmp560{pow(s24, 4)};
    const auto tmp1041{pow(s34, 2)};
    const auto tmp1278{pow(s34, 3)};
    const auto tmp1360{pow(s34, 4)};
    return (128 *
            (128 * pow(m32, 5) * s12 - 24 * m12 * m22 * m32 * s12 * s13 * s14 -
             16 * m12 * m22 * m32 * s12 * s13 * s23 -
             16 * m12 * m22 * m32 * s12 * s14 * s23 +
             8 * m12 * m22 * m32 * s13 * s14 * s23 +
             36 * m12 * m32 * s12 * s13 * s14 * s23 +
             4 * m22 * m32 * s12 * s13 * s14 * s23 -
             16 * m12 * m22 * m32 * s12 * s13 * s24 -
             16 * m12 * m22 * m32 * s12 * s14 * s24 +
             8 * m12 * m22 * m32 * s13 * s14 * s24 +
             36 * m12 * m32 * s12 * s13 * s14 * s24 +
             4 * m22 * m32 * s12 * s13 * s14 * s24 -
             24 * m12 * m22 * m32 * s12 * s23 * s24 -
             8 * m12 * m22 * m32 * s13 * s23 * s24 -
             4 * m12 * m32 * s12 * s13 * s23 * s24 -
             36 * m22 * m32 * s12 * s13 * s23 * s24 -
             8 * m12 * m22 * m32 * s14 * s23 * s24 -
             4 * m12 * m32 * s12 * s14 * s23 * s24 -
             36 * m22 * m32 * s12 * s14 * s23 * s24 -
             16 * m12 * m22 * s13 * s14 * s23 * s24 -
             64 * m12 * m32 * s13 * s14 * s23 * s24 -
             64 * m22 * m32 * s13 * s14 * s23 * s24 +
             12 * m12 * s12 * s13 * s14 * s23 * s24 +
             12 * m22 * s12 * s13 * s14 * s23 * s24 +
             160 * m32 * s12 * s13 * s14 * s23 * s24 +
             16 * m12 * m22 * m32 * s12 * s13 * s34 +
             16 * m12 * m22 * m32 * s12 * s14 * s34 +
             160 * m12 * m22 * m32 * s13 * s14 * s34 -
             12 * m12 * m22 * s12 * s13 * s14 * s34 +
             36 * m12 * m32 * s12 * s13 * s14 * s34 -
             156 * m22 * m32 * s12 * s13 * s14 * s34 -
             16 * m12 * m22 * m32 * s12 * s23 * s34 -
             96 * m12 * m22 * m32 * s13 * s23 * s34 -
             8 * m12 * m22 * s12 * s13 * s23 * s34 -
             44 * m12 * m32 * s12 * s13 * s23 * s34 -
             44 * m22 * m32 * s12 * s13 * s23 * s34 -
             128 * m12 * m22 * m32 * s14 * s23 * s34 -
             8 * m12 * m22 * s12 * s14 * s23 * s34 -
             20 * m12 * m32 * s12 * s14 * s23 * s34 -
             20 * m22 * m32 * s12 * s14 * s23 * s34 +
             4 * m12 * m22 * s13 * s14 * s23 * s34 -
             56 * m12 * m32 * s13 * s14 * s23 * s34 +
             72 * m22 * m32 * s13 * s14 * s23 * s34 +
             18 * m12 * s12 * s13 * s14 * s23 * s34 +
             2 * m22 * s12 * s13 * s14 * s23 * s34 +
             248 * m32 * s12 * s13 * s14 * s23 * s34 -
             16 * m12 * m22 * m32 * s12 * s24 * s34 -
             128 * m12 * m22 * m32 * s13 * s24 * s34 -
             8 * m12 * m22 * s12 * s13 * s24 * s34 -
             20 * m12 * m32 * s12 * s13 * s24 * s34 -
             20 * m22 * m32 * s12 * s13 * s24 * s34 -
             96 * m12 * m22 * m32 * s14 * s24 * s34 -
             8 * m12 * m22 * s12 * s14 * s24 * s34 -
             44 * m12 * m32 * s12 * s14 * s24 * s34 -
             44 * m22 * m32 * s12 * s14 * s24 * s34 +
             4 * m12 * m22 * s13 * s14 * s24 * s34 -
             56 * m12 * m32 * s13 * s14 * s24 * s34 +
             72 * m22 * m32 * s13 * s14 * s24 * s34 +
             18 * m12 * s12 * s13 * s14 * s24 * s34 +
             2 * m22 * s12 * s13 * s14 * s24 * s34 +
             248 * m32 * s12 * s13 * s14 * s24 * s34 +
             160 * m12 * m22 * m32 * s23 * s24 * s34 -
             12 * m12 * m22 * s12 * s23 * s24 * s34 -
             156 * m12 * m32 * s12 * s23 * s24 * s34 +
             36 * m22 * m32 * s12 * s23 * s24 * s34 -
             4 * m12 * m22 * s13 * s23 * s24 * s34 -
             72 * m12 * m32 * s13 * s23 * s24 * s34 +
             56 * m22 * m32 * s13 * s23 * s24 * s34 -
             2 * m12 * s12 * s13 * s23 * s24 * s34 -
             18 * m22 * s12 * s13 * s23 * s24 * s34 -
             248 * m32 * s12 * s13 * s23 * s24 * s34 -
             4 * m12 * m22 * s14 * s23 * s24 * s34 -
             72 * m12 * m32 * s14 * s23 * s24 * s34 +
             56 * m22 * m32 * s14 * s23 * s24 * s34 -
             2 * m12 * s12 * s14 * s23 * s24 * s34 -
             18 * m22 * s12 * s14 * s23 * s24 * s34 -
             248 * m32 * s12 * s14 * s23 * s24 * s34 -
             24 * m12 * s13 * s14 * s23 * s24 * s34 -
             24 * m22 * s13 * s14 * s23 * s24 * s34 -
             288 * m32 * s13 * s14 * s23 * s24 * s34 +
             80 * s12 * s13 * s14 * s23 * s24 * s34 + 4 * s12 * pow(s34, 5) -
             24 * m12 * m22 * m32 * s12 * tmp1041 -
             200 * m12 * m22 * m32 * s13 * tmp1041 +
             4 * m12 * m22 * s12 * s13 * tmp1041 -
             28 * m12 * m32 * s12 * s13 * tmp1041 +
             68 * m22 * m32 * s12 * s13 * tmp1041 -
             200 * m12 * m22 * m32 * s14 * tmp1041 +
             4 * m12 * m22 * s12 * s14 * tmp1041 -
             28 * m12 * m32 * s12 * s14 * tmp1041 +
             68 * m22 * m32 * s12 * s14 * tmp1041 +
             36 * m12 * m22 * s13 * s14 * tmp1041 +
             240 * m22 * m32 * s13 * s14 * tmp1041 +
             12 * m12 * s12 * s13 * s14 * tmp1041 -
             36 * m22 * s12 * s13 * s14 * tmp1041 +
             136 * m32 * s12 * s13 * s14 * tmp1041 +
             200 * m12 * m22 * m32 * s23 * tmp1041 -
             4 * m12 * m22 * s12 * s23 * tmp1041 -
             68 * m12 * m32 * s12 * s23 * tmp1041 +
             28 * m22 * m32 * s12 * s23 * tmp1041 -
             20 * m12 * m22 * s13 * s23 * tmp1041 -
             8 * m12 * m32 * s13 * s23 * tmp1041 -
             8 * m22 * m32 * s13 * s23 * tmp1041 -
             10 * m12 * s12 * s13 * s23 * tmp1041 -
             10 * m22 * s12 * s13 * s23 * tmp1041 -
             232 * m32 * s12 * s13 * s23 * tmp1041 -
             28 * m12 * m22 * s14 * s23 * tmp1041 -
             56 * m12 * m32 * s14 * s23 * tmp1041 -
             56 * m22 * m32 * s14 * s23 * tmp1041 -
             4 * m12 * s12 * s14 * s23 * tmp1041 -
             4 * m22 * s12 * s14 * s23 * tmp1041 -
             184 * m32 * s12 * s14 * s23 * tmp1041 -
             6 * m12 * s13 * s14 * s23 * tmp1041 +
             26 * m22 * s13 * s14 * s23 * tmp1041 -
             104 * m32 * s13 * s14 * s23 * tmp1041 +
             56 * s12 * s13 * s14 * s23 * tmp1041 +
             200 * m12 * m22 * m32 * s24 * tmp1041 -
             4 * m12 * m22 * s12 * s24 * tmp1041 -
             68 * m12 * m32 * s12 * s24 * tmp1041 +
             28 * m22 * m32 * s12 * s24 * tmp1041 -
             28 * m12 * m22 * s13 * s24 * tmp1041 -
             56 * m12 * m32 * s13 * s24 * tmp1041 -
             56 * m22 * m32 * s13 * s24 * tmp1041 -
             4 * m12 * s12 * s13 * s24 * tmp1041 -
             4 * m22 * s12 * s13 * s24 * tmp1041 -
             184 * m32 * s12 * s13 * s24 * tmp1041 -
             20 * m12 * m22 * s14 * s24 * tmp1041 -
             8 * m12 * m32 * s14 * s24 * tmp1041 -
             8 * m22 * m32 * s14 * s24 * tmp1041 -
             10 * m12 * s12 * s14 * s24 * tmp1041 -
             10 * m22 * s12 * s14 * s24 * tmp1041 -
             232 * m32 * s12 * s14 * s24 * tmp1041 -
             6 * m12 * s13 * s14 * s24 * tmp1041 +
             26 * m22 * s13 * s14 * s24 * tmp1041 -
             104 * m32 * s13 * s14 * s24 * tmp1041 +
             56 * s12 * s13 * s14 * s24 * tmp1041 +
             36 * m12 * m22 * s23 * s24 * tmp1041 +
             240 * m12 * m32 * s23 * s24 * tmp1041 -
             36 * m12 * s12 * s23 * s24 * tmp1041 +
             12 * m22 * s12 * s23 * s24 * tmp1041 +
             136 * m32 * s12 * s23 * s24 * tmp1041 -
             26 * m12 * s13 * s23 * s24 * tmp1041 +
             6 * m22 * s13 * s23 * s24 * tmp1041 +
             104 * m32 * s13 * s23 * s24 * tmp1041 -
             56 * s12 * s13 * s23 * s24 * tmp1041 -
             26 * m12 * s14 * s23 * s24 * tmp1041 +
             6 * m22 * s14 * s23 * s24 * tmp1041 +
             104 * m32 * s14 * s23 * s24 * tmp1041 -
             56 * s12 * s14 * s23 * s24 * tmp1041 -
             56 * s13 * s14 * s23 * s24 * tmp1041 +
             32 * m22 * m32 * s23 * s24 * tmp11 -
             24 * m32 * s12 * s23 * s24 * tmp11 -
             24 * m32 * s13 * s23 * s24 * tmp11 -
             24 * m32 * s14 * s23 * s24 * tmp11 +
             64 * m22 * m32 * s23 * s34 * tmp11 -
             48 * m32 * s12 * s23 * s34 * tmp11 -
             24 * m32 * s13 * s23 * s34 * tmp11 -
             24 * m32 * s14 * s23 * s34 * tmp11 +
             64 * m22 * m32 * s24 * s34 * tmp11 -
             48 * m32 * s12 * s24 * s34 * tmp11 -
             24 * m32 * s13 * s24 * s34 * tmp11 -
             24 * m32 * s14 * s24 * s34 * tmp11 +
             16 * m22 * s23 * s24 * s34 * tmp11 +
             144 * m32 * s23 * s24 * s34 * tmp11 -
             12 * s12 * s23 * s24 * s34 * tmp11 -
             12 * s13 * s23 * s24 * s34 * tmp11 -
             12 * s14 * s23 * s24 * s34 * tmp11 +
             48 * m22 * m32 * tmp1041 * tmp11 -
             36 * m32 * s12 * tmp1041 * tmp11 +
             16 * m22 * s23 * tmp1041 * tmp11 +
             72 * m32 * s23 * tmp1041 * tmp11 -
             12 * s12 * s23 * tmp1041 * tmp11 -
             6 * s13 * s23 * tmp1041 * tmp11 - 6 * s14 * s23 * tmp1041 * tmp11 +
             16 * m22 * s24 * tmp1041 * tmp11 +
             72 * m32 * s24 * tmp1041 * tmp11 -
             12 * s12 * s24 * tmp1041 * tmp11 -
             6 * s13 * s24 * tmp1041 * tmp11 - 6 * s14 * s24 * tmp1041 * tmp11 +
             36 * s23 * s24 * tmp1041 * tmp11 - 12 * m12 * m22 * m32 * tmp110 +
             16 * m22 * m32 * s12 * tmp110 + 32 * m22 * m32 * s13 * tmp110 -
             16 * m22 * m32 * s23 * tmp110 - 16 * m32 * s13 * s23 * tmp110 +
             12 * m12 * m32 * s24 * tmp110 - 4 * m22 * m32 * s24 * tmp110 -
             16 * m32 * s12 * s24 * tmp110 - 32 * m32 * s13 * s24 * tmp110 +
             8 * m32 * s23 * s24 * tmp110 - 8 * s13 * s23 * s24 * tmp110 -
             6 * m12 * m22 * s34 * tmp110 - 64 * m22 * m32 * s34 * tmp110 +
             8 * m22 * s12 * s34 * tmp110 - 4 * m32 * s12 * s34 * tmp110 +
             16 * m22 * s13 * s34 * tmp110 - 3 * m12 * s23 * s34 * tmp110 -
             11 * m22 * s23 * s34 * tmp110 - 12 * m32 * s23 * s34 * tmp110 +
             2 * s12 * s23 * s34 * tmp110 + 3 * m12 * s24 * s34 * tmp110 -
             5 * m22 * s24 * s34 * tmp110 + 36 * m32 * s24 * s34 * tmp110 -
             6 * s12 * s24 * s34 * tmp110 - 8 * s13 * s24 * s34 * tmp110 -
             16 * m22 * tmp1041 * tmp110 - 2 * s12 * tmp1041 * tmp110 -
             6 * s23 * tmp1041 * tmp110 + 6 * s24 * tmp1041 * tmp110 +
             8 * m22 * m32 * tmp116 - 8 * m32 * s24 * tmp116 +
             4 * m22 * s34 * tmp116 + 2 * s23 * s34 * tmp116 -
             2 * s24 * s34 * tmp116 - 32 * m12 * m22 * s12 * tmp12 -
             352 * m12 * m22 * s13 * tmp12 - 16 * m12 * s12 * s13 * tmp12 +
             112 * m22 * s12 * s13 * tmp12 - 352 * m12 * m22 * s14 * tmp12 -
             16 * m12 * s12 * s14 * tmp12 + 112 * m22 * s12 * s14 * tmp12 +
             320 * m22 * s13 * s14 * tmp12 + 160 * s12 * s13 * s14 * tmp12 +
             352 * m12 * m22 * s23 * tmp12 - 112 * m12 * s12 * s23 * tmp12 +
             16 * m22 * s12 * s23 * tmp12 + 32 * m12 * s13 * s23 * tmp12 +
             32 * m22 * s13 * s23 * tmp12 - 352 * s12 * s13 * s23 * tmp12 -
             32 * m12 * s14 * s23 * tmp12 - 32 * m22 * s14 * s23 * tmp12 -
             288 * s12 * s14 * s23 * tmp12 - 224 * s13 * s14 * s23 * tmp12 +
             352 * m12 * m22 * s24 * tmp12 - 112 * m12 * s12 * s24 * tmp12 +
             16 * m22 * s12 * s24 * tmp12 - 32 * m12 * s13 * s24 * tmp12 -
             32 * m22 * s13 * s24 * tmp12 - 288 * s12 * s13 * s24 * tmp12 +
             32 * m12 * s14 * s24 * tmp12 + 32 * m22 * s14 * s24 * tmp12 -
             352 * s12 * s14 * s24 * tmp12 - 224 * s13 * s14 * s24 * tmp12 +
             320 * m12 * s23 * s24 * tmp12 + 160 * s12 * s23 * s24 * tmp12 +
             224 * s13 * s23 * s24 * tmp12 + 224 * s14 * s23 * s24 * tmp12 +
             896 * m12 * m22 * s34 * tmp12 - 96 * m12 * s12 * s34 * tmp12 -
             96 * m22 * s12 * s34 * tmp12 - 256 * m22 * s13 * s34 * tmp12 -
             384 * s12 * s13 * s34 * tmp12 - 256 * m22 * s14 * s34 * tmp12 -
             384 * s12 * s14 * s34 * tmp12 + 256 * m12 * s23 * s34 * tmp12 +
             384 * s12 * s23 * s34 * tmp12 + 224 * s13 * s23 * s34 * tmp12 +
             96 * s14 * s23 * s34 * tmp12 + 256 * m12 * s24 * s34 * tmp12 +
             384 * s12 * s24 * s34 * tmp12 + 96 * s13 * s24 * s34 * tmp12 +
             224 * s14 * s24 * s34 * tmp12 + 320 * s12 * tmp1041 * tmp12 +
             64 * m22 * tmp11 * tmp12 - 48 * s12 * tmp11 * tmp12 +
             96 * s23 * tmp11 * tmp12 + 96 * s24 * tmp11 * tmp12 +
             160 * m12 * m22 * m32 * tmp1278 - 4 * m12 * m22 * s12 * tmp1278 -
             8 * m12 * m32 * s12 * tmp1278 - 8 * m22 * m32 * s12 * tmp1278 -
             28 * m12 * m22 * s13 * tmp1278 - 64 * m22 * m32 * s13 * tmp1278 -
             6 * m12 * s12 * s13 * tmp1278 + 10 * m22 * s12 * s13 * tmp1278 -
             96 * m32 * s12 * s13 * tmp1278 - 28 * m12 * m22 * s14 * tmp1278 -
             64 * m22 * m32 * s14 * tmp1278 - 6 * m12 * s12 * s14 * tmp1278 +
             10 * m22 * s12 * s14 * tmp1278 - 96 * m32 * s12 * s14 * tmp1278 +
             40 * m22 * s13 * s14 * tmp1278 + 24 * s12 * s13 * s14 * tmp1278 +
             28 * m12 * m22 * s23 * tmp1278 + 64 * m12 * m32 * s23 * tmp1278 -
             10 * m12 * s12 * s23 * tmp1278 + 6 * m22 * s12 * s23 * tmp1278 +
             96 * m32 * s12 * s23 * tmp1278 - 4 * m12 * s13 * s23 * tmp1278 -
             4 * m22 * s13 * s23 * tmp1278 + 40 * m32 * s13 * s23 * tmp1278 -
             36 * s12 * s13 * s23 * tmp1278 - 12 * m12 * s14 * s23 * tmp1278 -
             12 * m22 * s14 * s23 * tmp1278 + 8 * m32 * s14 * s23 * tmp1278 -
             28 * s12 * s14 * s23 * tmp1278 - 12 * s13 * s14 * s23 * tmp1278 +
             28 * m12 * m22 * s24 * tmp1278 + 64 * m12 * m32 * s24 * tmp1278 -
             10 * m12 * s12 * s24 * tmp1278 + 6 * m22 * s12 * s24 * tmp1278 +
             96 * m32 * s12 * s24 * tmp1278 - 12 * m12 * s13 * s24 * tmp1278 -
             12 * m22 * s13 * s24 * tmp1278 + 8 * m32 * s13 * s24 * tmp1278 -
             28 * s12 * s13 * s24 * tmp1278 - 4 * m12 * s14 * s24 * tmp1278 -
             4 * m22 * s14 * s24 * tmp1278 + 40 * m32 * s14 * s24 * tmp1278 -
             36 * s12 * s14 * s24 * tmp1278 - 12 * s13 * s14 * s24 * tmp1278 +
             40 * m12 * s23 * s24 * tmp1278 + 24 * s12 * s23 * s24 * tmp1278 +
             12 * s13 * s23 * s24 * tmp1278 + 12 * s14 * s23 * s24 * tmp1278 +
             8 * m22 * tmp11 * tmp1278 - 6 * s12 * tmp11 * tmp1278 +
             12 * s23 * tmp11 * tmp1278 + 12 * s24 * tmp11 * tmp1278 +
             16 * m12 * m22 * tmp1360 + 40 * m32 * s12 * tmp1360 -
             8 * m22 * s13 * tmp1360 - 12 * s12 * s13 * tmp1360 -
             8 * m22 * s14 * tmp1360 - 12 * s12 * s14 * tmp1360 +
             8 * m12 * s23 * tmp1360 + 12 * s12 * s23 * tmp1360 +
             4 * s13 * s23 * tmp1360 + 8 * m12 * s24 * tmp1360 +
             12 * s12 * s24 * tmp1360 + 4 * s14 * s24 * tmp1360 +
             32 * m12 * m32 * s13 * s14 * tmp14 -
             24 * m32 * s12 * s13 * s14 * tmp14 +
             24 * m32 * s13 * s14 * s23 * tmp14 +
             24 * m32 * s13 * s14 * s24 * tmp14 -
             64 * m12 * m32 * s13 * s34 * tmp14 +
             48 * m32 * s12 * s13 * s34 * tmp14 -
             64 * m12 * m32 * s14 * s34 * tmp14 +
             48 * m32 * s12 * s14 * s34 * tmp14 +
             16 * m12 * s13 * s14 * s34 * tmp14 +
             144 * m32 * s13 * s14 * s34 * tmp14 -
             12 * s12 * s13 * s14 * s34 * tmp14 -
             24 * m32 * s13 * s23 * s34 * tmp14 -
             24 * m32 * s14 * s23 * s34 * tmp14 +
             12 * s13 * s14 * s23 * s34 * tmp14 -
             24 * m32 * s13 * s24 * s34 * tmp14 -
             24 * m32 * s14 * s24 * s34 * tmp14 +
             12 * s13 * s14 * s24 * s34 * tmp14 +
             48 * m12 * m32 * tmp1041 * tmp14 -
             36 * m32 * s12 * tmp1041 * tmp14 -
             16 * m12 * s13 * tmp1041 * tmp14 -
             72 * m32 * s13 * tmp1041 * tmp14 +
             12 * s12 * s13 * tmp1041 * tmp14 -
             16 * m12 * s14 * tmp1041 * tmp14 -
             72 * m32 * s14 * tmp1041 * tmp14 +
             12 * s12 * s14 * tmp1041 * tmp14 +
             36 * s13 * s14 * tmp1041 * tmp14 -
             6 * s13 * s23 * tmp1041 * tmp14 - 6 * s14 * s23 * tmp1041 * tmp14 -
             6 * s13 * s24 * tmp1041 * tmp14 - 6 * s14 * s24 * tmp1041 * tmp14 -
             12 * m32 * tmp110 * tmp14 - 6 * s34 * tmp110 * tmp14 +
             64 * m12 * tmp12 * tmp14 - 48 * s12 * tmp12 * tmp14 -
             96 * s13 * tmp12 * tmp14 - 96 * s14 * tmp12 * tmp14 +
             8 * m12 * tmp1278 * tmp14 - 6 * s12 * tmp1278 * tmp14 -
             12 * s13 * tmp1278 * tmp14 - 12 * s14 * tmp1278 * tmp14 +
             512 * m12 * m22 * tmp16 - 64 * m12 * s12 * tmp16 -
             64 * m22 * s12 * tmp16 - 128 * m22 * s13 * tmp16 -
             192 * s12 * s13 * tmp16 - 128 * m22 * s14 * tmp16 -
             192 * s12 * s14 * tmp16 + 128 * m12 * s23 * tmp16 +
             192 * s12 * s23 * tmp16 + 128 * s13 * s23 * tmp16 +
             64 * s14 * s23 * tmp16 + 128 * m12 * s24 * tmp16 +
             192 * s12 * s24 * tmp16 + 64 * s13 * s24 * tmp16 +
             128 * s14 * s24 * tmp16 + 320 * s12 * s34 * tmp16 -
             12 * m12 * m22 * m32 * s12 * tmp202 -
             12 * m12 * m22 * m32 * s13 * tmp202 +
             4 * m12 * m32 * s12 * s13 * tmp202 -
             12 * m22 * m32 * s12 * s13 * tmp202 -
             28 * m12 * m22 * m32 * s14 * tmp202 +
             16 * m12 * m32 * s12 * s14 * tmp202 +
             4 * m12 * m32 * s13 * s14 * tmp202 +
             4 * m22 * m32 * s13 * s14 * tmp202 +
             32 * m32 * s12 * s13 * s14 * tmp202 +
             36 * m12 * m22 * m32 * s24 * tmp202 -
             48 * m12 * m32 * s12 * s24 * tmp202 -
             12 * m12 * m32 * s13 * s24 * tmp202 +
             36 * m22 * m32 * s13 * s24 * tmp202 -
             56 * m32 * s12 * s13 * s24 * tmp202 -
             24 * m12 * m32 * s14 * s24 * tmp202 +
             24 * m22 * m32 * s14 * s24 * tmp202 -
             40 * m32 * s12 * s14 * s24 * tmp202 -
             12 * m12 * s13 * s14 * s24 * tmp202 -
             12 * m22 * s13 * s14 * s24 * tmp202 -
             104 * m32 * s13 * s14 * s24 * tmp202 +
             16 * s12 * s13 * s14 * s24 * tmp202 +
             96 * m12 * m22 * m32 * s34 * tmp202 -
             6 * m12 * m22 * s12 * s34 * tmp202 -
             90 * m12 * m32 * s12 * s34 * tmp202 +
             6 * m22 * m32 * s12 * s34 * tmp202 -
             6 * m12 * m22 * s13 * s34 * tmp202 -
             32 * m12 * m32 * s13 * s34 * tmp202 +
             32 * m22 * m32 * s13 * s34 * tmp202 +
             2 * m12 * s12 * s13 * s34 * tmp202 -
             6 * m22 * s12 * s13 * s34 * tmp202 -
             116 * m32 * s12 * s13 * s34 * tmp202 -
             14 * m12 * m22 * s14 * s34 * tmp202 -
             88 * m12 * m32 * s14 * s34 * tmp202 -
             24 * m22 * m32 * s14 * s34 * tmp202 +
             8 * m12 * s12 * s14 * s34 * tmp202 -
             52 * m32 * s12 * s14 * s34 * tmp202 +
             6 * m12 * s13 * s14 * s34 * tmp202 +
             6 * m22 * s13 * s14 * s34 * tmp202 -
             64 * m32 * s13 * s14 * s34 * tmp202 +
             16 * s12 * s13 * s14 * s34 * tmp202 +
             18 * m12 * m22 * s24 * s34 * tmp202 +
             192 * m12 * m32 * s24 * s34 * tmp202 -
             24 * m12 * s12 * s24 * s34 * tmp202 +
             28 * m32 * s12 * s24 * s34 * tmp202 -
             15 * m12 * s13 * s24 * s34 * tmp202 +
             9 * m22 * s13 * s24 * s34 * tmp202 +
             108 * m32 * s13 * s24 * s34 * tmp202 -
             22 * s12 * s13 * s24 * s34 * tmp202 -
             21 * m12 * s14 * s24 * s34 * tmp202 +
             3 * m22 * s14 * s24 * s34 * tmp202 +
             60 * m32 * s14 * s24 * s34 * tmp202 -
             14 * s12 * s14 * s24 * s34 * tmp202 -
             40 * s13 * s14 * s24 * s34 * tmp202 +
             22 * m12 * m22 * tmp1041 * tmp202 +
             120 * m12 * m32 * tmp1041 * tmp202 -
             21 * m12 * s12 * tmp1041 * tmp202 +
             3 * m22 * s12 * tmp1041 * tmp202 +
             56 * m32 * s12 * tmp1041 * tmp202 -
             12 * m12 * s13 * tmp1041 * tmp202 +
             4 * m22 * s13 * tmp1041 * tmp202 +
             64 * m32 * s13 * tmp1041 * tmp202 -
             26 * s12 * s13 * tmp1041 * tmp202 -
             26 * m12 * s14 * tmp1041 * tmp202 -
             10 * m22 * s14 * tmp1041 * tmp202 -
             8 * m32 * s14 * tmp1041 * tmp202 -
             10 * s12 * s14 * tmp1041 * tmp202 -
             8 * s13 * s14 * tmp1041 * tmp202 +
             48 * m12 * s24 * tmp1041 * tmp202 +
             10 * s12 * s24 * tmp1041 * tmp202 +
             18 * s13 * s24 * tmp1041 * tmp202 +
             6 * s14 * s24 * tmp1041 * tmp202 +
             16 * m22 * m32 * tmp11 * tmp202 - 12 * m32 * s12 * tmp11 * tmp202 -
             12 * m32 * s13 * tmp11 * tmp202 - 12 * m32 * s14 * tmp11 * tmp202 +
             36 * m32 * s24 * tmp11 * tmp202 + 8 * m22 * s34 * tmp11 * tmp202 +
             72 * m32 * s34 * tmp11 * tmp202 - 6 * s12 * s34 * tmp11 * tmp202 -
             6 * s13 * s34 * tmp11 * tmp202 - 6 * s14 * s34 * tmp11 * tmp202 +
             18 * s24 * s34 * tmp11 * tmp202 + 18 * tmp1041 * tmp11 * tmp202 -
             6 * m12 * tmp110 * tmp202 - 6 * m22 * tmp110 * tmp202 -
             32 * m32 * tmp110 * tmp202 + 8 * s12 * tmp110 * tmp202 +
             8 * s13 * tmp110 * tmp202 - 8 * s24 * tmp110 * tmp202 -
             18 * s34 * tmp110 * tmp202 + 4 * tmp116 * tmp202 +
             160 * m12 * tmp12 * tmp202 + 64 * s12 * tmp12 * tmp202 +
             128 * s13 * tmp12 * tmp202 + 32 * s14 * tmp12 * tmp202 +
             20 * m12 * tmp1278 * tmp202 + 10 * s12 * tmp1278 * tmp202 +
             8 * s13 * tmp1278 * tmp202 - 4 * s14 * tmp1278 * tmp202 +
             16 * m22 * m32 * s13 * s14 * tmp25 +
             12 * m12 * m32 * s13 * s23 * tmp25 +
             12 * m22 * m32 * s13 * s23 * tmp25 +
             12 * m12 * m32 * s14 * s23 * tmp25 +
             12 * m22 * m32 * s14 * s23 * tmp25 -
             40 * m32 * s13 * s14 * s23 * tmp25 +
             12 * m12 * m32 * s13 * s24 * tmp25 +
             12 * m22 * m32 * s13 * s24 * tmp25 +
             12 * m12 * m32 * s14 * s24 * tmp25 +
             12 * m22 * m32 * s14 * s24 * tmp25 -
             40 * m32 * s13 * s14 * s24 * tmp25 +
             16 * m12 * m32 * s23 * s24 * tmp25 +
             40 * m32 * s13 * s23 * s24 * tmp25 +
             40 * m32 * s14 * s23 * s24 * tmp25 -
             8 * s13 * s14 * s23 * s24 * tmp25 +
             24 * m12 * m32 * s13 * s34 * tmp25 -
             8 * m22 * m32 * s13 * s34 * tmp25 +
             24 * m12 * m32 * s14 * s34 * tmp25 -
             8 * m22 * m32 * s14 * s34 * tmp25 +
             8 * m22 * s13 * s14 * s34 * tmp25 -
             56 * m32 * s13 * s14 * s34 * tmp25 +
             8 * m12 * m32 * s23 * s34 * tmp25 -
             24 * m22 * m32 * s23 * s34 * tmp25 +
             6 * m12 * s13 * s23 * s34 * tmp25 +
             6 * m22 * s13 * s23 * s34 * tmp25 +
             120 * m32 * s13 * s23 * s34 * tmp25 +
             6 * m12 * s14 * s23 * s34 * tmp25 +
             6 * m22 * s14 * s23 * s34 * tmp25 +
             104 * m32 * s14 * s23 * s34 * tmp25 -
             20 * s13 * s14 * s23 * s34 * tmp25 +
             8 * m12 * m32 * s24 * s34 * tmp25 -
             24 * m22 * m32 * s24 * s34 * tmp25 +
             6 * m12 * s13 * s24 * s34 * tmp25 +
             6 * m22 * s13 * s24 * s34 * tmp25 +
             104 * m32 * s13 * s24 * s34 * tmp25 +
             6 * m12 * s14 * s24 * s34 * tmp25 +
             6 * m22 * s14 * s24 * s34 * tmp25 +
             120 * m32 * s14 * s24 * s34 * tmp25 -
             20 * s13 * s14 * s24 * s34 * tmp25 +
             8 * m12 * s23 * s24 * s34 * tmp25 -
             56 * m32 * s23 * s24 * s34 * tmp25 +
             20 * s13 * s23 * s24 * s34 * tmp25 +
             20 * s14 * s23 * s24 * s34 * tmp25 -
             12 * m12 * m32 * tmp1041 * tmp25 -
             12 * m22 * m32 * tmp1041 * tmp25 +
             6 * m12 * s13 * tmp1041 * tmp25 - 2 * m22 * s13 * tmp1041 * tmp25 +
             96 * m32 * s13 * tmp1041 * tmp25 +
             6 * m12 * s14 * tmp1041 * tmp25 - 2 * m22 * s14 * tmp1041 * tmp25 +
             96 * m32 * s14 * tmp1041 * tmp25 -
             16 * s13 * s14 * tmp1041 * tmp25 +
             2 * m12 * s23 * tmp1041 * tmp25 - 6 * m22 * s23 * tmp1041 * tmp25 -
             96 * m32 * s23 * tmp1041 * tmp25 +
             28 * s13 * s23 * tmp1041 * tmp25 +
             24 * s14 * s23 * tmp1041 * tmp25 +
             2 * m12 * s24 * tmp1041 * tmp25 - 6 * m22 * s24 * tmp1041 * tmp25 -
             96 * m32 * s24 * tmp1041 * tmp25 +
             24 * s13 * s24 * tmp1041 * tmp25 +
             28 * s14 * s24 * tmp1041 * tmp25 -
             16 * s23 * s24 * tmp1041 * tmp25 - 16 * m12 * tmp12 * tmp25 -
             16 * m22 * tmp12 * tmp25 + 128 * s13 * tmp12 * tmp25 +
             128 * s14 * tmp12 * tmp25 - 128 * s23 * tmp12 * tmp25 -
             128 * s24 * tmp12 * tmp25 - 256 * s34 * tmp12 * tmp25 -
             2 * m12 * tmp1278 * tmp25 - 2 * m22 * tmp1278 * tmp25 -
             64 * m32 * tmp1278 * tmp25 + 16 * s13 * tmp1278 * tmp25 +
             16 * s14 * tmp1278 * tmp25 - 16 * s23 * tmp1278 * tmp25 -
             16 * s24 * tmp1278 * tmp25 - 8 * tmp1360 * tmp25 -
             128 * tmp16 * tmp25 + 8 * m12 * m32 * tmp202 * tmp25 +
             16 * m32 * s13 * tmp202 * tmp25 + 8 * m32 * s14 * tmp202 * tmp25 +
             4 * m12 * s34 * tmp202 * tmp25 - 20 * m32 * s34 * tmp202 * tmp25 +
             8 * s13 * s34 * tmp202 * tmp25 + 4 * s14 * s34 * tmp202 * tmp25 -
             6 * tmp1041 * tmp202 * tmp25 + 12 * m12 * m22 * m32 * tmp257 -
             16 * m12 * m32 * s12 * tmp257 - 4 * m12 * m32 * s13 * tmp257 +
             12 * m22 * m32 * s13 * tmp257 - 16 * m32 * s12 * s13 * tmp257 -
             16 * m12 * m32 * s14 * tmp257 - 8 * m32 * s13 * s14 * tmp257 +
             32 * m12 * m32 * s24 * tmp257 + 32 * m32 * s13 * s24 * tmp257 +
             16 * m32 * s14 * s24 * tmp257 - 8 * s13 * s14 * s24 * tmp257 +
             6 * m12 * m22 * s34 * tmp257 + 64 * m12 * m32 * s34 * tmp257 -
             8 * m12 * s12 * s34 * tmp257 + 4 * m32 * s12 * s34 * tmp257 -
             5 * m12 * s13 * s34 * tmp257 + 3 * m22 * s13 * s34 * tmp257 +
             36 * m32 * s13 * s34 * tmp257 - 6 * s12 * s13 * s34 * tmp257 -
             11 * m12 * s14 * s34 * tmp257 - 3 * m22 * s14 * s34 * tmp257 -
             12 * m32 * s14 * s34 * tmp257 + 2 * s12 * s14 * s34 * tmp257 +
             16 * m12 * s24 * s34 * tmp257 + 8 * s13 * s24 * s34 * tmp257 +
             16 * m12 * tmp1041 * tmp257 + 2 * s12 * tmp1041 * tmp257 +
             6 * s13 * tmp1041 * tmp257 - 6 * s14 * tmp1041 * tmp257 +
             12 * m32 * tmp11 * tmp257 + 6 * s34 * tmp11 * tmp257 -
             8 * tmp110 * tmp257 + 8 * m12 * m32 * tmp275 +
             8 * m32 * s13 * tmp275 + 4 * m12 * s34 * tmp275 +
             2 * s13 * s34 * tmp275 - 2 * s14 * s34 * tmp275 -
             8 * m32 * s13 * s23 * tmp29 - 8 * m32 * s14 * s23 * tmp29 -
             8 * m32 * s13 * s24 * tmp29 - 8 * m32 * s14 * s24 * tmp29 -
             16 * m32 * s13 * s34 * tmp29 - 16 * m32 * s14 * s34 * tmp29 +
             16 * m32 * s23 * s34 * tmp29 - 4 * s13 * s23 * s34 * tmp29 -
             4 * s14 * s23 * s34 * tmp29 + 16 * m32 * s24 * s34 * tmp29 -
             4 * s13 * s24 * s34 * tmp29 - 4 * s14 * s24 * s34 * tmp29 +
             24 * m32 * tmp1041 * tmp29 - 4 * s13 * tmp1041 * tmp29 -
             4 * s14 * tmp1041 * tmp29 + 4 * s23 * tmp1041 * tmp29 +
             4 * s24 * tmp1041 * tmp29 + 32 * tmp12 * tmp29 +
             4 * tmp1278 * tmp29 + 16 * m12 * m22 * s12 * s13 * tmp31 +
             16 * m12 * m22 * s12 * s14 * tmp31 +
             176 * m12 * m22 * s13 * s14 * tmp31 +
             24 * m12 * s12 * s13 * s14 * tmp31 -
             168 * m22 * s12 * s13 * s14 * tmp31 -
             16 * m12 * m22 * s12 * s23 * tmp31 -
             112 * m12 * m22 * s13 * s23 * tmp31 -
             48 * m12 * s12 * s13 * s23 * tmp31 -
             48 * m22 * s12 * s13 * s23 * tmp31 -
             144 * m12 * m22 * s14 * s23 * tmp31 -
             24 * m12 * s12 * s14 * s23 * tmp31 -
             24 * m22 * s12 * s14 * s23 * tmp31 -
             88 * m12 * s13 * s14 * s23 * tmp31 +
             40 * m22 * s13 * s14 * s23 * tmp31 +
             272 * s12 * s13 * s14 * s23 * tmp31 -
             16 * m12 * m22 * s12 * s24 * tmp31 -
             144 * m12 * m22 * s13 * s24 * tmp31 -
             24 * m12 * s12 * s13 * s24 * tmp31 -
             24 * m22 * s12 * s13 * s24 * tmp31 -
             112 * m12 * m22 * s14 * s24 * tmp31 -
             48 * m12 * s12 * s14 * s24 * tmp31 -
             48 * m22 * s12 * s14 * s24 * tmp31 -
             88 * m12 * s13 * s14 * s24 * tmp31 +
             40 * m22 * s13 * s14 * s24 * tmp31 +
             272 * s12 * s13 * s14 * s24 * tmp31 +
             176 * m12 * m22 * s23 * s24 * tmp31 -
             168 * m12 * s12 * s23 * s24 * tmp31 +
             24 * m22 * s12 * s23 * s24 * tmp31 -
             40 * m12 * s13 * s23 * s24 * tmp31 +
             88 * m22 * s13 * s23 * s24 * tmp31 -
             272 * s12 * s13 * s23 * s24 * tmp31 -
             40 * m12 * s14 * s23 * s24 * tmp31 +
             88 * m22 * s14 * s23 * s24 * tmp31 -
             272 * s12 * s14 * s23 * s24 * tmp31 -
             352 * s13 * s14 * s23 * s24 * tmp31 -
             48 * m12 * m22 * s12 * s34 * tmp31 -
             464 * m12 * m22 * s13 * s34 * tmp31 -
             40 * m12 * s12 * s13 * s34 * tmp31 +
             152 * m22 * s12 * s13 * s34 * tmp31 -
             464 * m12 * m22 * s14 * s34 * tmp31 -
             40 * m12 * s12 * s14 * s34 * tmp31 +
             152 * m22 * s12 * s14 * s34 * tmp31 +
             480 * m22 * s13 * s14 * s34 * tmp31 +
             256 * s12 * s13 * s14 * s34 * tmp31 +
             464 * m12 * m22 * s23 * s34 * tmp31 -
             152 * m12 * s12 * s23 * s34 * tmp31 +
             40 * m22 * s12 * s23 * s34 * tmp31 +
             16 * m12 * s13 * s23 * s34 * tmp31 +
             16 * m22 * s13 * s23 * s34 * tmp31 -
             496 * s12 * s13 * s23 * s34 * tmp31 -
             80 * m12 * s14 * s23 * s34 * tmp31 -
             80 * m22 * s14 * s23 * s34 * tmp31 -
             400 * s12 * s14 * s23 * s34 * tmp31 -
             272 * s13 * s14 * s23 * s34 * tmp31 +
             464 * m12 * m22 * s24 * s34 * tmp31 -
             152 * m12 * s12 * s24 * s34 * tmp31 +
             40 * m22 * s12 * s24 * s34 * tmp31 -
             80 * m12 * s13 * s24 * s34 * tmp31 -
             80 * m22 * s13 * s24 * s34 * tmp31 -
             400 * s12 * s13 * s24 * s34 * tmp31 +
             16 * m12 * s14 * s24 * s34 * tmp31 +
             16 * m22 * s14 * s24 * s34 * tmp31 -
             496 * s12 * s14 * s24 * s34 * tmp31 -
             272 * s13 * s14 * s24 * s34 * tmp31 +
             480 * m12 * s23 * s24 * s34 * tmp31 +
             256 * s12 * s23 * s24 * s34 * tmp31 +
             272 * s13 * s23 * s24 * s34 * tmp31 +
             272 * s14 * s23 * s24 * s34 * tmp31 +
             576 * m12 * m22 * tmp1041 * tmp31 -
             48 * m12 * s12 * tmp1041 * tmp31 -
             48 * m22 * s12 * tmp1041 * tmp31 -
             192 * m22 * s13 * tmp1041 * tmp31 -
             288 * s12 * s13 * tmp1041 * tmp31 -
             192 * m22 * s14 * tmp1041 * tmp31 -
             288 * s12 * s14 * tmp1041 * tmp31 +
             192 * m12 * s23 * tmp1041 * tmp31 +
             288 * s12 * s23 * tmp1041 * tmp31 +
             144 * s13 * s23 * tmp1041 * tmp31 +
             48 * s14 * s23 * tmp1041 * tmp31 +
             192 * m12 * s24 * tmp1041 * tmp31 +
             288 * s12 * s24 * tmp1041 * tmp31 +
             48 * s13 * s24 * tmp1041 * tmp31 +
             144 * s14 * s24 * tmp1041 * tmp31 +
             64 * m22 * s23 * tmp11 * tmp31 - 48 * s12 * s23 * tmp11 * tmp31 -
             24 * s13 * s23 * tmp11 * tmp31 - 24 * s14 * s23 * tmp11 * tmp31 +
             64 * m22 * s24 * tmp11 * tmp31 - 48 * s12 * s24 * tmp11 * tmp31 -
             24 * s13 * s24 * tmp11 * tmp31 - 24 * s14 * s24 * tmp11 * tmp31 +
             144 * s23 * s24 * tmp11 * tmp31 + 96 * m22 * s34 * tmp11 * tmp31 -
             72 * s12 * s34 * tmp11 * tmp31 + 144 * s23 * s34 * tmp11 * tmp31 +
             144 * s24 * s34 * tmp11 * tmp31 - 64 * m22 * tmp110 * tmp31 +
             48 * s24 * tmp110 * tmp31 + 160 * s12 * tmp1278 * tmp31 -
             64 * m12 * s13 * tmp14 * tmp31 + 48 * s12 * s13 * tmp14 * tmp31 -
             64 * m12 * s14 * tmp14 * tmp31 + 48 * s12 * s14 * tmp14 * tmp31 +
             144 * s13 * s14 * tmp14 * tmp31 - 24 * s13 * s23 * tmp14 * tmp31 -
             24 * s14 * s23 * tmp14 * tmp31 - 24 * s13 * s24 * tmp14 * tmp31 -
             24 * s14 * s24 * tmp14 * tmp31 + 96 * m12 * s34 * tmp14 * tmp31 -
             72 * s12 * s34 * tmp14 * tmp31 - 144 * s13 * s34 * tmp14 * tmp31 -
             144 * s14 * s34 * tmp14 * tmp31 +
             104 * m12 * m22 * tmp202 * tmp31 -
             96 * m12 * s12 * tmp202 * tmp31 - 16 * m12 * s13 * tmp202 * tmp31 +
             48 * m22 * s13 * tmp202 * tmp31 -
             128 * s12 * s13 * tmp202 * tmp31 -
             72 * m12 * s14 * tmp202 * tmp31 - 8 * m22 * s14 * tmp202 * tmp31 -
             64 * s12 * s14 * tmp202 * tmp31 - 96 * s13 * s14 * tmp202 * tmp31 +
             192 * m12 * s24 * tmp202 * tmp31 +
             16 * s12 * s24 * tmp202 * tmp31 +
             144 * s13 * s24 * tmp202 * tmp31 +
             96 * s14 * s24 * tmp202 * tmp31 +
             240 * m12 * s34 * tmp202 * tmp31 +
             104 * s12 * s34 * tmp202 * tmp31 +
             160 * s13 * s34 * tmp202 * tmp31 +
             16 * s14 * s34 * tmp202 * tmp31 + 72 * tmp11 * tmp202 * tmp31 +
             24 * m12 * s13 * tmp25 * tmp31 - 8 * m22 * s13 * tmp25 * tmp31 +
             24 * m12 * s14 * tmp25 * tmp31 - 8 * m22 * s14 * tmp25 * tmp31 -
             48 * s13 * s14 * tmp25 * tmp31 + 8 * m12 * s23 * tmp25 * tmp31 -
             24 * m22 * s23 * tmp25 * tmp31 + 128 * s13 * s23 * tmp25 * tmp31 +
             112 * s14 * s23 * tmp25 * tmp31 + 8 * m12 * s24 * tmp25 * tmp31 -
             24 * m22 * s24 * tmp25 * tmp31 + 112 * s13 * s24 * tmp25 * tmp31 +
             128 * s14 * s24 * tmp25 * tmp31 - 48 * s23 * s24 * tmp25 * tmp31 -
             24 * m12 * s34 * tmp25 * tmp31 - 24 * m22 * s34 * tmp25 * tmp31 +
             192 * s13 * s34 * tmp25 * tmp31 + 192 * s14 * s34 * tmp25 * tmp31 -
             192 * s23 * s34 * tmp25 * tmp31 - 192 * s24 * s34 * tmp25 * tmp31 -
             192 * tmp1041 * tmp25 * tmp31 - 16 * tmp202 * tmp25 * tmp31 +
             64 * m12 * tmp257 * tmp31 + 48 * s13 * tmp257 * tmp31 -
             16 * s13 * tmp29 * tmp31 - 16 * s14 * tmp29 * tmp31 +
             16 * s23 * tmp29 * tmp31 + 16 * s24 * tmp29 * tmp31 +
             48 * s34 * tmp29 * tmp31 - 12 * m12 * m22 * m32 * s12 * tmp45 -
             36 * m12 * m22 * m32 * s14 * tmp45 +
             48 * m22 * m32 * s12 * s14 * tmp45 +
             12 * m12 * m22 * m32 * s23 * tmp45 +
             12 * m12 * m32 * s12 * s23 * tmp45 -
             4 * m22 * m32 * s12 * s23 * tmp45 +
             36 * m12 * m32 * s14 * s23 * tmp45 -
             12 * m22 * m32 * s14 * s23 * tmp45 -
             56 * m32 * s12 * s14 * s23 * tmp45 +
             28 * m12 * m22 * m32 * s24 * tmp45 -
             16 * m22 * m32 * s12 * s24 * tmp45 +
             24 * m12 * m32 * s14 * s24 * tmp45 -
             24 * m22 * m32 * s14 * s24 * tmp45 -
             40 * m32 * s12 * s14 * s24 * tmp45 +
             4 * m12 * m32 * s23 * s24 * tmp45 +
             4 * m22 * m32 * s23 * s24 * tmp45 +
             32 * m32 * s12 * s23 * s24 * tmp45 +
             12 * m12 * s14 * s23 * s24 * tmp45 +
             12 * m22 * s14 * s23 * s24 * tmp45 +
             104 * m32 * s14 * s23 * s24 * tmp45 -
             16 * s12 * s14 * s23 * s24 * tmp45 +
             96 * m12 * m22 * m32 * s34 * tmp45 -
             6 * m12 * m22 * s12 * s34 * tmp45 +
             6 * m12 * m32 * s12 * s34 * tmp45 -
             90 * m22 * m32 * s12 * s34 * tmp45 -
             18 * m12 * m22 * s14 * s34 * tmp45 -
             192 * m22 * m32 * s14 * s34 * tmp45 +
             24 * m22 * s12 * s14 * s34 * tmp45 -
             28 * m32 * s12 * s14 * s34 * tmp45 +
             6 * m12 * m22 * s23 * s34 * tmp45 -
             32 * m12 * m32 * s23 * s34 * tmp45 +
             32 * m22 * m32 * s23 * s34 * tmp45 +
             6 * m12 * s12 * s23 * s34 * tmp45 -
             2 * m22 * s12 * s23 * s34 * tmp45 +
             116 * m32 * s12 * s23 * s34 * tmp45 +
             9 * m12 * s14 * s23 * s34 * tmp45 -
             15 * m22 * s14 * s23 * s34 * tmp45 +
             108 * m32 * s14 * s23 * s34 * tmp45 -
             22 * s12 * s14 * s23 * s34 * tmp45 +
             14 * m12 * m22 * s24 * s34 * tmp45 +
             24 * m12 * m32 * s24 * s34 * tmp45 +
             88 * m22 * m32 * s24 * s34 * tmp45 -
             8 * m22 * s12 * s24 * s34 * tmp45 +
             52 * m32 * s12 * s24 * s34 * tmp45 +
             3 * m12 * s14 * s24 * s34 * tmp45 -
             21 * m22 * s14 * s24 * s34 * tmp45 +
             60 * m32 * s14 * s24 * s34 * tmp45 -
             14 * s12 * s14 * s24 * s34 * tmp45 +
             6 * m12 * s23 * s24 * s34 * tmp45 +
             6 * m22 * s23 * s24 * s34 * tmp45 -
             64 * m32 * s23 * s24 * s34 * tmp45 +
             16 * s12 * s23 * s24 * s34 * tmp45 +
             40 * s14 * s23 * s24 * s34 * tmp45 +
             22 * m12 * m22 * tmp1041 * tmp45 +
             120 * m22 * m32 * tmp1041 * tmp45 +
             3 * m12 * s12 * tmp1041 * tmp45 -
             21 * m22 * s12 * tmp1041 * tmp45 +
             56 * m32 * s12 * tmp1041 * tmp45 -
             48 * m22 * s14 * tmp1041 * tmp45 -
             10 * s12 * s14 * tmp1041 * tmp45 -
             4 * m12 * s23 * tmp1041 * tmp45 +
             12 * m22 * s23 * tmp1041 * tmp45 -
             64 * m32 * s23 * tmp1041 * tmp45 +
             26 * s12 * s23 * tmp1041 * tmp45 +
             18 * s14 * s23 * tmp1041 * tmp45 +
             10 * m12 * s24 * tmp1041 * tmp45 +
             26 * m22 * s24 * tmp1041 * tmp45 +
             8 * m32 * s24 * tmp1041 * tmp45 +
             10 * s12 * s24 * tmp1041 * tmp45 +
             6 * s14 * s24 * tmp1041 * tmp45 - 8 * s23 * s24 * tmp1041 * tmp45 +
             160 * m22 * tmp12 * tmp45 + 64 * s12 * tmp12 * tmp45 -
             128 * s23 * tmp12 * tmp45 - 32 * s24 * tmp12 * tmp45 +
             20 * m22 * tmp1278 * tmp45 + 10 * s12 * tmp1278 * tmp45 -
             8 * s23 * tmp1278 * tmp45 + 4 * s24 * tmp1278 * tmp45 +
             16 * m12 * m32 * tmp14 * tmp45 - 12 * m32 * s12 * tmp14 * tmp45 -
             36 * m32 * s14 * tmp14 * tmp45 + 12 * m32 * s23 * tmp14 * tmp45 +
             12 * m32 * s24 * tmp14 * tmp45 + 8 * m12 * s34 * tmp14 * tmp45 +
             72 * m32 * s34 * tmp14 * tmp45 - 6 * s12 * s34 * tmp14 * tmp45 -
             18 * s14 * s34 * tmp14 * tmp45 + 6 * s23 * s34 * tmp14 * tmp45 +
             6 * s24 * s34 * tmp14 * tmp45 + 18 * tmp1041 * tmp14 * tmp45 +
             16 * m32 * s12 * tmp202 * tmp45 + 24 * m32 * s14 * tmp202 * tmp45 -
             24 * m32 * s24 * tmp202 * tmp45 + 16 * s14 * s24 * tmp202 * tmp45 +
             2 * m12 * s34 * tmp202 * tmp45 + 2 * m22 * s34 * tmp202 * tmp45 -
             48 * m32 * s34 * tmp202 * tmp45 + 8 * s12 * s34 * tmp202 * tmp45 +
             6 * s14 * s34 * tmp202 * tmp45 - 6 * s24 * s34 * tmp202 * tmp45 -
             8 * tmp1041 * tmp202 * tmp45 + 8 * m22 * m32 * tmp25 * tmp45 -
             16 * m32 * s23 * tmp25 * tmp45 - 8 * m32 * s24 * tmp25 * tmp45 +
             4 * m22 * s34 * tmp25 * tmp45 - 20 * m32 * s34 * tmp25 * tmp45 -
             8 * s23 * s34 * tmp25 * tmp45 - 4 * s24 * s34 * tmp25 * tmp45 -
             6 * tmp1041 * tmp25 * tmp45 - 8 * m32 * tmp257 * tmp45 -
             2 * s34 * tmp257 * tmp45 + 104 * m12 * m22 * tmp31 * tmp45 -
             96 * m22 * s12 * tmp31 * tmp45 - 192 * m22 * s14 * tmp31 * tmp45 -
             16 * s12 * s14 * tmp31 * tmp45 - 48 * m12 * s23 * tmp31 * tmp45 +
             16 * m22 * s23 * tmp31 * tmp45 + 128 * s12 * s23 * tmp31 * tmp45 +
             144 * s14 * s23 * tmp31 * tmp45 + 8 * m12 * s24 * tmp31 * tmp45 +
             72 * m22 * s24 * tmp31 * tmp45 + 64 * s12 * s24 * tmp31 * tmp45 +
             96 * s14 * s24 * tmp31 * tmp45 - 96 * s23 * s24 * tmp31 * tmp45 +
             240 * m22 * s34 * tmp31 * tmp45 + 104 * s12 * s34 * tmp31 * tmp45 -
             160 * s23 * s34 * tmp31 * tmp45 - 16 * s24 * s34 * tmp31 * tmp45 +
             72 * tmp14 * tmp31 * tmp45 - 64 * tmp202 * tmp31 * tmp45 -
             16 * tmp25 * tmp31 * tmp45 - 12 * m12 * m22 * m32 * s12 * tmp451 -
             28 * m12 * m22 * m32 * s13 * tmp451 +
             16 * m12 * m32 * s12 * s13 * tmp451 -
             12 * m12 * m22 * m32 * s14 * tmp451 +
             4 * m12 * m32 * s12 * s14 * tmp451 -
             12 * m22 * m32 * s12 * s14 * tmp451 +
             4 * m12 * m32 * s13 * s14 * tmp451 +
             4 * m22 * m32 * s13 * s14 * tmp451 +
             32 * m32 * s12 * s13 * s14 * tmp451 +
             36 * m12 * m22 * m32 * s23 * tmp451 -
             48 * m12 * m32 * s12 * s23 * tmp451 -
             24 * m12 * m32 * s13 * s23 * tmp451 +
             24 * m22 * m32 * s13 * s23 * tmp451 -
             40 * m32 * s12 * s13 * s23 * tmp451 -
             12 * m12 * m32 * s14 * s23 * tmp451 +
             36 * m22 * m32 * s14 * s23 * tmp451 -
             56 * m32 * s12 * s14 * s23 * tmp451 -
             12 * m12 * s13 * s14 * s23 * tmp451 -
             12 * m22 * s13 * s14 * s23 * tmp451 -
             104 * m32 * s13 * s14 * s23 * tmp451 +
             16 * s12 * s13 * s14 * s23 * tmp451 +
             96 * m12 * m22 * m32 * s34 * tmp451 -
             6 * m12 * m22 * s12 * s34 * tmp451 -
             90 * m12 * m32 * s12 * s34 * tmp451 +
             6 * m22 * m32 * s12 * s34 * tmp451 -
             14 * m12 * m22 * s13 * s34 * tmp451 -
             88 * m12 * m32 * s13 * s34 * tmp451 -
             24 * m22 * m32 * s13 * s34 * tmp451 +
             8 * m12 * s12 * s13 * s34 * tmp451 -
             52 * m32 * s12 * s13 * s34 * tmp451 -
             6 * m12 * m22 * s14 * s34 * tmp451 -
             32 * m12 * m32 * s14 * s34 * tmp451 +
             32 * m22 * m32 * s14 * s34 * tmp451 +
             2 * m12 * s12 * s14 * s34 * tmp451 -
             6 * m22 * s12 * s14 * s34 * tmp451 -
             116 * m32 * s12 * s14 * s34 * tmp451 +
             6 * m12 * s13 * s14 * s34 * tmp451 +
             6 * m22 * s13 * s14 * s34 * tmp451 -
             64 * m32 * s13 * s14 * s34 * tmp451 +
             16 * s12 * s13 * s14 * s34 * tmp451 +
             18 * m12 * m22 * s23 * s34 * tmp451 +
             192 * m12 * m32 * s23 * s34 * tmp451 -
             24 * m12 * s12 * s23 * s34 * tmp451 +
             28 * m32 * s12 * s23 * s34 * tmp451 -
             21 * m12 * s13 * s23 * s34 * tmp451 +
             3 * m22 * s13 * s23 * s34 * tmp451 +
             60 * m32 * s13 * s23 * s34 * tmp451 -
             14 * s12 * s13 * s23 * s34 * tmp451 -
             15 * m12 * s14 * s23 * s34 * tmp451 +
             9 * m22 * s14 * s23 * s34 * tmp451 +
             108 * m32 * s14 * s23 * s34 * tmp451 -
             22 * s12 * s14 * s23 * s34 * tmp451 -
             40 * s13 * s14 * s23 * s34 * tmp451 +
             22 * m12 * m22 * tmp1041 * tmp451 +
             120 * m12 * m32 * tmp1041 * tmp451 -
             21 * m12 * s12 * tmp1041 * tmp451 +
             3 * m22 * s12 * tmp1041 * tmp451 +
             56 * m32 * s12 * tmp1041 * tmp451 -
             26 * m12 * s13 * tmp1041 * tmp451 -
             10 * m22 * s13 * tmp1041 * tmp451 -
             8 * m32 * s13 * tmp1041 * tmp451 -
             10 * s12 * s13 * tmp1041 * tmp451 -
             12 * m12 * s14 * tmp1041 * tmp451 +
             4 * m22 * s14 * tmp1041 * tmp451 +
             64 * m32 * s14 * tmp1041 * tmp451 -
             26 * s12 * s14 * tmp1041 * tmp451 -
             8 * s13 * s14 * tmp1041 * tmp451 +
             48 * m12 * s23 * tmp1041 * tmp451 +
             10 * s12 * s23 * tmp1041 * tmp451 +
             6 * s13 * s23 * tmp1041 * tmp451 +
             18 * s14 * s23 * tmp1041 * tmp451 +
             16 * m22 * m32 * tmp11 * tmp451 - 12 * m32 * s12 * tmp11 * tmp451 -
             12 * m32 * s13 * tmp11 * tmp451 - 12 * m32 * s14 * tmp11 * tmp451 +
             36 * m32 * s23 * tmp11 * tmp451 + 8 * m22 * s34 * tmp11 * tmp451 +
             72 * m32 * s34 * tmp11 * tmp451 - 6 * s12 * s34 * tmp11 * tmp451 -
             6 * s13 * s34 * tmp11 * tmp451 - 6 * s14 * s34 * tmp11 * tmp451 +
             18 * s23 * s34 * tmp11 * tmp451 + 18 * tmp1041 * tmp11 * tmp451 +
             8 * m32 * tmp110 * tmp451 + 2 * s34 * tmp110 * tmp451 +
             160 * m12 * tmp12 * tmp451 + 64 * s12 * tmp12 * tmp451 +
             32 * s13 * tmp12 * tmp451 + 128 * s14 * tmp12 * tmp451 +
             20 * m12 * tmp1278 * tmp451 + 10 * s12 * tmp1278 * tmp451 -
             4 * s13 * tmp1278 * tmp451 + 8 * s14 * tmp1278 * tmp451 +
             48 * m12 * m32 * tmp202 * tmp451 +
             40 * m32 * s13 * tmp202 * tmp451 +
             40 * m32 * s14 * tmp202 * tmp451 -
             16 * s13 * s14 * tmp202 * tmp451 +
             24 * m12 * s34 * tmp202 * tmp451 +
             8 * s13 * s34 * tmp202 * tmp451 + 8 * s14 * s34 * tmp202 * tmp451 +
             8 * m12 * m32 * tmp25 * tmp451 + 8 * m32 * s13 * tmp25 * tmp451 +
             16 * m32 * s14 * tmp25 * tmp451 + 4 * m12 * s34 * tmp25 * tmp451 -
             20 * m32 * s34 * tmp25 * tmp451 + 4 * s13 * s34 * tmp25 * tmp451 +
             8 * s14 * s34 * tmp25 * tmp451 - 6 * tmp1041 * tmp25 * tmp451 +
             104 * m12 * m22 * tmp31 * tmp451 -
             96 * m12 * s12 * tmp31 * tmp451 - 72 * m12 * s13 * tmp31 * tmp451 -
             8 * m22 * s13 * tmp31 * tmp451 - 64 * s12 * s13 * tmp31 * tmp451 -
             16 * m12 * s14 * tmp31 * tmp451 + 48 * m22 * s14 * tmp31 * tmp451 -
             128 * s12 * s14 * tmp31 * tmp451 -
             96 * s13 * s14 * tmp31 * tmp451 +
             192 * m12 * s23 * tmp31 * tmp451 +
             16 * s12 * s23 * tmp31 * tmp451 + 96 * s13 * s23 * tmp31 * tmp451 +
             144 * s14 * s23 * tmp31 * tmp451 +
             240 * m12 * s34 * tmp31 * tmp451 +
             104 * s12 * s34 * tmp31 * tmp451 +
             16 * s13 * s34 * tmp31 * tmp451 +
             160 * s14 * s34 * tmp31 * tmp451 + 72 * tmp11 * tmp31 * tmp451 -
             16 * tmp25 * tmp31 * tmp451 + 8 * m12 * m22 * tmp45 * tmp451 +
             40 * m12 * m32 * tmp45 * tmp451 + 40 * m22 * m32 * tmp45 * tmp451 -
             6 * m12 * s12 * tmp45 * tmp451 - 6 * m22 * s12 * tmp45 * tmp451 -
             32 * m32 * s12 * tmp45 * tmp451 - 6 * m12 * s14 * tmp45 * tmp451 -
             6 * m22 * s14 * tmp45 * tmp451 - 16 * m32 * s14 * tmp45 * tmp451 +
             8 * s12 * s14 * tmp45 * tmp451 + 6 * m12 * s23 * tmp45 * tmp451 +
             6 * m22 * s23 * tmp45 * tmp451 + 16 * m32 * s23 * tmp45 * tmp451 -
             8 * s12 * s23 * tmp45 * tmp451 + 8 * s14 * s23 * tmp45 * tmp451 +
             22 * m12 * s34 * tmp45 * tmp451 + 22 * m22 * s34 * tmp45 * tmp451 +
             64 * m32 * s34 * tmp45 * tmp451 - 16 * s12 * s34 * tmp45 * tmp451 -
             14 * s14 * s34 * tmp45 * tmp451 + 14 * s23 * s34 * tmp45 * tmp451 +
             20 * tmp1041 * tmp45 * tmp451 + 4 * tmp202 * tmp45 * tmp451 +
             4 * tmp25 * tmp45 * tmp451 + 48 * tmp31 * tmp45 * tmp451 +
             12 * m12 * m22 * m32 * tmp537 - 16 * m12 * m32 * s12 * tmp537 -
             16 * m12 * m32 * s13 * tmp537 - 4 * m12 * m32 * s14 * tmp537 +
             12 * m22 * m32 * s14 * tmp537 - 16 * m32 * s12 * s14 * tmp537 -
             8 * m32 * s13 * s14 * tmp537 + 32 * m12 * m32 * s23 * tmp537 +
             16 * m32 * s13 * s23 * tmp537 + 32 * m32 * s14 * s23 * tmp537 -
             8 * s13 * s14 * s23 * tmp537 + 6 * m12 * m22 * s34 * tmp537 +
             64 * m12 * m32 * s34 * tmp537 - 8 * m12 * s12 * s34 * tmp537 +
             4 * m32 * s12 * s34 * tmp537 - 11 * m12 * s13 * s34 * tmp537 -
             3 * m22 * s13 * s34 * tmp537 - 12 * m32 * s13 * s34 * tmp537 +
             2 * s12 * s13 * s34 * tmp537 - 5 * m12 * s14 * s34 * tmp537 +
             3 * m22 * s14 * s34 * tmp537 + 36 * m32 * s14 * s34 * tmp537 -
             6 * s12 * s14 * s34 * tmp537 + 16 * m12 * s23 * s34 * tmp537 +
             8 * s14 * s23 * s34 * tmp537 + 16 * m12 * tmp1041 * tmp537 +
             2 * s12 * tmp1041 * tmp537 - 6 * s13 * tmp1041 * tmp537 +
             6 * s14 * tmp1041 * tmp537 + 12 * m32 * tmp11 * tmp537 +
             6 * s34 * tmp11 * tmp537 + 64 * m12 * tmp31 * tmp537 +
             48 * s14 * tmp31 * tmp537 + 6 * m12 * tmp45 * tmp537 +
             6 * m22 * tmp45 * tmp537 + 32 * m32 * tmp45 * tmp537 -
             8 * s12 * tmp45 * tmp537 - 8 * s14 * tmp45 * tmp537 +
             8 * s23 * tmp45 * tmp537 + 18 * s34 * tmp45 * tmp537 -
             12 * m12 * m22 * m32 * tmp56 + 16 * m22 * m32 * s12 * tmp56 +
             32 * m22 * m32 * s14 * tmp56 + 12 * m12 * m32 * s23 * tmp56 -
             4 * m22 * m32 * s23 * tmp56 - 16 * m32 * s12 * s23 * tmp56 -
             32 * m32 * s14 * s23 * tmp56 - 16 * m22 * m32 * s24 * tmp56 -
             16 * m32 * s14 * s24 * tmp56 + 8 * m32 * s23 * s24 * tmp56 -
             8 * s14 * s23 * s24 * tmp56 - 6 * m12 * m22 * s34 * tmp56 -
             64 * m22 * m32 * s34 * tmp56 + 8 * m22 * s12 * s34 * tmp56 -
             4 * m32 * s12 * s34 * tmp56 + 16 * m22 * s14 * s34 * tmp56 +
             3 * m12 * s23 * s34 * tmp56 - 5 * m22 * s23 * s34 * tmp56 +
             36 * m32 * s23 * s34 * tmp56 - 6 * s12 * s23 * s34 * tmp56 -
             8 * s14 * s23 * s34 * tmp56 - 3 * m12 * s24 * s34 * tmp56 -
             11 * m22 * s24 * s34 * tmp56 - 12 * m32 * s24 * s34 * tmp56 +
             2 * s12 * s24 * s34 * tmp56 - 16 * m22 * tmp1041 * tmp56 -
             2 * s12 * tmp1041 * tmp56 + 6 * s23 * tmp1041 * tmp56 -
             6 * s24 * tmp1041 * tmp56 - 12 * m32 * tmp14 * tmp56 -
             6 * s34 * tmp14 * tmp56 + 8 * m32 * tmp202 * tmp56 +
             2 * s34 * tmp202 * tmp56 - 64 * m22 * tmp31 * tmp56 +
             48 * s23 * tmp31 * tmp56 - 6 * m12 * tmp451 * tmp56 -
             6 * m22 * tmp451 * tmp56 - 32 * m32 * tmp451 * tmp56 +
             8 * s12 * tmp451 * tmp56 + 8 * s14 * tmp451 * tmp56 -
             8 * s23 * tmp451 * tmp56 - 18 * s34 * tmp451 * tmp56 -
             8 * tmp537 * tmp56 + 8 * m12 * m32 * tmp560 +
             8 * m32 * s14 * tmp560 + 4 * m12 * s34 * tmp560 -
             2 * s13 * s34 * tmp560 + 2 * s14 * s34 * tmp560 +
             4 * tmp45 * tmp560 + 8 * m22 * m32 * tmp61 -
             8 * m32 * s23 * tmp61 + 4 * m22 * s34 * tmp61 -
             2 * s23 * s34 * tmp61 + 2 * s24 * s34 * tmp61 +
             4 * tmp451 * tmp61 - 12 * m12 * m22 * m32 * s12 * tmp93 -
             36 * m12 * m22 * m32 * s13 * tmp93 +
             48 * m22 * m32 * s12 * s13 * tmp93 +
             28 * m12 * m22 * m32 * s23 * tmp93 -
             16 * m22 * m32 * s12 * s23 * tmp93 +
             24 * m12 * m32 * s13 * s23 * tmp93 -
             24 * m22 * m32 * s13 * s23 * tmp93 -
             40 * m32 * s12 * s13 * s23 * tmp93 +
             12 * m12 * m22 * m32 * s24 * tmp93 +
             12 * m12 * m32 * s12 * s24 * tmp93 -
             4 * m22 * m32 * s12 * s24 * tmp93 +
             36 * m12 * m32 * s13 * s24 * tmp93 -
             12 * m22 * m32 * s13 * s24 * tmp93 -
             56 * m32 * s12 * s13 * s24 * tmp93 +
             4 * m12 * m32 * s23 * s24 * tmp93 +
             4 * m22 * m32 * s23 * s24 * tmp93 +
             32 * m32 * s12 * s23 * s24 * tmp93 +
             12 * m12 * s13 * s23 * s24 * tmp93 +
             12 * m22 * s13 * s23 * s24 * tmp93 +
             104 * m32 * s13 * s23 * s24 * tmp93 -
             16 * s12 * s13 * s23 * s24 * tmp93 +
             96 * m12 * m22 * m32 * s34 * tmp93 -
             6 * m12 * m22 * s12 * s34 * tmp93 +
             6 * m12 * m32 * s12 * s34 * tmp93 -
             90 * m22 * m32 * s12 * s34 * tmp93 -
             18 * m12 * m22 * s13 * s34 * tmp93 -
             192 * m22 * m32 * s13 * s34 * tmp93 +
             24 * m22 * s12 * s13 * s34 * tmp93 -
             28 * m32 * s12 * s13 * s34 * tmp93 +
             14 * m12 * m22 * s23 * s34 * tmp93 +
             24 * m12 * m32 * s23 * s34 * tmp93 +
             88 * m22 * m32 * s23 * s34 * tmp93 -
             8 * m22 * s12 * s23 * s34 * tmp93 +
             52 * m32 * s12 * s23 * s34 * tmp93 +
             3 * m12 * s13 * s23 * s34 * tmp93 -
             21 * m22 * s13 * s23 * s34 * tmp93 +
             60 * m32 * s13 * s23 * s34 * tmp93 -
             14 * s12 * s13 * s23 * s34 * tmp93 +
             6 * m12 * m22 * s24 * s34 * tmp93 -
             32 * m12 * m32 * s24 * s34 * tmp93 +
             32 * m22 * m32 * s24 * s34 * tmp93 +
             6 * m12 * s12 * s24 * s34 * tmp93 -
             2 * m22 * s12 * s24 * s34 * tmp93 +
             116 * m32 * s12 * s24 * s34 * tmp93 +
             9 * m12 * s13 * s24 * s34 * tmp93 -
             15 * m22 * s13 * s24 * s34 * tmp93 +
             108 * m32 * s13 * s24 * s34 * tmp93 -
             22 * s12 * s13 * s24 * s34 * tmp93 +
             6 * m12 * s23 * s24 * s34 * tmp93 +
             6 * m22 * s23 * s24 * s34 * tmp93 -
             64 * m32 * s23 * s24 * s34 * tmp93 +
             16 * s12 * s23 * s24 * s34 * tmp93 +
             40 * s13 * s23 * s24 * s34 * tmp93 +
             22 * m12 * m22 * tmp1041 * tmp93 +
             120 * m22 * m32 * tmp1041 * tmp93 +
             3 * m12 * s12 * tmp1041 * tmp93 -
             21 * m22 * s12 * tmp1041 * tmp93 +
             56 * m32 * s12 * tmp1041 * tmp93 -
             48 * m22 * s13 * tmp1041 * tmp93 -
             10 * s12 * s13 * tmp1041 * tmp93 +
             10 * m12 * s23 * tmp1041 * tmp93 +
             26 * m22 * s23 * tmp1041 * tmp93 +
             8 * m32 * s23 * tmp1041 * tmp93 +
             10 * s12 * s23 * tmp1041 * tmp93 +
             6 * s13 * s23 * tmp1041 * tmp93 - 4 * m12 * s24 * tmp1041 * tmp93 +
             12 * m22 * s24 * tmp1041 * tmp93 -
             64 * m32 * s24 * tmp1041 * tmp93 +
             26 * s12 * s24 * tmp1041 * tmp93 +
             18 * s13 * s24 * tmp1041 * tmp93 -
             8 * s23 * s24 * tmp1041 * tmp93 + 160 * m22 * tmp12 * tmp93 +
             64 * s12 * tmp12 * tmp93 - 32 * s23 * tmp12 * tmp93 -
             128 * s24 * tmp12 * tmp93 + 20 * m22 * tmp1278 * tmp93 +
             10 * s12 * tmp1278 * tmp93 + 4 * s23 * tmp1278 * tmp93 -
             8 * s24 * tmp1278 * tmp93 + 16 * m12 * m32 * tmp14 * tmp93 -
             12 * m32 * s12 * tmp14 * tmp93 - 36 * m32 * s13 * tmp14 * tmp93 +
             12 * m32 * s23 * tmp14 * tmp93 + 12 * m32 * s24 * tmp14 * tmp93 +
             8 * m12 * s34 * tmp14 * tmp93 + 72 * m32 * s34 * tmp14 * tmp93 -
             6 * s12 * s34 * tmp14 * tmp93 - 18 * s13 * s34 * tmp14 * tmp93 +
             6 * s23 * s34 * tmp14 * tmp93 + 6 * s24 * s34 * tmp14 * tmp93 +
             18 * tmp1041 * tmp14 * tmp93 + 8 * m12 * m22 * tmp202 * tmp93 +
             40 * m12 * m32 * tmp202 * tmp93 + 40 * m22 * m32 * tmp202 * tmp93 -
             6 * m12 * s12 * tmp202 * tmp93 - 6 * m22 * s12 * tmp202 * tmp93 -
             32 * m32 * s12 * tmp202 * tmp93 - 6 * m12 * s13 * tmp202 * tmp93 -
             6 * m22 * s13 * tmp202 * tmp93 - 16 * m32 * s13 * tmp202 * tmp93 +
             8 * s12 * s13 * tmp202 * tmp93 + 6 * m12 * s24 * tmp202 * tmp93 +
             6 * m22 * s24 * tmp202 * tmp93 + 16 * m32 * s24 * tmp202 * tmp93 -
             8 * s12 * s24 * tmp202 * tmp93 + 8 * s13 * s24 * tmp202 * tmp93 +
             22 * m12 * s34 * tmp202 * tmp93 + 22 * m22 * s34 * tmp202 * tmp93 +
             64 * m32 * s34 * tmp202 * tmp93 - 16 * s12 * s34 * tmp202 * tmp93 -
             14 * s13 * s34 * tmp202 * tmp93 + 14 * s24 * s34 * tmp202 * tmp93 +
             20 * tmp1041 * tmp202 * tmp93 + 8 * m22 * m32 * tmp25 * tmp93 -
             8 * m32 * s23 * tmp25 * tmp93 - 16 * m32 * s24 * tmp25 * tmp93 +
             4 * m22 * s34 * tmp25 * tmp93 - 20 * m32 * s34 * tmp25 * tmp93 -
             4 * s23 * s34 * tmp25 * tmp93 - 8 * s24 * s34 * tmp25 * tmp93 -
             6 * tmp1041 * tmp25 * tmp93 + 4 * tmp202 * tmp25 * tmp93 +
             6 * m12 * tmp257 * tmp93 + 6 * m22 * tmp257 * tmp93 +
             32 * m32 * tmp257 * tmp93 - 8 * s12 * tmp257 * tmp93 -
             8 * s13 * tmp257 * tmp93 + 8 * s24 * tmp257 * tmp93 +
             18 * s34 * tmp257 * tmp93 + 4 * tmp275 * tmp93 +
             104 * m12 * m22 * tmp31 * tmp93 - 96 * m22 * s12 * tmp31 * tmp93 -
             192 * m22 * s13 * tmp31 * tmp93 - 16 * s12 * s13 * tmp31 * tmp93 +
             8 * m12 * s23 * tmp31 * tmp93 + 72 * m22 * s23 * tmp31 * tmp93 +
             64 * s12 * s23 * tmp31 * tmp93 + 96 * s13 * s23 * tmp31 * tmp93 -
             48 * m12 * s24 * tmp31 * tmp93 + 16 * m22 * s24 * tmp31 * tmp93 +
             128 * s12 * s24 * tmp31 * tmp93 + 144 * s13 * s24 * tmp31 * tmp93 -
             96 * s23 * s24 * tmp31 * tmp93 + 240 * m22 * s34 * tmp31 * tmp93 +
             104 * s12 * s34 * tmp31 * tmp93 - 16 * s23 * s34 * tmp31 * tmp93 -
             160 * s24 * s34 * tmp31 * tmp93 + 72 * tmp14 * tmp31 * tmp93 +
             48 * tmp202 * tmp31 * tmp93 - 16 * tmp25 * tmp31 * tmp93 +
             48 * m22 * m32 * tmp45 * tmp93 - 40 * m32 * s23 * tmp45 * tmp93 -
             40 * m32 * s24 * tmp45 * tmp93 - 16 * s23 * s24 * tmp45 * tmp93 +
             24 * m22 * s34 * tmp45 * tmp93 - 8 * s23 * s34 * tmp45 * tmp93 -
             8 * s24 * s34 * tmp45 * tmp93 + 4 * tmp202 * tmp45 * tmp93 +
             16 * m32 * s12 * tmp451 * tmp93 + 24 * m32 * s13 * tmp451 * tmp93 -
             24 * m32 * s23 * tmp451 * tmp93 + 16 * s13 * s23 * tmp451 * tmp93 +
             2 * m12 * s34 * tmp451 * tmp93 + 2 * m22 * s34 * tmp451 * tmp93 -
             48 * m32 * s34 * tmp451 * tmp93 + 8 * s12 * s34 * tmp451 * tmp93 +
             6 * s13 * s34 * tmp451 * tmp93 - 6 * s23 * s34 * tmp451 * tmp93 -
             8 * tmp1041 * tmp451 * tmp93 + 4 * tmp202 * tmp451 * tmp93 -
             64 * tmp31 * tmp451 * tmp93 + 4 * tmp45 * tmp451 * tmp93 -
             8 * m32 * tmp537 * tmp93 - 2 * s34 * tmp537 * tmp93)) /
           (3. * pow(s34 + tmp1, 2) * pow(-s13 - s14 + s34 + tmp1, 2) *
            pow(s23 + s24 + s34 + tmp1, 2));
}

MUSTARD_OPTIMIZE_FAST auto MSqM2ENNEE::TwoBornPol(double s12, double s13, double s14, double s23, double s24, double s34,
                                                  double m12, double m22, double m32,
                                                  double s2n, double s3n, double s4n) -> double {
    using muc::pow;

    // Adapt from McMule v0.5.1, mudecrare/mudecrare_1l_twotrace.opt.f95, FUNCTION bornPol
    //
    // Copyright (C) 2020-2025  Yannick Ulrich and others (The McMule development team)
    //

    const auto tmp7{2 * m32};
    const auto tmp8{s34 + tmp7};
    const auto tmp10{s23 + s24 + s34 + tmp7};
    const auto tmp39{pow(s13, 2)};
    const auto tmp44{pow(s23, 2)};
    const auto tmp40{pow(s24, 2)};
    const auto tmp59{pow(s34, 2)};
    const auto tmp68{pow(s34, 3)};
    const auto tmp57{pow(s24, 3)};
    const auto tmp47{pow(s23, 3)};
    const auto tmp29{s2n + s3n + s4n};
    const auto tmp131{pow(s14, 2)};
    const auto tmp177{2 * s2n};
    const auto tmp178{s3n + s4n + tmp177};
    const auto tmp204{-(s3n * tmp40)};
    const auto tmp207{-(s4n * tmp44)};
    const auto tmp269{2 * s34};
    const auto tmp270{s23 + s24 + tmp269};
    const auto tmp135{-(s2n * s34)};
    const auto tmp31{s3n + s4n};
    const auto tmp32{2 * tmp31};
    const auto tmp33{s2n + tmp32};
    const auto tmp132{s2n + s4n};
    const auto tmp20{s2n * s34};
    const auto tmp292{2 * s4n};
    const auto tmp16{-s13};
    const auto tmp17{-s14};
    const auto tmp37{pow(m32, 4)};
    const auto tmp41{-2 * s2n * tmp39 * tmp40};
    const auto tmp42{2 * s12 * s13 * s23 * s2n * s34};
    const auto tmp43{s23 * s2n * s34 * tmp39};
    const auto tmp49{2 * s12 * s13 * s24 * s2n * s34};
    const auto tmp50{-(s24 * s2n * s34 * tmp39)};
    const auto tmp60{2 * s12 * s13 * s2n * tmp59};
    const auto tmp72{-2 * s3n * tmp39 * tmp40};
    const auto tmp73{2 * s13 * s23 * s3n * tmp40};
    const auto tmp74{2 * s13 * s3n * tmp57};
    const auto tmp75{s12 * s13 * s23 * s34 * s3n};
    const auto tmp76{s23 * s34 * s3n * tmp39};
    const auto tmp77{-(s12 * s34 * s3n * tmp44)};
    const auto tmp79{s34 * s3n * tmp47};
    const auto tmp80{s12 * s13 * s24 * s34 * s3n};
    const auto tmp81{-(s24 * s34 * s3n * tmp39)};
    const auto tmp82{-2 * s12 * s23 * s24 * s34 * s3n};
    const auto tmp84{3 * s24 * s34 * s3n * tmp44};
    const auto tmp85{-(s12 * s34 * s3n * tmp40)};
    const auto tmp86{6 * s13 * s34 * s3n * tmp40};
    const auto tmp87{s23 * s34 * s3n * tmp40};
    const auto tmp88{-(s34 * s3n * tmp57)};
    const auto tmp89{s12 * s13 * s3n * tmp59};
    const auto tmp90{-3 * s12 * s23 * s3n * tmp59};
    const auto tmp91{-3 * s12 * s24 * s3n * tmp59};
    const auto tmp92{2 * s13 * s24 * s3n * tmp59};
    const auto tmp94{-2 * s3n * tmp40 * tmp59};
    const auto tmp95{-2 * s12 * s3n * tmp68};
    const auto tmp108{2 * s23 * s24 * s4n * tmp39};
    const auto tmp109{-2 * s13 * s24 * s4n * tmp44};
    const auto tmp110{-2 * s13 * s23 * s4n * tmp40};
    const auto tmp111{s12 * s13 * s23 * s34 * s4n};
    const auto tmp112{s23 * s34 * s4n * tmp39};
    const auto tmp113{-(s12 * s34 * s4n * tmp44)};
    const auto tmp114{-(s34 * s4n * tmp47)};
    const auto tmp115{s12 * s13 * s24 * s34 * s4n};
    const auto tmp116{s24 * s34 * s4n * tmp39};
    const auto tmp117{-2 * s12 * s23 * s24 * s34 * s4n};
    const auto tmp119{s24 * s34 * s4n * tmp44};
    const auto tmp120{-(s12 * s34 * s4n * tmp40)};
    const auto tmp122{3 * s23 * s34 * s4n * tmp40};
    const auto tmp123{s34 * s4n * tmp57};
    const auto tmp124{s12 * s13 * s4n * tmp59};
    const auto tmp125{-3 * s12 * s23 * s4n * tmp59};
    const auto tmp126{-2 * s13 * s23 * s4n * tmp59};
    const auto tmp127{-2 * s4n * tmp44 * tmp59};
    const auto tmp128{-3 * s12 * s24 * s4n * tmp59};
    const auto tmp130{-2 * s12 * s4n * tmp68};
    const auto tmp96{pow(m32, 3)};
    const auto tmp143{pow(m32, 2)};
    const auto tmp144{-(s2n * tmp39)};
    const auto tmp145{-(s2n * tmp131)};
    const auto tmp256{12 * s23 * s2n * s34};
    const auto tmp257{12 * s24 * s2n * s34};
    const auto tmp154{12 * s2n * tmp59};
    const auto tmp283{4 * s24 * s34 * s3n};
    const auto tmp288{4 * s4n * tmp40};
    const auto tmp170{6 * s2n * s34};
    const auto tmp382{-2 * s23 * s2n};
    const auto tmp383{-2 * s24 * s2n};
    const auto tmp171{-3 * s23 * s3n};
    const auto tmp172{-3 * s24 * s3n};
    const auto tmp173{-6 * s34 * s3n};
    const auto tmp174{-3 * s23 * s4n};
    const auto tmp175{-3 * s24 * s4n};
    const auto tmp176{-6 * s34 * s4n};
    const auto tmp179{s13 * tmp178};
    const auto tmp180{s14 * tmp178};
    const auto tmp215{-(s23 * s2n * tmp131)};
    const auto tmp218{s24 * s2n * tmp131};
    const auto tmp224{-(s2n * s34 * tmp131)};
    const auto tmp233{2 * s23 * s3n * tmp131};
    const auto tmp235{2 * s3n * tmp47};
    const auto tmp236{2 * s24 * s3n * tmp131};
    const auto tmp237{6 * s24 * s3n * tmp44};
    const auto tmp239{4 * s23 * s3n * tmp40};
    const auto tmp244{2 * s24 * s4n * tmp131};
    const auto tmp246{4 * s24 * s4n * tmp44};
    const auto tmp248{6 * s23 * s4n * tmp40};
    const auto tmp249{2 * s4n * tmp57};
    const auto tmp200{2 * s23 * s4n * tmp59};
    const auto tmp259{-(s3n * tmp44)};
    const auto tmp260{-2 * s23 * s24 * s3n};
    const auto tmp261{-6 * s23 * s34 * s3n};
    const auto tmp262{-6 * s24 * s34 * s3n};
    const auto tmp263{-6 * s3n * tmp59};
    const auto tmp264{-2 * s23 * s24 * s4n};
    const auto tmp265{-(s4n * tmp40)};
    const auto tmp266{-6 * s23 * s34 * s4n};
    const auto tmp267{-6 * s24 * s34 * s4n};
    const auto tmp268{-6 * s4n * tmp59};
    const auto tmp271{s13 * tmp178 * tmp270};
    const auto tmp272{s14 * tmp178 * tmp270};
    const auto tmp275{-(s24 * s2n)};
    const auto tmp276{2 * s24 * s4n};
    const auto tmp277{s23 * tmp33};
    const auto tmp278{tmp135 + tmp275 + tmp276 + tmp277};
    const auto tmp279{tmp278 * tmp39};
    const auto tmp445{-6 * s2n * tmp59};
    const auto tmp295{2 * s3n};
    const auto tmp296{s4n + tmp177 + tmp295};
    const auto tmp297{s23 * tmp296};
    const auto tmp298{s3n + tmp177 + tmp292};
    const auto tmp299{s24 * tmp298};
    const auto tmp300{tmp20 + tmp297 + tmp299};
    const auto tmp301{2 * s14 * tmp300};
    const auto tmp46{3 * s13 * s2n * s34 * tmp44};
    const auto tmp320{s24 * s2n * s34 * tmp131};
    const auto tmp325{2 * s12 * s14 * s2n * tmp59};
    const auto tmp61{6 * s12 * s23 * s2n * tmp59};
    const auto tmp64{6 * s12 * s24 * s2n * tmp59};
    const auto tmp69{2 * s12 * s2n * tmp68};
    const auto tmp70{-6 * s23 * s2n * tmp68};
    const auto tmp71{-6 * s24 * s2n * tmp68};
    const auto tmp469{pow(s13, 3)};
    const auto tmp475{pow(s14, 3)};
    const auto tmp348{s12 * s14 * s23 * s34 * s3n};
    const auto tmp351{s12 * s14 * s24 * s34 * s3n};
    const auto tmp341{pow(s34, 4)};
    const auto tmp366{s12 * s14 * s23 * s34 * s4n};
    const auto tmp369{s12 * s14 * s24 * s34 * s4n};
    const auto tmp411{-6 * s2n * s34};
    const auto tmp164{-2 * s24 * s3n};
    const auto tmp168{6 * s23 * s2n};
    const auto tmp169{6 * s24 * s2n};
    const auto tmp163{8 * s2n * s34};
    const auto tmp22{s23 * s3n};
    const auto tmp23{s24 * s3n};
    const auto tmp26{s23 * s4n};
    const auto tmp27{s24 * s4n};
    const auto tmp152{-10 * s23 * s2n * s34};
    const auto tmp153{-10 * s24 * s2n * s34};
    const auto tmp396{-4 * s14 * s23 * s3n};
    const auto tmp401{-4 * s14 * s24 * s4n};
    const auto tmp612{s24 * s2n};
    const auto tmp290{3 * s2n};
    const auto tmp421{-12 * s14 * s24 * s2n * s34};
    const auto tmp187{3 * s2n * s34 * tmp40};
    const auto tmp238{2 * s14 * s3n * tmp40};
    const auto tmp388{-2 * s24 * s4n};
    const auto tmp628{3 * s2n * tmp40};
    const auto tmp707{-4 * s3n};
    const auto tmp102{-s3n};
    const auto tmp797{-2 * s2n};
    const auto tmp384{-4 * s2n * s34};
    const auto tmp386{-(s24 * s3n)};
    const auto tmp791{6 * s2n};
    const auto tmp103{-s4n};
    const auto tmp830{tmp102 + tmp103 + tmp791};
    const auto tmp831{s23 * tmp830};
    return (128 *
            (-2 * pow(m12, 2) * s2n * pow(tmp10, 2) * tmp8 -
             6 * pow(m22, 2) *
                 (-(s14 * s2n) - s14 * s3n + 2 * s34 * s3n - s14 * s4n +
                  2 * s34 * s4n + tmp20 + tmp22 + tmp23 + tmp26 + tmp27 -
                  s13 * tmp29 + 2 * m32 * tmp33) *
                 (s34 + tmp16 + tmp17 + tmp7) * tmp8 +
             m12 *
                 (8 * s12 * s23 * s24 * s2n * s34 +
                  4 * s13 * s23 * s24 * s2n * s34 +
                  2 * s13 * s23 * s24 * s34 * s3n -
                  4 * s13 * s23 * s24 * s34 * s4n + tmp108 + tmp109 + tmp110 +
                  tmp111 + tmp112 + tmp113 + tmp114 + tmp115 + tmp116 + tmp117 +
                  tmp119 + tmp120 + tmp122 + tmp123 + tmp124 + tmp125 + tmp126 +
                  tmp127 + tmp128 + tmp130 + 64 * s2n * tmp37 +
                  4 * s12 * s2n * s34 * tmp40 + 5 * s13 * s2n * s34 * tmp40 -
                  12 * s23 * s2n * s34 * tmp40 + 2 * s13 * s34 * s4n * tmp40 +
                  tmp41 + tmp42 + tmp43 + 4 * s12 * s2n * s34 * tmp44 -
                  12 * s24 * s2n * s34 * tmp44 + 2 * s13 * s34 * s3n * tmp44 +
                  4 * tmp143 *
                      (-3 * s14 * s23 * s2n - 5 * s14 * s24 * s2n -
                       22 * s23 * s24 * s2n - 8 * s14 * s2n * s34 -
                       2 * s14 * s24 * s3n + 2 * s23 * s24 * s3n +
                       2 * s14 * s23 * s4n + 2 * s23 * s24 * s4n + tmp144 +
                       tmp145 + tmp152 + tmp153 + tmp154 -
                       s13 * (5 * s23 * s2n + 3 * s24 * s2n + 2 * s23 * s4n +
                              tmp163 + tmp164) +
                       s12 * (tmp168 + tmp169 + tmp170 + tmp171 + tmp172 +
                              tmp173 + tmp174 + tmp175 + tmp176 + tmp179 +
                              tmp180) -
                       12 * s2n * tmp40 - 2 * s3n * tmp40 - 12 * s2n * tmp44 -
                       2 * s4n * tmp44) +
                  tmp131 *
                      (s23 * (2 * s24 * s3n + s34 * s3n - s34 * s4n + tmp135) +
                       s24 * s34 * tmp29 - 2 * tmp132 * tmp44) +
                  tmp46 - 4 * s2n * s34 * tmp47 + tmp49 + tmp50 -
                  4 * s2n * s34 * tmp57 + s13 * s23 * s2n * tmp59 +
                  3 * s13 * s24 * s2n * tmp59 - 20 * s23 * s24 * s2n * tmp59 +
                  2 * s23 * s24 * s3n * tmp59 + 2 * s23 * s24 * s4n * tmp59 -
                  11 * s2n * tmp40 * tmp59 - 11 * s2n * tmp44 * tmp59 +
                  s14 * (4 * s23 * s24 * s2n * s34 - 4 * s23 * s24 * s34 * s3n +
                         2 * s23 * s24 * s34 * s4n +
                         s12 * s34 * (s23 + s24 + s34) * tmp178 + tmp187 +
                         tmp200 - 2 * s23 * s3n * tmp40 +
                         2 * s34 * s4n * tmp40 + 5 * s2n * s34 * tmp44 -
                         2 * s24 * s3n * tmp44 + 2 * s34 * s3n * tmp44 +
                         2 * s24 * s4n * tmp44 + 6 * s34 * s4n * tmp44 +
                         2 * s4n * tmp47 + 3 * s23 * s2n * tmp59 +
                         s24 * s2n * tmp59 - 2 * s24 * s3n * tmp59 +
                         2 * s13 *
                             (s23 * s34 * (s3n + tmp177) +
                              s24 * s34 * (s4n + tmp177) + s23 * s24 * tmp178 +
                              tmp204 + tmp207 + s2n * tmp59)) +
                  tmp60 + tmp61 + tmp64 +
                  2 * m32 *
                      (4 * s14 * s23 * s24 * s2n - 4 * s14 * s24 * s2n * s34 -
                       42 * s23 * s24 * s2n * s34 - 4 * s14 * s24 * s34 * s3n +
                       4 * s23 * s24 * s34 * s3n + 6 * s14 * s23 * s24 * s4n +
                       4 * s14 * s23 * s34 * s4n + 4 * s23 * s24 * s34 * s4n +
                       tmp215 + tmp218 + tmp224 + tmp233 + tmp235 + tmp236 +
                       tmp237 + tmp238 + tmp239 + tmp244 + tmp246 + tmp248 +
                       tmp249 + tmp279 + 3 * s14 * s2n * tmp40 -
                       12 * s23 * s2n * tmp40 - 23 * s2n * s34 * tmp40 -
                       4 * s34 * s3n * tmp40 + 4 * s14 * s4n * tmp40 +
                       5 * s14 * s2n * tmp44 - 12 * s24 * s2n * tmp44 -
                       23 * s2n * s34 * tmp44 + 4 * s14 * s3n * tmp44 +
                       8 * s14 * s4n * tmp44 - 4 * s34 * s4n * tmp44 -
                       4 * s2n * tmp47 - 4 * s2n * tmp57 -
                       4 * s14 * s2n * tmp59 - 14 * s23 * s2n * tmp59 -
                       14 * s24 * s2n * tmp59 +
                       s13 * (s23 * s24 * (4 * s2n + 6 * s3n) -
                              4 * s23 * s34 * tmp132 + tmp283 + tmp288 +
                              tmp301 + 5 * s2n * tmp40 + 8 * s3n * tmp40 +
                              (4 * s3n + tmp290 + tmp292) * tmp44 -
                              4 * s2n * tmp59) +
                       s12 * (8 * s23 * s24 * s2n + tmp204 + tmp207 + tmp256 +
                              tmp257 + tmp259 + tmp260 + tmp261 + tmp262 +
                              tmp263 + tmp264 + tmp265 + tmp266 + tmp267 +
                              tmp268 + tmp271 + tmp272 + 4 * s2n * tmp40 +
                              4 * s2n * tmp44 + 6 * s2n * tmp59) +
                       4 * s2n * tmp68) +
                  tmp69 + tmp70 + tmp71 + tmp72 + tmp73 + tmp74 + tmp75 +
                  tmp76 + tmp77 + tmp79 + tmp80 + tmp81 + tmp82 + tmp84 +
                  tmp85 + tmp86 + tmp87 + tmp88 + tmp89 + tmp90 + tmp91 +
                  tmp92 + tmp94 + tmp95 +
                  16 *
                      (-(s2n * (2 * s13 + 2 * s14 + s23 + s24 - 6 * s34)) +
                       s12 * (s2n + tmp102 + tmp103)) *
                      tmp96) +
             2 * (-s12 + s23 + s24 + s34 + tmp16 + tmp17 + tmp7) *
                 (4 * s13 * s14 * s23 * s24 * s2n +
                  2 * s12 * s14 * s23 * s2n * s34 +
                  4 * s13 * s14 * s23 * s2n * s34 +
                  2 * s12 * s14 * s24 * s2n * s34 +
                  4 * s13 * s14 * s24 * s2n * s34 -
                  4 * s13 * s23 * s24 * s2n * s34 -
                  4 * s14 * s23 * s24 * s2n * s34 +
                  2 * s13 * s14 * s23 * s24 * s3n +
                  2 * s13 * s14 * s23 * s34 * s3n -
                  6 * s14 * s23 * s24 * s34 * s3n +
                  2 * s13 * s14 * s23 * s24 * s4n +
                  2 * s13 * s14 * s24 * s34 * s4n -
                  6 * s13 * s23 * s24 * s34 * s4n + tmp108 + tmp109 + tmp110 +
                  tmp111 + tmp112 + tmp113 + tmp114 + tmp115 + tmp116 + tmp117 +
                  tmp119 + tmp120 + tmp122 + tmp123 + tmp124 + tmp125 + tmp126 +
                  tmp127 + tmp128 + tmp130 - s23 * s2n * s34 * tmp131 +
                  2 * s23 * s24 * s3n * tmp131 + s23 * s34 * s3n * tmp131 +
                  s24 * s34 * s3n * tmp131 - s23 * s34 * s4n * tmp131 +
                  s24 * s34 * s4n * tmp131 + tmp320 + tmp325 +
                  2 * s2n * tmp341 + tmp348 + tmp351 + tmp366 + tmp369 +
                  32 * s2n * tmp37 + s13 * s2n * s34 * tmp40 -
                  s14 * s2n * s34 * tmp40 - 2 * s13 * s14 * s3n * tmp40 -
                  2 * s14 * s23 * s3n * tmp40 + tmp41 + tmp42 + tmp43 -
                  s13 * s2n * s34 * tmp44 + s14 * s2n * s34 * tmp44 -
                  2 * s14 * s24 * s3n * tmp44 - 2 * s13 * s14 * s4n * tmp44 +
                  2 * s14 * s24 * s4n * tmp44 + 6 * s14 * s34 * s4n * tmp44 -
                  2 * s2n * tmp131 * tmp44 - 2 * s4n * tmp131 * tmp44 +
                  4 * tmp143 *
                      (-5 * s14 * s23 * s2n - 7 * s14 * s24 * s2n +
                       2 * s23 * s24 * s2n - 6 * s14 * s2n * s34 -
                       4 * s14 * s24 * s3n + 8 * s23 * s24 * s3n +
                       10 * s23 * s34 * s3n + 8 * s23 * s24 * s4n +
                       4 * s23 * s34 * s4n + 10 * s24 * s34 * s4n + tmp144 +
                       tmp145 + tmp154 + tmp256 + tmp257 + tmp283 + tmp288 -
                       s13 * (7 * s23 * s2n + 5 * s24 * s2n + 4 * s24 * s4n +
                              tmp170 + 4 * s23 * tmp31) +
                       tmp396 + tmp401 +
                       s12 * (tmp171 + tmp172 + tmp173 + tmp174 + tmp175 +
                              tmp176 + tmp179 + tmp180 + tmp382 + tmp383 +
                              tmp411) +
                       4 * s3n * tmp44) +
                  2 * s14 * s4n * tmp47 + tmp49 + tmp50 +
                  2 * s13 * s14 * s2n * tmp59 - 2 * s12 * s23 * s2n * tmp59 -
                  5 * s13 * s23 * s2n * tmp59 - 3 * s14 * s23 * s2n * tmp59 -
                  2 * s12 * s24 * s2n * tmp59 - 3 * s13 * s24 * s2n * tmp59 -
                  5 * s14 * s24 * s2n * tmp59 + 4 * s23 * s24 * s2n * tmp59 +
                  s12 * s14 * s3n * tmp59 - 2 * s13 * s23 * s3n * tmp59 -
                  2 * s14 * s23 * s3n * tmp59 - 2 * s14 * s24 * s3n * tmp59 +
                  4 * s23 * s24 * s3n * tmp59 + s12 * s14 * s4n * tmp59 +
                  2 * s14 * s23 * s4n * tmp59 - 2 * s13 * s24 * s4n * tmp59 -
                  2 * s14 * s24 * s4n * tmp59 + 4 * s23 * s24 * s4n * tmp59 +
                  s2n * tmp40 * tmp59 + 2 * s4n * tmp40 * tmp59 +
                  s2n * tmp44 * tmp59 + 2 * s3n * tmp44 * tmp59 + tmp60 -
                  2 * s12 * s2n * tmp68 - 2 * s13 * s2n * tmp68 -
                  2 * s14 * s2n * tmp68 + 4 * s23 * s2n * tmp68 +
                  4 * s24 * s2n * tmp68 + 2 * s23 * s3n * tmp68 +
                  2 * s24 * s4n * tmp68 +
                  2 * m32 *
                      (-4 * s14 * s23 * s24 * s2n - 8 * s14 * s23 * s2n * s34 +
                       6 * s23 * s24 * s2n * s34 - 6 * s14 * s23 * s24 * s3n -
                       6 * s14 * s23 * s34 * s3n - 6 * s14 * s24 * s34 * s3n +
                       12 * s23 * s24 * s34 * s3n + 2 * s14 * s23 * s34 * s4n -
                       6 * s14 * s24 * s34 * s4n + 12 * s23 * s24 * s34 * s4n +
                       tmp200 + tmp215 + tmp218 + tmp224 + tmp233 + tmp235 +
                       tmp236 + tmp237 + tmp239 + tmp244 + tmp246 + tmp248 +
                       tmp249 + tmp279 - s14 * s2n * tmp40 + s2n * s34 * tmp40 -
                       2 * s34 * s3n * tmp40 + 6 * s34 * s4n * tmp40 + tmp421 +
                       s14 * s2n * tmp44 + s2n * s34 * tmp44 +
                       6 * s34 * s3n * tmp44 + 6 * s14 * s4n * tmp44 -
                       2 * s34 * s4n * tmp44 +
                       s12 * (-4 * s23 * s2n * s34 - 4 * s24 * s2n * s34 +
                              tmp204 + tmp207 + tmp259 + tmp260 + tmp261 +
                              tmp262 + tmp263 + tmp264 + tmp265 + tmp266 +
                              tmp267 + tmp268 + tmp271 + tmp272 + tmp445) +
                       s13 *
                           (-8 * s24 * s2n * s34 + 2 * s24 * s34 * s3n -
                            2 * s23 *
                                (s24 * (3 * s4n + tmp177) + 3 * s34 * tmp178) +
                            tmp267 + tmp301 + s2n * tmp40 + 6 * s3n * tmp40 -
                            s2n * tmp44 + tmp445) -
                       6 * s14 * s2n * tmp59 + 12 * s23 * s2n * tmp59 +
                       12 * s24 * s2n * tmp59 + 8 * s23 * s3n * tmp59 +
                       2 * s24 * s3n * tmp59 + 8 * s24 * s4n * tmp59 +
                       8 * s2n * tmp68) +
                  tmp72 + tmp73 + tmp74 + tmp75 + tmp76 + tmp77 + tmp79 +
                  tmp80 + tmp81 + tmp82 + tmp84 + tmp85 + tmp86 + tmp87 +
                  tmp88 + tmp89 + tmp90 + tmp91 + tmp92 + tmp94 + tmp95 -
                  16 *
                      (s13 * s2n + s14 * s2n - 2 * s23 * s3n - s23 * s4n +
                       s12 * tmp29 + tmp382 + tmp383 + tmp384 + tmp386 +
                       tmp388) *
                      tmp96) -
             m22 *
                 (-12 * s13 * s14 * s23 * s24 * s2n -
                  8 * s12 * s13 * s14 * s2n * s34 -
                  6 * s12 * s13 * s23 * s2n * s34 -
                  6 * s12 * s14 * s23 * s2n * s34 -
                  4 * s13 * s14 * s23 * s2n * s34 -
                  6 * s12 * s13 * s24 * s2n * s34 -
                  6 * s12 * s14 * s24 * s2n * s34 -
                  4 * s13 * s14 * s24 * s2n * s34 +
                  12 * s13 * s23 * s24 * s2n * s34 +
                  12 * s14 * s23 * s24 * s2n * s34 -
                  6 * s13 * s14 * s23 * s24 * s3n -
                  8 * s12 * s13 * s14 * s34 * s3n +
                  10 * s13 * s14 * s23 * s34 * s3n +
                  16 * s13 * s14 * s24 * s34 * s3n +
                  6 * s12 * s23 * s24 * s34 * s3n -
                  6 * s13 * s23 * s24 * s34 * s3n +
                  12 * s14 * s23 * s24 * s34 * s3n -
                  6 * s13 * s14 * s23 * s24 * s4n -
                  8 * s12 * s13 * s14 * s34 * s4n +
                  16 * s13 * s14 * s23 * s34 * s4n +
                  10 * s13 * s14 * s24 * s34 * s4n +
                  6 * s12 * s23 * s24 * s34 * s4n +
                  12 * s13 * s23 * s24 * s34 * s4n -
                  6 * s14 * s23 * s24 * s34 * s4n + tmp111 + tmp115 + tmp130 -
                  4 * s12 * s2n * s34 * tmp131 - 12 * s13 * s2n * s34 * tmp131 +
                  7 * s23 * s2n * s34 * tmp131 - 6 * s23 * s24 * s3n * tmp131 -
                  4 * s12 * s34 * s3n * tmp131 - 12 * s13 * s34 * s3n * tmp131 +
                  5 * s23 * s34 * s3n * tmp131 + 5 * s24 * s34 * s3n * tmp131 -
                  4 * s12 * s34 * s4n * tmp131 - 12 * s13 * s34 * s4n * tmp131 +
                  11 * s23 * s34 * s4n * tmp131 + 5 * s24 * s34 * s4n * tmp131 +
                  tmp320 + tmp325 + 8 * s3n * tmp341 + 8 * s4n * tmp341 +
                  tmp348 + tmp351 + tmp366 + tmp369 + 64 * tmp33 * tmp37 -
                  4 * s12 * s2n * s34 * tmp39 - 12 * s14 * s2n * s34 * tmp39 +
                  7 * s24 * s2n * s34 * tmp39 - 4 * s12 * s34 * s3n * tmp39 -
                  12 * s14 * s34 * s3n * tmp39 + 5 * s23 * s34 * s3n * tmp39 +
                  11 * s24 * s34 * s3n * tmp39 - 6 * s23 * s24 * s4n * tmp39 -
                  4 * s12 * s34 * s4n * tmp39 - 12 * s14 * s34 * s4n * tmp39 +
                  5 * s23 * s34 * s4n * tmp39 + 5 * s24 * s34 * s4n * tmp39 -
                  3 * s13 * s2n * s34 * tmp40 + 3 * s14 * s2n * s34 * tmp40 +
                  6 * s13 * s14 * s3n * tmp40 - 6 * s13 * s23 * s3n * tmp40 +
                  6 * s14 * s23 * s3n * tmp40 + 3 * s12 * s34 * s3n * tmp40 -
                  22 * s13 * s34 * s3n * tmp40 - 4 * s14 * s34 * s3n * tmp40 -
                  3 * s23 * s34 * s3n * tmp40 + 6 * s13 * s23 * s4n * tmp40 +
                  3 * s12 * s34 * s4n * tmp40 - 2 * s13 * s34 * s4n * tmp40 -
                  2 * s14 * s34 * s4n * tmp40 - 9 * s23 * s34 * s4n * tmp40 +
                  6 * s2n * tmp39 * tmp40 + 6 * s3n * tmp39 * tmp40 + tmp43 -
                  3 * s14 * s2n * s34 * tmp44 + 6 * s14 * s24 * s3n * tmp44 +
                  3 * s12 * s34 * s3n * tmp44 - 2 * s13 * s34 * s3n * tmp44 -
                  2 * s14 * s34 * s3n * tmp44 - 9 * s24 * s34 * s3n * tmp44 +
                  6 * s13 * s14 * s4n * tmp44 + 6 * s13 * s24 * s4n * tmp44 -
                  6 * s14 * s24 * s4n * tmp44 + 3 * s12 * s34 * s4n * tmp44 -
                  4 * s13 * s34 * s4n * tmp44 - 22 * s14 * s34 * s4n * tmp44 -
                  3 * s24 * s34 * s4n * tmp44 + 6 * s2n * tmp131 * tmp44 +
                  6 * s4n * tmp131 * tmp44 + tmp46 - 4 * s2n * s34 * tmp469 -
                  4 * s34 * s3n * tmp469 - 4 * s34 * s4n * tmp469 -
                  3 * s34 * s3n * tmp47 - 6 * s14 * s4n * tmp47 +
                  3 * s34 * s4n * tmp47 - 4 * s2n * s34 * tmp475 -
                  4 * s34 * s3n * tmp475 - 4 * s34 * s4n * tmp475 -
                  6 * s13 * s3n * tmp57 + 3 * s34 * s3n * tmp57 -
                  3 * s34 * s4n * tmp57 + 18 * s13 * s14 * s2n * tmp59 +
                  5 * s13 * s23 * s2n * tmp59 - s14 * s23 * s2n * tmp59 -
                  s13 * s24 * s2n * tmp59 + 5 * s14 * s24 * s2n * tmp59 -
                  12 * s23 * s24 * s2n * tmp59 + 9 * s12 * s13 * s3n * tmp59 +
                  9 * s12 * s14 * s3n * tmp59 + 32 * s13 * s14 * s3n * tmp59 +
                  5 * s12 * s23 * s3n * tmp59 - 12 * s13 * s23 * s3n * tmp59 -
                  12 * s14 * s23 * s3n * tmp59 + 5 * s12 * s24 * s3n * tmp59 -
                  26 * s13 * s24 * s3n * tmp59 - 14 * s14 * s24 * s3n * tmp59 -
                  6 * s23 * s24 * s3n * tmp59 + 9 * s12 * s13 * s4n * tmp59 +
                  9 * s12 * s14 * s4n * tmp59 + 32 * s13 * s14 * s4n * tmp59 +
                  5 * s12 * s23 * s4n * tmp59 - 14 * s13 * s23 * s4n * tmp59 -
                  26 * s14 * s23 * s4n * tmp59 + 5 * s12 * s24 * s4n * tmp59 -
                  12 * s13 * s24 * s4n * tmp59 - 12 * s14 * s24 * s4n * tmp59 -
                  6 * s23 * s24 * s4n * tmp59 + 12 * s2n * tmp131 * tmp59 +
                  16 * s3n * tmp131 * tmp59 + 16 * s4n * tmp131 * tmp59 +
                  12 * s2n * tmp39 * tmp59 + 16 * s3n * tmp39 * tmp59 +
                  16 * s4n * tmp39 * tmp59 - 3 * s2n * tmp40 * tmp59 +
                  10 * s3n * tmp40 * tmp59 - 4 * s4n * tmp40 * tmp59 -
                  3 * s2n * tmp44 * tmp59 - 4 * s3n * tmp44 * tmp59 +
                  10 * s4n * tmp44 * tmp59 + tmp60 + tmp61 + tmp64 -
                  8 * s13 * s2n * tmp68 - 8 * s14 * s2n * tmp68 -
                  20 * s13 * s3n * tmp68 - 20 * s14 * s3n * tmp68 +
                  4 * s23 * s3n * tmp68 + 12 * s24 * s3n * tmp68 -
                  20 * s13 * s4n * tmp68 - 20 * s14 * s4n * tmp68 +
                  12 * s23 * s4n * tmp68 + 4 * s24 * s4n * tmp68 + tmp69 +
                  tmp70 +
                  4 * tmp143 *
                      (s14 * s23 * s2n + 7 * s14 * s24 * s2n -
                       6 * s23 * s24 * s2n - 32 * s14 * s2n * s34 -
                       6 * s14 * s24 * s3n - 22 * s23 * s24 * s3n -
                       60 * s14 * s34 * s3n - 4 * s23 * s34 * s3n +
                       20 * s24 * s34 * s3n - 18 * s14 * s23 * s4n -
                       22 * s23 * s24 * s4n - 60 * s14 * s34 * s4n +
                       20 * s23 * s34 * s4n - 4 * s24 * s34 * s4n +
                       15 * s2n * tmp131 + 16 * s3n * tmp131 +
                       16 * s4n * tmp131 + tmp152 + tmp153 + tmp154 +
                       s12 * (2 * s13 * s2n + 2 * s14 * s2n + 5 * s23 * s3n +
                              5 * s24 * s3n + 5 * s23 * s4n + 5 * s24 * s4n +
                              tmp168 + tmp169 + tmp170 + tmp173 + tmp176 +
                              9 * s13 * tmp31 + 9 * s14 * tmp31) +
                       (15 * s2n + 16 * tmp31) * tmp39 + tmp396 +
                       2 * s3n * tmp40 - 12 * s4n * tmp40 + tmp401 -
                       12 * s3n * tmp44 + 2 * s4n * tmp44 + 48 * s3n * tmp59 +
                       48 * s4n * tmp59 +
                       s13 * (-32 * s2n * s34 - 18 * s24 * s3n -
                              60 * s34 * s3n - 4 * s24 * s4n - 60 * s34 * s4n +
                              8 * s14 * (tmp290 + 4 * tmp31) + tmp612 +
                              s23 * (7 * s2n - 6 * s4n + tmp707))) +
                  tmp71 + tmp75 +
                  2 * m12 *
                      (6 * s23 * s24 * s2n - 2 * s14 * s2n * s34 +
                       6 * s23 * s2n * s34 + 6 * s24 * s2n * s34 -
                       s14 * s23 * s3n - s14 * s24 * s3n - 3 * s14 * s34 * s3n +
                       s23 * s34 * s3n + s24 * s34 * s3n - s14 * s23 * s4n -
                       s14 * s24 * s4n - 3 * s14 * s34 * s4n + s23 * s34 * s4n +
                       s24 * s34 * s4n + s2n * tmp131 + s3n * tmp131 +
                       s4n * tmp131 + 8 * tmp143 * tmp178 +
                       2 * m32 *
                           (-2 * s13 * s2n - 2 * s14 * s2n + 4 * s34 * s3n +
                            4 * s34 * s4n + tmp163 + tmp168 + tmp169 + tmp22 +
                            tmp23 + tmp26 + tmp27 - 3 * s13 * tmp31 -
                            3 * s14 * tmp31) -
                       s13 * (2 * s2n * s34 - 2 * s14 * tmp29 +
                              (s23 + s24 + 3 * s34) * tmp31) +
                       tmp29 * tmp39 + 3 * s2n * tmp44 + 4 * s2n * tmp59 +
                       2 * s3n * tmp59 + 2 * s4n * tmp59 + tmp628) *
                      tmp8 +
                  tmp80 -
                  2 * m32 *
                      (-12 * s14 * s23 * s24 * s2n +
                       18 * s23 * s24 * s2n * s34 - 16 * s14 * s23 * s24 * s3n +
                       16 * s14 * s23 * s34 * s3n + 20 * s14 * s24 * s34 * s3n +
                       28 * s23 * s24 * s34 * s3n + 2 * s14 * s23 * s24 * s4n +
                       44 * s14 * s23 * s34 * s4n + 16 * s14 * s24 * s34 * s4n +
                       28 * s23 * s24 * s34 * s4n - 7 * s23 * s2n * tmp131 -
                       s24 * s2n * tmp131 - 27 * s2n * s34 * tmp131 -
                       2 * s23 * s3n * tmp131 - 2 * s24 * s3n * tmp131 -
                       32 * s34 * s3n * tmp131 - 8 * s23 * s4n * tmp131 -
                       2 * s24 * s4n * tmp131 - 32 * s34 * s4n * tmp131 +
                       tmp187 + tmp238 +
                       (-7 * s24 * s2n - 27 * s2n * s34 - 8 * s24 * s3n -
                        32 * s34 * s3n - 32 * s34 * s4n + 12 * s14 * tmp29 -
                        s23 * tmp33 + tmp388) *
                           tmp39 -
                       3 * s14 * s2n * tmp40 + 12 * s23 * s3n * tmp40 -
                       12 * s34 * s3n * tmp40 + 18 * s23 * s4n * tmp40 +
                       16 * s34 * s4n * tmp40 + tmp421 + 3 * s14 * s2n * tmp44 +
                       3 * s2n * s34 * tmp44 + 18 * s24 * s3n * tmp44 +
                       16 * s34 * s3n * tmp44 + 20 * s14 * s4n * tmp44 +
                       12 * s24 * s4n * tmp44 - 12 * s34 * s4n * tmp44 +
                       4 * tmp29 * tmp469 + 6 * s3n * tmp47 + 4 * s2n * tmp475 +
                       4 * s3n * tmp475 + 4 * s4n * tmp475 + 6 * s4n * tmp57 +
                       28 * s14 * s2n * tmp59 + 14 * s23 * s2n * tmp59 +
                       14 * s24 * s2n * tmp59 + 60 * s14 * s3n * tmp59 -
                       4 * s23 * s3n * tmp59 - 28 * s24 * s3n * tmp59 +
                       60 * s14 * s4n * tmp59 - 28 * s23 * s4n * tmp59 -
                       4 * s24 * s4n * tmp59 - 4 * s2n * tmp68 -
                       32 * s3n * tmp68 - 32 * s4n * tmp68 +
                       s13 * (44 * s24 * s34 * s3n + 16 * s24 * s34 * s4n +
                              12 * tmp131 * tmp29 + 20 * s3n * tmp40 +
                              (-3 * s2n + tmp292) * tmp44 + 28 * s2n * tmp59 +
                              60 * s3n * tmp59 + 60 * s4n * tmp59 + tmp628 -
                              2 * s23 *
                                  (2 * s34 * (-5 * s4n + tmp290 + tmp707) +
                                   s24 * (8 * s4n + tmp102 + tmp791)) -
                              2 * s14 *
                                  (s34 * (21 * s2n + 32 * s3n + 32 * s4n) +
                                   s24 * (5 * s3n + tmp292 + tmp797) +
                                   s23 * (5 * s4n + tmp295 + tmp797))) +
                       s12 * (-12 * s23 * s2n * s34 - 12 * s24 * s2n * s34 -
                              6 * s23 * s24 * s3n - 10 * s23 * s34 * s3n -
                              10 * s24 * s34 * s3n - 6 * s23 * s24 * s4n -
                              10 * s23 * s34 * s4n - 10 * s24 * s34 * s4n +
                              4 * tmp131 * tmp29 + 4 * tmp29 * tmp39 -
                              3 * s3n * tmp40 - 3 * s4n * tmp40 -
                              3 * s3n * tmp44 - 3 * s4n * tmp44 + tmp445 +
                              6 * s3n * tmp59 + 6 * s4n * tmp59 +
                              s13 * (-18 * s34 * s3n - s24 * s4n -
                                     18 * s34 * s4n + tmp169 + 8 * s14 * tmp29 +
                                     tmp384 + tmp386 + tmp831) +
                              s14 * (-2 * s34 * (tmp177 + 9 * tmp31) +
                                     s24 * tmp830 + tmp831))) +
                  tmp95 -
                  16 *
                      (6 * s13 * s2n + 6 * s14 * s2n + s23 * s2n +
                       10 * s14 * s3n + 2 * s23 * s3n - 16 * s34 * s3n +
                       10 * s14 * s4n - 2 * s23 * s4n - 16 * s34 * s4n +
                       s12 * (-s2n + s3n + s4n) + tmp164 + tmp276 +
                       10 * s13 * tmp31 + tmp411 + tmp612) *
                      tmp96))) /
           (3. * pow(-2 * m32 + s13 + s14 - s34, 2) * pow(tmp10, 2) *
            pow(tmp8, 2));
}

MUSTARD_OPTIMIZE_FAST auto MSqM2ENNEE::MSqMcMuleLegacy(const InitialStateMomenta& pI, const FinalStateMomenta& pF) const -> double {
    const auto& q1{pI};
    const auto& [q2, q3, q4, q6, q5]{pF}; // should 5 <-> 6 for this version
    const CLHEP::HepLorentzVector pol1{InitialStatePolarization()};

    // Adapt from McMule v0.5.1, mudecrare/mudecrare_pm2ennee.f95, FUNCTION PM2ENNEE
    //
    // Copyright (C) 2020-2025  Yannick Ulrich and others (The McMule development team)
    //

    constexpr auto s{[](auto&& a, auto&& b) {
        return 2 * (a * b);
    }};

    const auto s12{s(q1, q2)};
    const auto s13{s(q1, q3)};
    const auto s14{s(q1, q4)};
    const auto s15{s(q1, q5)};
    const auto s16{s(q1, q6)};
    const auto s23{s(q2, q3)};
    const auto s24{s(q2, q4)};
    const auto s25{s(q2, q5)};
    const auto s26{s(q2, q6)};
    const auto s34{s(q3, q4)};
    const auto s35{s(q3, q5)};
    const auto s36{s(q3, q6)};
    const auto s45{s(q4, q5)};
    const auto s46{s(q4, q6)};
    const auto s56{s(q5, q6)};

    const auto s2n{s(q2, pol1)};
    const auto s3n{s(q3, pol1)};
    const auto s4n{s(q4, pol1)};
    const auto s5n{s(q5, pol1)};
    const auto s6n{s(q6, pol1)};

    const auto M1{std::sqrt(0.5 * s(q1, q1))};
    const auto M2{std::sqrt(0.5 * std::abs(s(q2, q2)))};

    using muc::pow;

    const auto if11{
        -4 * pow(M2, 4) * s13 * s24 - 4 * pow(M1, 2) * pow(M2, 2) * s13 * s24 -
        2 * pow(M2, 2) * s13 * s15 * s24 - 2 * pow(M2, 2) * s13 * s16 * s24 +
        2 * s13 * s15 * s16 * s24 + 4 * pow(M1, 2) * pow(M2, 2) * s24 * s35 +
        4 * pow(M2, 2) * s15 * s24 * s35 + 2 * pow(M2, 2) * s16 * s24 * s35 -
        s15 * s16 * s24 * s35 + pow(s16, 2) * s24 * s35 +
        4 * pow(M1, 2) * pow(M2, 2) * s24 * s36 +
        2 * pow(M2, 2) * s15 * s24 * s36 + pow(s15, 2) * s24 * s36 +
        4 * pow(M2, 2) * s16 * s24 * s36 - s15 * s16 * s24 * s36 +
        4 * M1 * pow(M2, 4) * s24 * s3n +
        4 * pow(M1, 3) * pow(M2, 2) * s24 * s3n -
        2 * M1 * s15 * s16 * s24 * s3n - 2 * pow(M1, 2) * s13 * s24 * s56 -
        2 * pow(M2, 2) * s13 * s24 * s56 - s13 * s15 * s24 * s56 -
        s13 * s16 * s24 * s56 + 2 * pow(M1, 2) * s24 * s35 * s56 +
        s15 * s24 * s35 * s56 + 2 * pow(M1, 2) * s24 * s36 * s56 +
        s16 * s24 * s36 * s56 + 2 * pow(M1, 3) * s24 * s3n * s56 +
        2 * M1 * pow(M2, 2) * s24 * s3n * s56 +
        2 * M1 * pow(M2, 2) * s13 * s24 * s5n -
        4 * M1 * pow(M2, 2) * s24 * s35 * s5n -
        2 * M1 * pow(M2, 2) * s24 * s36 * s5n - M1 * s15 * s24 * s36 * s5n +
        M1 * s16 * s24 * s36 * s5n + M1 * s13 * s24 * s56 * s5n -
        M1 * s24 * s35 * s56 * s5n + 2 * M1 * pow(M2, 2) * s13 * s24 * s6n -
        2 * M1 * pow(M2, 2) * s24 * s35 * s6n + M1 * s15 * s24 * s35 * s6n -
        M1 * s16 * s24 * s35 * s6n - 4 * M1 * pow(M2, 2) * s24 * s36 * s6n +
        M1 * s13 * s24 * s56 * s6n - M1 * s24 * s36 * s56 * s6n};
    const auto if22{
        -8 * pow(M2, 4) * s13 * s24 + 2 * pow(M2, 2) * s13 * s24 * s25 +
        2 * pow(M2, 2) * s13 * s24 * s26 + 2 * s13 * s24 * s25 * s26 +
        8 * M1 * pow(M2, 4) * s24 * s3n -
        2 * M1 * pow(M2, 2) * s24 * s25 * s3n -
        2 * M1 * pow(M2, 2) * s24 * s26 * s3n - 2 * M1 * s24 * s25 * s26 * s3n -
        4 * pow(M2, 4) * s13 * s45 + 4 * pow(M2, 2) * s13 * s25 * s45 +
        2 * pow(M2, 2) * s13 * s26 * s45 + s13 * s25 * s26 * s45 -
        s13 * pow(s26, 2) * s45 + 4 * M1 * pow(M2, 4) * s3n * s45 -
        4 * M1 * pow(M2, 2) * s25 * s3n * s45 -
        2 * M1 * pow(M2, 2) * s26 * s3n * s45 - M1 * s25 * s26 * s3n * s45 +
        M1 * pow(s26, 2) * s3n * s45 - 4 * pow(M2, 4) * s13 * s46 +
        2 * pow(M2, 2) * s13 * s25 * s46 - s13 * pow(s25, 2) * s46 +
        4 * pow(M2, 2) * s13 * s26 * s46 + s13 * s25 * s26 * s46 +
        4 * M1 * pow(M2, 4) * s3n * s46 -
        2 * M1 * pow(M2, 2) * s25 * s3n * s46 + M1 * pow(s25, 2) * s3n * s46 -
        4 * M1 * pow(M2, 2) * s26 * s3n * s46 - M1 * s25 * s26 * s3n * s46 -
        4 * pow(M2, 2) * s13 * s24 * s56 + s13 * s24 * s25 * s56 +
        s13 * s24 * s26 * s56 + 4 * M1 * pow(M2, 2) * s24 * s3n * s56 -
        M1 * s24 * s25 * s3n * s56 - M1 * s24 * s26 * s3n * s56 -
        2 * pow(M2, 2) * s13 * s45 * s56 + s13 * s25 * s45 * s56 +
        2 * M1 * pow(M2, 2) * s3n * s45 * s56 - M1 * s25 * s3n * s45 * s56 -
        2 * pow(M2, 2) * s13 * s46 * s56 + s13 * s26 * s46 * s56 +
        2 * M1 * pow(M2, 2) * s3n * s46 * s56 - M1 * s26 * s3n * s46 * s56};
    const auto if33{
        -4 * pow(M2, 4) * s13 * s45 - 4 * pow(M1, 2) * pow(M2, 2) * s13 * s45 -
        2 * pow(M2, 2) * s12 * s13 * s45 - 2 * pow(M2, 2) * s13 * s16 * s45 +
        2 * s12 * s13 * s16 * s45 + 4 * pow(M1, 2) * pow(M2, 2) * s23 * s45 +
        4 * pow(M2, 2) * s12 * s23 * s45 + 2 * pow(M2, 2) * s16 * s23 * s45 -
        s12 * s16 * s23 * s45 + pow(s16, 2) * s23 * s45 -
        2 * pow(M1, 2) * s13 * s26 * s45 - 2 * pow(M2, 2) * s13 * s26 * s45 -
        s12 * s13 * s26 * s45 - s13 * s16 * s26 * s45 +
        2 * pow(M1, 2) * s23 * s26 * s45 + s12 * s23 * s26 * s45 +
        2 * M1 * pow(M2, 2) * s13 * s2n * s45 -
        4 * M1 * pow(M2, 2) * s23 * s2n * s45 + M1 * s13 * s26 * s2n * s45 -
        M1 * s23 * s26 * s2n * s45 + 4 * pow(M1, 2) * pow(M2, 2) * s36 * s45 +
        2 * pow(M2, 2) * s12 * s36 * s45 + pow(s12, 2) * s36 * s45 +
        4 * pow(M2, 2) * s16 * s36 * s45 - s12 * s16 * s36 * s45 +
        2 * pow(M1, 2) * s26 * s36 * s45 + s16 * s26 * s36 * s45 -
        2 * M1 * pow(M2, 2) * s2n * s36 * s45 - M1 * s12 * s2n * s36 * s45 +
        M1 * s16 * s2n * s36 * s45 + 4 * M1 * pow(M2, 4) * s3n * s45 +
        4 * pow(M1, 3) * pow(M2, 2) * s3n * s45 -
        2 * M1 * s12 * s16 * s3n * s45 + 2 * pow(M1, 3) * s26 * s3n * s45 +
        2 * M1 * pow(M2, 2) * s26 * s3n * s45 +
        2 * M1 * pow(M2, 2) * s13 * s45 * s6n -
        2 * M1 * pow(M2, 2) * s23 * s45 * s6n + M1 * s12 * s23 * s45 * s6n -
        M1 * s16 * s23 * s45 * s6n + M1 * s13 * s26 * s45 * s6n -
        4 * M1 * pow(M2, 2) * s36 * s45 * s6n - M1 * s26 * s36 * s45 * s6n};
    const auto if44{
        -4 * pow(M2, 4) * s13 * s24 + 4 * pow(M2, 2) * s13 * s24 * s25 -
        2 * pow(M2, 2) * s13 * s24 * s26 + s13 * s24 * s25 * s26 +
        4 * M1 * pow(M2, 4) * s24 * s3n -
        4 * M1 * pow(M2, 2) * s24 * s25 * s3n +
        2 * M1 * pow(M2, 2) * s24 * s26 * s3n - M1 * s24 * s25 * s26 * s3n -
        8 * pow(M2, 4) * s13 * s45 + 2 * pow(M2, 2) * s13 * s25 * s45 -
        4 * pow(M2, 2) * s13 * s26 * s45 + s13 * s25 * s26 * s45 +
        8 * M1 * pow(M2, 4) * s3n * s45 -
        2 * M1 * pow(M2, 2) * s25 * s3n * s45 +
        4 * M1 * pow(M2, 2) * s26 * s3n * s45 - M1 * s25 * s26 * s3n * s45 -
        4 * pow(M2, 4) * s13 * s46 + 2 * pow(M2, 2) * s13 * s25 * s46 -
        s13 * pow(s25, 2) * s46 - 2 * pow(M2, 2) * s13 * s26 * s46 +
        4 * M1 * pow(M2, 4) * s3n * s46 -
        2 * M1 * pow(M2, 2) * s25 * s3n * s46 + M1 * pow(s25, 2) * s3n * s46 +
        2 * M1 * pow(M2, 2) * s26 * s3n * s46 +
        2 * pow(M2, 2) * s13 * s24 * s56 + s13 * s24 * s25 * s56 -
        2 * M1 * pow(M2, 2) * s24 * s3n * s56 - M1 * s24 * s25 * s3n * s56 +
        2 * pow(M2, 2) * s13 * s45 * s56 + 2 * s13 * s25 * s45 * s56 +
        s13 * s26 * s45 * s56 - 2 * M1 * pow(M2, 2) * s3n * s45 * s56 -
        2 * M1 * s25 * s3n * s45 * s56 - M1 * s26 * s3n * s45 * s56 +
        4 * pow(M2, 2) * s13 * s46 * s56 + s13 * s25 * s46 * s56 +
        s13 * s26 * s46 * s56 - 4 * M1 * pow(M2, 2) * s3n * s46 * s56 -
        M1 * s25 * s3n * s46 * s56 - M1 * s26 * s3n * s46 * s56 -
        s13 * s24 * pow(s56, 2) + M1 * s24 * s3n * pow(s56, 2)};
    const auto if12{
        -8 * pow(M2, 4) * s14 * s23 - 4 * pow(M2, 2) * s12 * s13 * s24 -
        2 * pow(M2, 2) * s15 * s23 * s24 - 2 * pow(M2, 2) * s16 * s23 * s24 +
        2 * pow(M2, 2) * s13 * s14 * s25 + 2 * s13 * s16 * s24 * s25 +
        2 * pow(M2, 2) * s13 * s14 * s26 + 2 * s13 * s15 * s24 * s26 +
        8 * pow(M2, 4) * s12 * s34 - 2 * pow(M2, 2) * s15 * s25 * s34 -
        2 * pow(M2, 2) * s16 * s26 * s34 - 8 * M1 * pow(M2, 4) * s2n * s34 +
        2 * pow(M2, 2) * s12 * s24 * s35 + 2 * pow(M2, 2) * s14 * s25 * s35 -
        s16 * s24 * s25 * s35 + s16 * s24 * s26 * s35 -
        2 * M1 * pow(M2, 2) * s24 * s2n * s35 +
        2 * pow(M2, 2) * s12 * s24 * s36 + s15 * s24 * s25 * s36 +
        2 * pow(M2, 2) * s14 * s26 * s36 - s15 * s24 * s26 * s36 -
        2 * M1 * pow(M2, 2) * s24 * s2n * s36 +
        4 * M1 * pow(M2, 2) * s12 * s24 * s3n -
        2 * M1 * pow(M2, 2) * s14 * s25 * s3n - 2 * M1 * s16 * s24 * s25 * s3n -
        2 * M1 * pow(M2, 2) * s14 * s26 * s3n - 2 * M1 * s15 * s24 * s26 * s3n -
        2 * pow(M2, 2) * s12 * s13 * s45 + 2 * pow(M2, 2) * s15 * s23 * s45 +
        s13 * s15 * s26 * s45 - s13 * s16 * s26 * s45 -
        2 * pow(M2, 2) * s12 * s35 * s45 + 2 * s16 * s26 * s35 * s45 +
        2 * M1 * pow(M2, 2) * s2n * s35 * s45 - 2 * s15 * s26 * s36 * s45 +
        2 * M1 * pow(M2, 2) * s12 * s3n * s45 - M1 * s15 * s26 * s3n * s45 +
        M1 * s16 * s26 * s3n * s45 - 2 * pow(M2, 2) * s12 * s13 * s46 +
        2 * pow(M2, 2) * s16 * s23 * s46 - s13 * s15 * s25 * s46 +
        s13 * s16 * s25 * s46 - 2 * s16 * s25 * s35 * s46 -
        2 * pow(M2, 2) * s12 * s36 * s46 + 2 * s15 * s25 * s36 * s46 +
        2 * M1 * pow(M2, 2) * s2n * s36 * s46 +
        2 * M1 * pow(M2, 2) * s12 * s3n * s46 + M1 * s15 * s25 * s3n * s46 -
        M1 * s16 * s25 * s3n * s46 + 8 * M1 * pow(M2, 4) * s23 * s4n -
        2 * M1 * pow(M2, 2) * s25 * s35 * s4n -
        2 * M1 * pow(M2, 2) * s26 * s36 * s4n -
        4 * pow(M2, 2) * s14 * s23 * s56 - 2 * s12 * s13 * s24 * s56 -
        s15 * s23 * s24 * s56 - s16 * s23 * s24 * s56 + s13 * s14 * s25 * s56 +
        s13 * s14 * s26 * s56 + 4 * pow(M2, 2) * s12 * s34 * s56 +
        s16 * s25 * s34 * s56 + s15 * s26 * s34 * s56 -
        4 * M1 * pow(M2, 2) * s2n * s34 * s56 + s12 * s24 * s35 * s56 -
        s14 * s26 * s35 * s56 - M1 * s24 * s2n * s35 * s56 +
        s12 * s24 * s36 * s56 - s14 * s25 * s36 * s56 -
        M1 * s24 * s2n * s36 * s56 + 2 * M1 * s12 * s24 * s3n * s56 -
        M1 * s14 * s25 * s3n * s56 - M1 * s14 * s26 * s3n * s56 -
        s12 * s13 * s45 * s56 - s16 * s23 * s45 * s56 + s12 * s36 * s45 * s56 -
        M1 * s2n * s36 * s45 * s56 + M1 * s12 * s3n * s45 * s56 -
        s12 * s13 * s46 * s56 - s15 * s23 * s46 * s56 + s12 * s35 * s46 * s56 -
        M1 * s2n * s35 * s46 * s56 + M1 * s12 * s3n * s46 * s56 +
        4 * M1 * pow(M2, 2) * s23 * s4n * s56 + M1 * s26 * s35 * s4n * s56 +
        M1 * s25 * s36 * s4n * s56 + 2 * M1 * pow(M2, 2) * s23 * s24 * s5n +
        2 * M1 * pow(M2, 2) * s25 * s34 * s5n - M1 * s24 * s25 * s36 * s5n +
        M1 * s24 * s26 * s36 * s5n - 2 * M1 * pow(M2, 2) * s23 * s45 * s5n +
        2 * M1 * s26 * s36 * s45 * s5n - 2 * M1 * s25 * s36 * s46 * s5n +
        M1 * s23 * s24 * s56 * s5n - M1 * s26 * s34 * s56 * s5n +
        M1 * s23 * s46 * s56 * s5n + 2 * M1 * pow(M2, 2) * s23 * s24 * s6n +
        2 * M1 * pow(M2, 2) * s26 * s34 * s6n + M1 * s24 * s25 * s35 * s6n -
        M1 * s24 * s26 * s35 * s6n - 2 * M1 * s26 * s35 * s45 * s6n -
        2 * M1 * pow(M2, 2) * s23 * s46 * s6n + 2 * M1 * s25 * s35 * s46 * s6n +
        M1 * s23 * s24 * s56 * s6n - M1 * s25 * s34 * s56 * s6n +
        M1 * s23 * s45 * s56 * s6n};
    const auto if13{
        4 * pow(M2, 4) * s13 * s14 - 2 * pow(M2, 2) * s13 * s14 * s16 -
        2 * pow(M2, 4) * s14 * s23 - pow(M2, 2) * s14 * s15 * s23 -
        pow(M2, 2) * s14 * s16 * s23 + 2 * pow(M2, 4) * s13 * s24 +
        2 * pow(M1, 2) * pow(M2, 2) * s13 * s24 - pow(M2, 2) * s13 * s15 * s24 +
        pow(M2, 2) * s13 * s16 * s24 - s13 * s15 * s16 * s24 +
        2 * pow(M2, 2) * s13 * s14 * s25 + s13 * s14 * s16 * s25 +
        2 * pow(M2, 2) * s13 * s14 * s26 - 8 * pow(M1, 2) * pow(M2, 4) * s34 -
        2 * pow(M2, 4) * s12 * s34 - 2 * pow(M2, 4) * s15 * s34 +
        2 * pow(M2, 2) * s12 * s15 * s34 - 4 * pow(M2, 4) * s16 * s34 +
        2 * pow(M2, 2) * s12 * s16 * s34 + 2 * pow(M2, 2) * s15 * s16 * s34 -
        4 * pow(M1, 2) * pow(M2, 2) * s25 * s34 + pow(s16, 2) * s25 * s34 -
        4 * pow(M1, 2) * pow(M2, 2) * s26 * s34 - pow(M2, 2) * s15 * s26 * s34 -
        pow(M2, 2) * s16 * s26 * s34 + 2 * M1 * pow(M2, 4) * s2n * s34 -
        M1 * pow(M2, 2) * s15 * s2n * s34 - M1 * pow(M2, 2) * s16 * s2n * s34 -
        2 * pow(M2, 4) * s14 * s35 - pow(M2, 2) * s12 * s14 * s35 -
        pow(M2, 2) * s14 * s16 * s35 + 2 * pow(M1, 2) * pow(M2, 2) * s24 * s35 -
        pow(s16, 2) * s24 * s35 - pow(M2, 2) * s14 * s26 * s35 -
        4 * pow(M2, 4) * s14 * s36 - pow(M2, 2) * s12 * s14 * s36 -
        pow(M2, 2) * s14 * s15 * s36 - 2 * pow(M1, 2) * pow(M2, 2) * s24 * s36 -
        2 * pow(M2, 2) * s16 * s24 * s36 + s15 * s16 * s24 * s36 -
        s14 * s16 * s25 * s36 - pow(M2, 2) * s14 * s26 * s36 +
        2 * M1 * pow(M2, 2) * s14 * s16 * s3n -
        2 * M1 * pow(M2, 4) * s24 * s3n -
        2 * pow(M1, 3) * pow(M2, 2) * s24 * s3n + M1 * s15 * s16 * s24 * s3n -
        M1 * s14 * s16 * s25 * s3n + 2 * pow(M2, 4) * s13 * s45 +
        2 * pow(M1, 2) * pow(M2, 2) * s13 * s45 - pow(M2, 2) * s12 * s13 * s45 +
        pow(M2, 2) * s13 * s16 * s45 - s12 * s13 * s16 * s45 +
        2 * pow(M1, 2) * pow(M2, 2) * s23 * s45 - pow(s16, 2) * s23 * s45 +
        pow(M1, 2) * s13 * s26 * s45 + pow(M2, 2) * s13 * s26 * s45 +
        s13 * s16 * s26 * s45 + M1 * pow(M2, 2) * s13 * s2n * s45 -
        2 * pow(M1, 2) * pow(M2, 2) * s36 * s45 -
        2 * pow(M2, 2) * s16 * s36 * s45 + s12 * s16 * s36 * s45 -
        2 * pow(M1, 2) * s26 * s36 * s45 - s16 * s26 * s36 * s45 -
        M1 * s16 * s2n * s36 * s45 - 2 * M1 * pow(M2, 4) * s3n * s45 -
        2 * pow(M1, 3) * pow(M2, 2) * s3n * s45 + M1 * s12 * s16 * s3n * s45 -
        pow(M1, 3) * s26 * s3n * s45 - M1 * pow(M2, 2) * s26 * s3n * s45 +
        2 * pow(M2, 4) * s13 * s46 + 2 * pow(M1, 2) * pow(M2, 2) * s13 * s46 -
        pow(M2, 2) * s12 * s13 * s46 - pow(M2, 2) * s13 * s15 * s46 +
        2 * pow(M1, 2) * pow(M2, 2) * s23 * s46 + pow(M2, 2) * s15 * s23 * s46 +
        pow(M2, 2) * s16 * s23 * s46 - pow(M1, 2) * s13 * s25 * s46 -
        pow(M2, 2) * s13 * s25 * s46 - s13 * s16 * s25 * s46 +
        M1 * pow(M2, 2) * s13 * s2n * s46 +
        2 * pow(M1, 2) * pow(M2, 2) * s35 * s46 + pow(M2, 2) * s12 * s35 * s46 +
        pow(M2, 2) * s16 * s35 * s46 - M1 * pow(M2, 2) * s2n * s35 * s46 +
        pow(M2, 2) * s12 * s36 * s46 + pow(M2, 2) * s15 * s36 * s46 +
        2 * pow(M1, 2) * s25 * s36 * s46 + s16 * s25 * s36 * s46 -
        M1 * pow(M2, 2) * s2n * s36 * s46 - 2 * M1 * pow(M2, 4) * s3n * s46 -
        2 * pow(M1, 3) * pow(M2, 2) * s3n * s46 + pow(M1, 3) * s25 * s3n * s46 +
        M1 * pow(M2, 2) * s25 * s3n * s46 - 4 * M1 * pow(M2, 4) * s13 * s4n +
        2 * M1 * pow(M2, 4) * s23 * s4n + M1 * pow(M2, 2) * s15 * s23 * s4n +
        M1 * pow(M2, 2) * s16 * s23 * s4n -
        2 * M1 * pow(M2, 2) * s13 * s25 * s4n -
        2 * M1 * pow(M2, 2) * s13 * s26 * s4n +
        2 * M1 * pow(M2, 4) * s35 * s4n + M1 * pow(M2, 2) * s12 * s35 * s4n +
        M1 * pow(M2, 2) * s16 * s35 * s4n + M1 * pow(M2, 2) * s26 * s35 * s4n +
        4 * M1 * pow(M2, 4) * s36 * s4n + M1 * pow(M2, 2) * s12 * s36 * s4n +
        M1 * pow(M2, 2) * s15 * s36 * s4n + M1 * s16 * s25 * s36 * s4n +
        M1 * pow(M2, 2) * s26 * s36 * s4n + 2 * pow(M2, 2) * s13 * s14 * s56 -
        pow(M2, 2) * s14 * s23 * s56 + pow(M1, 2) * s13 * s24 * s56 +
        pow(M2, 2) * s13 * s24 * s56 + s13 * s16 * s24 * s56 -
        4 * pow(M1, 2) * pow(M2, 2) * s34 * s56 - pow(M2, 2) * s12 * s34 * s56 -
        pow(M2, 2) * s16 * s34 * s56 + M1 * pow(M2, 2) * s2n * s34 * s56 -
        pow(M2, 2) * s14 * s36 * s56 - 2 * pow(M1, 2) * s24 * s36 * s56 -
        s16 * s24 * s36 * s56 - pow(M1, 3) * s24 * s3n * s56 -
        M1 * pow(M2, 2) * s24 * s3n * s56 -
        2 * M1 * pow(M2, 2) * s13 * s4n * s56 +
        M1 * pow(M2, 2) * s23 * s4n * s56 + M1 * pow(M2, 2) * s36 * s4n * s56 +
        M1 * pow(M2, 2) * s13 * s24 * s5n + 2 * M1 * pow(M2, 4) * s34 * s5n -
        M1 * pow(M2, 2) * s12 * s34 * s5n - M1 * pow(M2, 2) * s16 * s34 * s5n +
        M1 * pow(M2, 2) * s26 * s34 * s5n - M1 * s16 * s24 * s36 * s5n +
        M1 * pow(M2, 2) * s13 * s46 * s5n - M1 * pow(M2, 2) * s23 * s46 * s5n -
        M1 * pow(M2, 2) * s36 * s46 * s5n - M1 * pow(M2, 2) * s13 * s24 * s6n +
        4 * M1 * pow(M2, 4) * s34 * s6n - M1 * pow(M2, 2) * s12 * s34 * s6n -
        M1 * pow(M2, 2) * s15 * s34 * s6n - M1 * s16 * s25 * s34 * s6n +
        M1 * pow(M2, 2) * s26 * s34 * s6n + M1 * s16 * s24 * s35 * s6n +
        2 * M1 * pow(M2, 2) * s24 * s36 * s6n -
        M1 * pow(M2, 2) * s13 * s45 * s6n + M1 * s16 * s23 * s45 * s6n -
        M1 * s13 * s26 * s45 * s6n + 2 * M1 * pow(M2, 2) * s36 * s45 * s6n +
        M1 * s26 * s36 * s45 * s6n - M1 * pow(M2, 2) * s23 * s46 * s6n +
        M1 * s13 * s25 * s46 * s6n - M1 * pow(M2, 2) * s35 * s46 * s6n -
        M1 * s25 * s36 * s46 * s6n - M1 * s13 * s24 * s56 * s6n +
        M1 * pow(M2, 2) * s34 * s56 * s6n + M1 * s24 * s36 * s56 * s6n};
    const auto if14{
        -4 * pow(M2, 4) * s14 * s23 - 2 * pow(M2, 2) * s12 * s13 * s24 +
        4 * pow(M2, 2) * s13 * s15 * s24 - 4 * pow(M2, 2) * s13 * s16 * s24 -
        2 * pow(M2, 2) * s16 * s23 * s24 + 2 * pow(M2, 2) * s13 * s14 * s25 +
        s13 * s16 * s24 * s25 + s13 * s15 * s24 * s26 +
        4 * pow(M2, 4) * s12 * s34 - 2 * pow(M2, 4) * s15 * s34 +
        2 * pow(M2, 4) * s16 * s34 - pow(M2, 2) * s15 * s25 * s34 +
        pow(M2, 2) * s16 * s25 * s34 - pow(M2, 2) * s15 * s26 * s34 -
        pow(M2, 2) * s16 * s26 * s34 - 4 * M1 * pow(M2, 4) * s2n * s34 +
        2 * pow(M2, 4) * s14 * s35 + 4 * pow(M2, 2) * s16 * s24 * s35 +
        pow(M2, 2) * s14 * s25 * s35 + pow(M2, 2) * s14 * s26 * s35 +
        s16 * s24 * s26 * s35 - 2 * pow(M2, 4) * s14 * s36 +
        2 * pow(M2, 2) * s12 * s24 * s36 - 4 * pow(M2, 2) * s15 * s24 * s36 -
        pow(M2, 2) * s14 * s25 * s36 + pow(M2, 2) * s14 * s26 * s36 -
        s15 * s24 * s26 * s36 - 2 * M1 * pow(M2, 2) * s24 * s2n * s36 +
        2 * M1 * pow(M2, 2) * s12 * s24 * s3n -
        4 * M1 * pow(M2, 2) * s15 * s24 * s3n +
        4 * M1 * pow(M2, 2) * s16 * s24 * s3n -
        2 * M1 * pow(M2, 2) * s14 * s25 * s3n - M1 * s16 * s24 * s25 * s3n -
        M1 * s15 * s24 * s26 * s3n - 2 * pow(M2, 2) * s12 * s13 * s45 +
        2 * pow(M2, 2) * s13 * s15 * s45 - 2 * pow(M2, 2) * s13 * s16 * s45 +
        pow(M2, 2) * s15 * s23 * s45 - pow(M2, 2) * s16 * s23 * s45 +
        s13 * s15 * s26 * s45 - pow(M2, 2) * s12 * s35 * s45 +
        pow(M2, 2) * s16 * s35 * s45 + s16 * s26 * s35 * s45 +
        M1 * pow(M2, 2) * s2n * s35 * s45 + pow(M2, 2) * s12 * s36 * s45 -
        pow(M2, 2) * s15 * s36 * s45 - s15 * s26 * s36 * s45 -
        M1 * pow(M2, 2) * s2n * s36 * s45 +
        2 * M1 * pow(M2, 2) * s12 * s3n * s45 -
        2 * M1 * pow(M2, 2) * s15 * s3n * s45 +
        2 * M1 * pow(M2, 2) * s16 * s3n * s45 - M1 * s15 * s26 * s3n * s45 +
        2 * pow(M2, 2) * s13 * s15 * s46 - 2 * pow(M2, 2) * s13 * s16 * s46 +
        pow(M2, 2) * s15 * s23 * s46 + pow(M2, 2) * s16 * s23 * s46 -
        s13 * s15 * s25 * s46 - pow(M2, 2) * s12 * s35 * s46 +
        pow(M2, 2) * s16 * s35 * s46 - s16 * s25 * s35 * s46 +
        M1 * pow(M2, 2) * s2n * s35 * s46 - pow(M2, 2) * s12 * s36 * s46 -
        pow(M2, 2) * s15 * s36 * s46 + s15 * s25 * s36 * s46 +
        M1 * pow(M2, 2) * s2n * s36 * s46 -
        2 * M1 * pow(M2, 2) * s15 * s3n * s46 +
        2 * M1 * pow(M2, 2) * s16 * s3n * s46 + M1 * s15 * s25 * s3n * s46 +
        4 * M1 * pow(M2, 4) * s23 * s4n - 2 * M1 * pow(M2, 4) * s35 * s4n -
        M1 * pow(M2, 2) * s25 * s35 * s4n - M1 * pow(M2, 2) * s26 * s35 * s4n +
        2 * M1 * pow(M2, 4) * s36 * s4n + M1 * pow(M2, 2) * s25 * s36 * s4n -
        M1 * pow(M2, 2) * s26 * s36 * s4n - 2 * pow(M2, 2) * s14 * s23 * s56 -
        s12 * s13 * s24 * s56 - s16 * s23 * s24 * s56 + s13 * s14 * s25 * s56 +
        2 * pow(M2, 2) * s12 * s34 * s56 - pow(M2, 2) * s15 * s34 * s56 +
        pow(M2, 2) * s16 * s34 * s56 + s16 * s25 * s34 * s56 -
        2 * M1 * pow(M2, 2) * s2n * s34 * s56 + pow(M2, 2) * s14 * s35 * s56 -
        pow(M2, 2) * s14 * s36 * s56 + s12 * s24 * s36 * s56 -
        s14 * s25 * s36 * s56 - M1 * s24 * s2n * s36 * s56 +
        M1 * s12 * s24 * s3n * s56 - M1 * s14 * s25 * s3n * s56 -
        s12 * s13 * s45 * s56 - s16 * s23 * s45 * s56 + s12 * s36 * s45 * s56 -
        M1 * s2n * s36 * s45 * s56 + M1 * s12 * s3n * s45 * s56 +
        2 * M1 * pow(M2, 2) * s23 * s4n * s56 -
        M1 * pow(M2, 2) * s35 * s4n * s56 + M1 * pow(M2, 2) * s36 * s4n * s56 +
        M1 * s25 * s36 * s4n * s56 + 2 * M1 * pow(M2, 4) * s34 * s5n +
        M1 * pow(M2, 2) * s25 * s34 * s5n + M1 * pow(M2, 2) * s26 * s34 * s5n +
        4 * M1 * pow(M2, 2) * s24 * s36 * s5n + M1 * s24 * s26 * s36 * s5n -
        M1 * pow(M2, 2) * s23 * s45 * s5n + M1 * pow(M2, 2) * s36 * s45 * s5n +
        M1 * s26 * s36 * s45 * s5n - M1 * pow(M2, 2) * s23 * s46 * s5n +
        M1 * pow(M2, 2) * s36 * s46 * s5n - M1 * s25 * s36 * s46 * s5n +
        M1 * pow(M2, 2) * s34 * s56 * s5n +
        2 * M1 * pow(M2, 2) * s23 * s24 * s6n -
        2 * M1 * pow(M2, 4) * s34 * s6n - M1 * pow(M2, 2) * s25 * s34 * s6n +
        M1 * pow(M2, 2) * s26 * s34 * s6n -
        4 * M1 * pow(M2, 2) * s24 * s35 * s6n - M1 * s24 * s26 * s35 * s6n +
        M1 * pow(M2, 2) * s23 * s45 * s6n - M1 * pow(M2, 2) * s35 * s45 * s6n -
        M1 * s26 * s35 * s45 * s6n - M1 * pow(M2, 2) * s23 * s46 * s6n -
        M1 * pow(M2, 2) * s35 * s46 * s6n + M1 * s25 * s35 * s46 * s6n +
        M1 * s23 * s24 * s56 * s6n - M1 * pow(M2, 2) * s34 * s56 * s6n -
        M1 * s25 * s34 * s56 * s6n + M1 * s23 * s45 * s56 * s6n};
    const auto if23{
        2 * pow(M2, 4) * s14 * s23 + 2 * pow(M2, 2) * s12 * s13 * s24 -
        2 * pow(M2, 2) * s13 * s15 * s24 - 2 * pow(M2, 2) * s13 * s16 * s24 -
        pow(M2, 2) * s15 * s23 * s24 + pow(M2, 2) * s16 * s23 * s24 +
        2 * pow(M2, 2) * s13 * s14 * s25 + pow(M2, 2) * s14 * s23 * s25 +
        pow(M2, 2) * s14 * s23 * s26 - s13 * s15 * s24 * s26 +
        s13 * s14 * s25 * s26 - 2 * pow(M2, 4) * s12 * s34 +
        4 * pow(M2, 4) * s15 * s34 + 2 * pow(M2, 4) * s16 * s34 -
        pow(M2, 2) * s12 * s25 * s34 + pow(M2, 2) * s16 * s25 * s34 -
        pow(M2, 2) * s12 * s26 * s34 + 2 * pow(M2, 2) * s15 * s26 * s34 +
        pow(M2, 2) * s16 * s26 * s34 + s16 * s25 * s26 * s34 +
        2 * M1 * pow(M2, 4) * s2n * s34 + M1 * pow(M2, 2) * s25 * s2n * s34 +
        M1 * pow(M2, 2) * s26 * s2n * s34 - 4 * pow(M2, 4) * s14 * s35 +
        pow(M2, 2) * s12 * s24 * s35 - pow(M2, 2) * s16 * s24 * s35 -
        2 * pow(M2, 2) * s14 * s26 * s35 - s16 * s24 * s26 * s35 -
        M1 * pow(M2, 2) * s24 * s2n * s35 - 2 * pow(M2, 4) * s14 * s36 -
        pow(M2, 2) * s12 * s24 * s36 + pow(M2, 2) * s15 * s24 * s36 -
        pow(M2, 2) * s14 * s25 * s36 - pow(M2, 2) * s14 * s26 * s36 +
        s15 * s24 * s26 * s36 - s14 * s25 * s26 * s36 +
        M1 * pow(M2, 2) * s24 * s2n * s36 -
        2 * M1 * pow(M2, 2) * s12 * s24 * s3n +
        2 * M1 * pow(M2, 2) * s15 * s24 * s3n +
        2 * M1 * pow(M2, 2) * s16 * s24 * s3n -
        2 * M1 * pow(M2, 2) * s14 * s25 * s3n + M1 * s15 * s24 * s26 * s3n -
        M1 * s14 * s25 * s26 * s3n + 4 * pow(M2, 2) * s12 * s13 * s45 -
        2 * pow(M2, 2) * s13 * s15 * s45 - 4 * pow(M2, 2) * s13 * s16 * s45 +
        4 * pow(M2, 2) * s16 * s23 * s45 + s13 * s16 * s25 * s45 -
        s13 * s15 * s26 * s45 - 2 * pow(M2, 2) * s16 * s35 * s45 -
        s16 * s26 * s35 * s45 - 4 * pow(M2, 2) * s12 * s36 * s45 +
        2 * pow(M2, 2) * s15 * s36 * s45 + s15 * s26 * s36 * s45 +
        4 * M1 * pow(M2, 2) * s2n * s36 * s45 -
        4 * M1 * pow(M2, 2) * s12 * s3n * s45 +
        2 * M1 * pow(M2, 2) * s15 * s3n * s45 +
        4 * M1 * pow(M2, 2) * s16 * s3n * s45 - M1 * s16 * s25 * s3n * s45 +
        M1 * s15 * s26 * s3n * s45 + 2 * pow(M2, 2) * s12 * s13 * s46 -
        2 * pow(M2, 2) * s13 * s16 * s46 - pow(M2, 2) * s15 * s23 * s46 +
        pow(M2, 2) * s16 * s23 * s46 - s12 * s13 * s25 * s46 -
        s16 * s23 * s25 * s46 + pow(M2, 2) * s12 * s35 * s46 +
        pow(M2, 2) * s16 * s35 * s46 - M1 * pow(M2, 2) * s2n * s35 * s46 -
        pow(M2, 2) * s12 * s36 * s46 - pow(M2, 2) * s15 * s36 * s46 +
        s12 * s25 * s36 * s46 + M1 * pow(M2, 2) * s2n * s36 * s46 -
        M1 * s25 * s2n * s36 * s46 - 2 * M1 * pow(M2, 2) * s12 * s3n * s46 +
        2 * M1 * pow(M2, 2) * s16 * s3n * s46 + M1 * s12 * s25 * s3n * s46 -
        2 * M1 * pow(M2, 4) * s23 * s4n - M1 * pow(M2, 2) * s23 * s25 * s4n -
        M1 * pow(M2, 2) * s23 * s26 * s4n + 4 * M1 * pow(M2, 4) * s35 * s4n +
        2 * M1 * pow(M2, 2) * s26 * s35 * s4n +
        2 * M1 * pow(M2, 4) * s36 * s4n + M1 * pow(M2, 2) * s25 * s36 * s4n +
        M1 * pow(M2, 2) * s26 * s36 * s4n + M1 * s25 * s26 * s36 * s4n +
        pow(M2, 2) * s14 * s23 * s56 + s12 * s13 * s24 * s56 +
        s16 * s23 * s24 * s56 - pow(M2, 2) * s12 * s34 * s56 -
        pow(M2, 2) * s16 * s34 * s56 + M1 * pow(M2, 2) * s2n * s34 * s56 +
        pow(M2, 2) * s14 * s36 * s56 - s12 * s24 * s36 * s56 +
        M1 * s24 * s2n * s36 * s56 - M1 * s12 * s24 * s3n * s56 +
        s12 * s13 * s45 * s56 + s16 * s23 * s45 * s56 - s12 * s36 * s45 * s56 +
        M1 * s2n * s36 * s45 * s56 - M1 * s12 * s3n * s45 * s56 -
        M1 * pow(M2, 2) * s23 * s4n * s56 - M1 * pow(M2, 2) * s36 * s4n * s56 +
        M1 * pow(M2, 2) * s23 * s24 * s5n - 4 * M1 * pow(M2, 4) * s34 * s5n -
        2 * M1 * pow(M2, 2) * s26 * s34 * s5n -
        M1 * pow(M2, 2) * s24 * s36 * s5n - M1 * s24 * s26 * s36 * s5n -
        2 * M1 * pow(M2, 2) * s36 * s45 * s5n - M1 * s26 * s36 * s45 * s5n +
        M1 * pow(M2, 2) * s23 * s46 * s5n + M1 * pow(M2, 2) * s36 * s46 * s5n -
        M1 * pow(M2, 2) * s23 * s24 * s6n - 2 * M1 * pow(M2, 4) * s34 * s6n -
        M1 * pow(M2, 2) * s25 * s34 * s6n - M1 * pow(M2, 2) * s26 * s34 * s6n -
        M1 * s25 * s26 * s34 * s6n + M1 * pow(M2, 2) * s24 * s35 * s6n +
        M1 * s24 * s26 * s35 * s6n - 4 * M1 * pow(M2, 2) * s23 * s45 * s6n +
        2 * M1 * pow(M2, 2) * s35 * s45 * s6n + M1 * s26 * s35 * s45 * s6n -
        M1 * pow(M2, 2) * s23 * s46 * s6n + M1 * s23 * s25 * s46 * s6n -
        M1 * pow(M2, 2) * s35 * s46 * s6n - M1 * s23 * s24 * s56 * s6n +
        M1 * pow(M2, 2) * s34 * s56 * s6n - M1 * s23 * s45 * s56 * s6n};
    const auto if24{
        -8 * pow(M2, 4) * s13 * s24 + 8 * pow(M2, 2) * s13 * s24 * s25 -
        4 * pow(M2, 2) * s13 * s24 * s26 + 2 * s13 * s24 * s25 * s26 +
        8 * M1 * pow(M2, 4) * s24 * s3n -
        8 * M1 * pow(M2, 2) * s24 * s25 * s3n +
        4 * M1 * pow(M2, 2) * s24 * s26 * s3n - 2 * M1 * s24 * s25 * s26 * s3n -
        8 * pow(M2, 4) * s13 * s45 + 8 * pow(M2, 2) * s13 * s25 * s45 -
        4 * pow(M2, 2) * s13 * s26 * s45 + 2 * s13 * s25 * s26 * s45 +
        8 * M1 * pow(M2, 4) * s3n * s45 -
        8 * M1 * pow(M2, 2) * s25 * s3n * s45 +
        4 * M1 * pow(M2, 2) * s26 * s3n * s45 - 2 * M1 * s25 * s26 * s3n * s45 +
        8 * pow(M2, 2) * s13 * s25 * s46 - 2 * s13 * pow(s25, 2) * s46 -
        8 * M1 * pow(M2, 2) * s25 * s3n * s46 +
        2 * M1 * pow(s25, 2) * s3n * s46 - 4 * pow(M2, 2) * s13 * s24 * s56 +
        2 * s13 * s24 * s25 * s56 + 4 * M1 * pow(M2, 2) * s24 * s3n * s56 -
        2 * M1 * s24 * s25 * s3n * s56 - 4 * pow(M2, 2) * s13 * s45 * s56 +
        2 * s13 * s25 * s45 * s56 + 4 * M1 * pow(M2, 2) * s3n * s45 * s56 -
        2 * M1 * s25 * s3n * s45 * s56};
    const auto if34{
        -2 * pow(M2, 2) * s13 * s15 * s24 - 2 * pow(M2, 2) * s15 * s23 * s24 +
        2 * pow(M2, 2) * s13 * s14 * s25 + 2 * pow(M2, 2) * s14 * s23 * s25 -
        s13 * s15 * s24 * s26 + s13 * s14 * s25 * s26 +
        8 * pow(M2, 4) * s15 * s34 - 2 * pow(M2, 2) * s12 * s25 * s34 +
        4 * pow(M2, 2) * s15 * s26 * s34 + s16 * s25 * s26 * s34 +
        2 * M1 * pow(M2, 2) * s25 * s2n * s34 - 8 * pow(M2, 4) * s14 * s35 +
        2 * pow(M2, 2) * s12 * s24 * s35 - 4 * pow(M2, 2) * s14 * s26 * s35 -
        s16 * s24 * s26 * s35 - 2 * M1 * pow(M2, 2) * s24 * s2n * s35 +
        s15 * s24 * s26 * s36 - s14 * s25 * s26 * s36 +
        2 * M1 * pow(M2, 2) * s15 * s24 * s3n -
        2 * M1 * pow(M2, 2) * s14 * s25 * s3n + M1 * s15 * s24 * s26 * s3n -
        M1 * s14 * s25 * s26 * s3n - 4 * pow(M2, 2) * s13 * s15 * s45 +
        2 * pow(M2, 2) * s15 * s23 * s45 + 2 * s13 * s16 * s25 * s45 -
        s16 * s23 * s25 * s45 - 2 * s13 * s15 * s26 * s45 +
        s15 * s23 * s26 * s45 - 2 * pow(M2, 2) * s12 * s35 * s45 -
        2 * pow(M2, 2) * s16 * s35 * s45 - s12 * s26 * s35 * s45 -
        s16 * s26 * s35 * s45 + 2 * M1 * pow(M2, 2) * s2n * s35 * s45 +
        M1 * s26 * s2n * s35 * s45 + 2 * pow(M2, 2) * s15 * s36 * s45 +
        s12 * s25 * s36 * s45 + s15 * s26 * s36 * s45 -
        M1 * s25 * s2n * s36 * s45 + 4 * M1 * pow(M2, 2) * s15 * s3n * s45 -
        2 * M1 * s16 * s25 * s3n * s45 + 2 * M1 * s15 * s26 * s3n * s45 -
        2 * pow(M2, 2) * s13 * s15 * s46 - s12 * s13 * s25 * s46 +
        s13 * s16 * s25 * s46 - 2 * s16 * s23 * s25 * s46 -
        s13 * s15 * s26 * s46 + s15 * s23 * s26 * s46 +
        2 * pow(M2, 2) * s16 * s35 * s46 - s12 * s26 * s35 * s46 +
        M1 * s26 * s2n * s35 * s46 - 2 * pow(M2, 2) * s15 * s36 * s46 +
        2 * s12 * s25 * s36 * s46 - 2 * M1 * s25 * s2n * s36 * s46 +
        2 * M1 * pow(M2, 2) * s15 * s3n * s46 + M1 * s12 * s25 * s3n * s46 -
        M1 * s16 * s25 * s3n * s46 + M1 * s15 * s26 * s3n * s46 -
        2 * M1 * pow(M2, 2) * s23 * s25 * s4n +
        8 * M1 * pow(M2, 4) * s35 * s4n +
        4 * M1 * pow(M2, 2) * s26 * s35 * s4n + M1 * s25 * s26 * s36 * s4n +
        2 * pow(M2, 2) * s13 * s14 * s56 + s12 * s13 * s24 * s56 -
        s13 * s16 * s24 * s56 + 2 * s16 * s23 * s24 * s56 +
        s13 * s14 * s26 * s56 - s14 * s23 * s26 * s56 -
        2 * pow(M2, 2) * s16 * s34 * s56 + s12 * s26 * s34 * s56 -
        M1 * s26 * s2n * s34 * s56 + 2 * pow(M2, 2) * s14 * s36 * s56 -
        2 * s12 * s24 * s36 * s56 + 2 * M1 * s24 * s2n * s36 * s56 -
        2 * M1 * pow(M2, 2) * s14 * s3n * s56 - M1 * s12 * s24 * s3n * s56 +
        M1 * s16 * s24 * s3n * s56 - M1 * s14 * s26 * s3n * s56 +
        2 * s12 * s13 * s45 * s56 + s16 * s23 * s45 * s56 -
        s12 * s36 * s45 * s56 + M1 * s2n * s36 * s45 * s56 -
        2 * M1 * s12 * s3n * s45 * s56 + M1 * s23 * s26 * s4n * s56 -
        2 * M1 * pow(M2, 2) * s36 * s4n * s56 +
        2 * M1 * pow(M2, 2) * s23 * s24 * s5n -
        8 * M1 * pow(M2, 4) * s34 * s5n -
        4 * M1 * pow(M2, 2) * s26 * s34 * s5n - M1 * s24 * s26 * s36 * s5n -
        2 * M1 * pow(M2, 2) * s23 * s45 * s5n - M1 * s23 * s26 * s45 * s5n -
        2 * M1 * pow(M2, 2) * s36 * s45 * s5n - M1 * s26 * s36 * s45 * s5n -
        M1 * s23 * s26 * s46 * s5n + 2 * M1 * pow(M2, 2) * s36 * s46 * s5n -
        M1 * s25 * s26 * s34 * s6n + M1 * s24 * s26 * s35 * s6n +
        M1 * s23 * s25 * s45 * s6n + 2 * M1 * pow(M2, 2) * s35 * s45 * s6n +
        M1 * s26 * s35 * s45 * s6n + 2 * M1 * s23 * s25 * s46 * s6n -
        2 * M1 * pow(M2, 2) * s35 * s46 * s6n - 2 * M1 * s23 * s24 * s56 * s6n +
        2 * M1 * pow(M2, 2) * s34 * s56 * s6n - M1 * s23 * s45 * s56 * s6n};

    const auto den1{(2 * pow(M2, 2) + s56) *
                    (2 * pow(M2, 2) - s15 - s16 + s56)};
    const auto den2{(2 * pow(M2, 2) + s56) *
                    (2 * pow(M2, 2) + s25 + s26 + s56)};
    const auto den3{(2 * pow(M2, 2) + s26) *
                    (2 * pow(M2, 2) - s12 - s16 + s26)};
    const auto den4{(2 * pow(M2, 2) + s26) *
                    (2 * pow(M2, 2) + s25 + s26 + s56)};

    const auto pm2ennee{8. * (if11 / pow(den1, 2) + if22 / pow(den2, 2) +
                              if33 / pow(den3, 2) + if44 / pow(den4, 2) +
                              if12 / (den1 * den2) + if13 / (den1 * den3) +
                              if14 / (den1 * den4) + if23 / (den2 * den3) +
                              if24 / (den2 * den4) + if34 / (den3 * den4))};

    constexpr auto constant{8. * pow(reduced_fermi_constant, 2) * pow(4. * pi * fine_structure_const, 2)};
    return constant * pm2ennee;
}

} // namespace Mustard::inline Physics::QFT
