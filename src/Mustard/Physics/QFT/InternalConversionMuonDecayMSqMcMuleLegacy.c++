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

#include "Mustard/Physics/QFT/InternalConversionMuonDecayMSqMcMuleLegacy.h++"
#include "Mustard/Utility/MathConstant.h++"
#include "Mustard/Utility/PhysicalConstant.h++"

#include "CLHEP/Vector/LorentzVector.h"

#include "muc/math"

#include <cmath>

namespace Mustard::inline Physics::QFT {

using namespace PhysicalConstant;
using namespace MathConstant;

MUSTARD_OPTIMIZE_FAST auto InternalConversionMuonDecayMSqMcMuleLegacy::operator()(const InitialStateMomenta& pI, const FinalStateMomenta& pF) const -> double {
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
