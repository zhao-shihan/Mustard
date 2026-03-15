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

#include "Mustard/Math/Vector.h++"
#include "Mustard/Physics/QFT/MSqM2ENNG.h++"
#include "Mustard/Utility/MathConstant.h++"
#include "Mustard/Utility/PhysicalConstant.h++"

#include "muc/math"

namespace Mustard::inline Physics::QFT {

using namespace PhysicalConstant;
using namespace MathConstant;

auto MSqM2ENNG::operator()(const InitialStateMomenta& pI, const FinalStateMomenta& pF) const -> double {
    const auto& q1{pI};
    const auto& [q2, _1, _2, q5]{pF};

    // Adapt from McMule v0.7, mudec/mudec_mat_el.f95
    //
    // Copyright (C) 2020-2026  Yannick Ulrich and others (The McMule development team)
    //

    const auto& p1{q1};
    const auto p2{q1 - q2};
    const auto& p4{q5};

    const auto mm{q1.m()};
    const auto me2{q2.m2()};

    const auto p1p2{p1 * p2};
    const auto p1p4{p1 * p4};
    const auto p2p4{p2 * p4};

    const VectorLor pol1{Polarization()};
    const auto np2{pol1 * p2};
    const auto np4{pol1 * p4};

    return PM2ENNGav(mm, me2, p1p2, p1p4, p2p4, np2, np4);
}

MUSTARD_OPTIMIZE_FAST auto MSqM2ENNG::PM2ENNGav(double mm, double me2, double p1p2, double p1p4,
                                                double p2p4, double np2, double np4) -> double {
    using muc::pow;

    // Adapt from McMule v0.7, mudec/mudec_mat_el.f95
    //
    // Copyright (C) 2020-2026  Yannick Ulrich and others (The McMule development team)
    //

    const auto mm2{pow(mm, 2)};
    return ((-32 * 4 * pi * fine_structure_const *
             pow(reduced_Fermi_constant, 2) *
             (pow(me2, 2) * p1p4 *
                  (mm2 * p1p4 - 3 * mm * np4 * p2p4 +
                   3 * p1p4 * (-p1p2 + p2p4)) +
              me2 * (3 * pow(mm, 3) * np4 * (2 * p1p4 - p2p4) * p2p4 +
                     pow(mm2, 2) * (-2 * pow(p1p4, 2) + pow(p2p4, 2)) +
                     p1p4 * (-4 * pow(p1p4, 3) + 8 * pow(p1p4, 2) * p2p4 +
                             2 * p1p2 * (7 * p1p4 - 3 * p2p4) * p2p4 -
                             11 * p1p4 * pow(p2p4, 2) + 3 * pow(p2p4, 3) +
                             pow(p1p2, 2) * (-10 * p1p4 + 6 * p2p4)) +
                     mm * np4 *
                         (4 * pow(p1p4, 3) - 8 * pow(p1p4, 2) * p2p4 +
                          3 * (p1p2 - p2p4) * pow(p2p4, 2) +
                          p1p4 * p2p4 * (-7 * p1p2 + 11 * p2p4)) +
                     mm2 * (-8 * pow(p1p4, 2) * p2p4 + 3 * pow(p2p4, 3) +
                            p1p2 * (10 * pow(p1p4, 2) - 2 * p1p4 * p2p4 -
                                    3 * pow(p2p4, 2)))) +
              (p1p4 - p2p4) *
                  (-3 * pow(mm, 5) * np4 * p2p4 + pow(mm2, 3) * (p1p4 + p2p4) +
                   pow(mm, 3) * np4 *
                       (-4 * pow(p1p4, 2) + 4 * p1p4 * p2p4 +
                        7 * (p1p2 - p2p4) * p2p4) -
                   4 * p1p4 * (p1p2 - p2p4) *
                       (2 * pow(p1p2, 2) + 2 * pow(p1p4, 2) - 2 * p1p2 * p2p4 -
                        2 * p1p4 * p2p4 + pow(p2p4, 2)) +
                   mm2 * (4 * pow(p1p4, 3) - 4 * pow(p1p4, 2) * p2p4 +
                          5 * p1p4 * pow(p2p4, 2) + 4 * pow(p2p4, 3) +
                          2 * pow(p1p2, 2) * (7 * p1p4 + 2 * p2p4) -
                          2 * p1p2 * p2p4 * (9 * p1p4 + 4 * p2p4)) +
                   pow(mm2, 2) * (5 * p2p4 * (p1p4 + p2p4) -
                                  p1p2 * (7 * p1p4 + 5 * p2p4)) -
                   4 * mm * np4 *
                       (pow(p1p2, 2) * p2p4 +
                        p2p4 * (2 * pow(p1p4, 2) - 2 * p1p4 * p2p4 +
                                pow(p2p4, 2)) -
                        2 * p1p2 *
                            (pow(p1p4, 2) - p1p4 * p2p4 + pow(p2p4, 2)))) +
              mm * np2 *
                  (3 * pow(me2, 2) * pow(p1p4, 2) +
                   (p1p4 - p2p4) * (3 * mm2 - 4 * p1p2 + 4 * p2p4) *
                       (p1p4 * (-2 * p1p2 + p2p4) + mm2 * (p1p4 + p2p4)) +
                   me2 * (mm2 * (-6 * pow(p1p4, 2) + 3 * pow(p2p4, 2)) +
                          p1p4 * (10 * p1p2 * p1p4 - 6 * p1p2 * p2p4 -
                                  7 * p1p4 * p2p4 + 3 * pow(p2p4, 2)))))) /
            (3. * pow(p1p4, 2) * pow(p1p4 - p2p4, 2))) /
           2;
}

} // namespace Mustard::inline Physics::QFT
