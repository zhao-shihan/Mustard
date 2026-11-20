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

#include "Mustard/Math/StatisticalTest.h++"

#include "Math/ProbFuncMathCore.h"
#include "Math/QuantFuncMathCore.h"

namespace Mustard::inline Math {

// ============================================================================
// Statistical test base class
// ============================================================================

auto StatisticalTest::CheckPValue(double p) -> void {
    if (p < 0 or p > 1) {
        Throw<std::domain_error>(fmt::format("Invalid p-value: {}", p));
    }
}

auto StatisticalTest::PValueToSignificance(double p) -> double {
    CheckPValue(p);
    return ROOT::Math::normal_quantile_c(p, 1);
}

// ===========================================================================
// Welch's t-test
// ===========================================================================

auto StatisticalTTest::LeftTailP() const -> double {
    return ROOT::Math::tdistribution_cdf(fT, fNDF);
}

auto StatisticalTTest::RightTailP() const -> double {
    return ROOT::Math::tdistribution_cdf_c(fT, fNDF);
}

auto StatisticalTTest::TwoSidedP() const -> double {
    return 2 * ROOT::Math::tdistribution_cdf_c(std::abs(fT), fNDF);
}

// ===========================================================================
// Z-test
// ===========================================================================

auto StatisticalZTest::LeftTailP() const -> double {
    return ROOT::Math::normal_cdf(fZ);
}

auto StatisticalZTest::RightTailP() const -> double {
    return ROOT::Math::normal_cdf_c(fZ);
}

auto StatisticalZTest::TwoSidedP() const -> double {
    return 2 * ROOT::Math::normal_cdf_c(std::abs(fZ));
}

// ===========================================================================
// T-squared test
// ===========================================================================

auto StatisticalTSquaredTest::PValue() const -> double {
    // (f - p + 1) / (f * p) T^2 ~ F(p, f - p + 1)
    const auto denomNDF{fNDF2 - fNDF1 + 1};
    const auto statF{denomNDF * fTSq / (fNDF1 * fNDF2)};
    return ROOT::Math::fdistribution_cdf_c(statF, fNDF1, denomNDF);
}

// ===========================================================================
// Chi-squared test
// ===========================================================================

auto StatisticalChiSquaredTest::PValue() const -> double {
    return ROOT::Math::chisquared_cdf_c(fChiSq, fNDF);
}

} // namespace Mustard::inline Math
