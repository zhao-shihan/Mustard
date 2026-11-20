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

#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Math/StatisticalTest.h++"
#include "Mustard/Testing/TestEstimate/TestEstimateCommon.h++"

#include "muc/numeric"

#include <cmath>
#include <string>

namespace Mustard::Testing::TestStatisticalTest {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

inline auto CheckBetween(double actual, double lo, double hi, const std::string& ctx) -> void {
    if (not(lo <= actual and actual <= hi)) {
        Throw<std::runtime_error>(fmt::format("\n"
                                              "{}: expected in [{}, {}], got {}",
                                              ctx, lo, hi, actual));
    }
}

inline auto CheckNonNegative(double actual, const std::string& ctx) -> void {
    if (actual < 0) {
        Throw<std::runtime_error>(fmt::format("\n"
                                              "{}: expected non-negative, got {}",
                                              ctx, actual));
    }
}

template<typename F>
auto CheckThrows(const std::string& ctx, F&& f) -> void {
    auto threw{false};
    try {
        f();
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    if (not threw) {
        Throw<std::runtime_error>(fmt::format("\n"
                                              "{}: expected std::invalid_argument, none thrown",
                                              ctx));
    }
}

} // namespace Mustard::Testing::TestStatisticalTest
