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

#pragma once

#include "muc/array"

namespace Mustard::Math {

/// @brief Monte Carlo integration internal state
struct MCIntegrationState {
    muc::array2d sum;     ///< Sum of integrand and squared integrand
    unsigned long long n; ///< Sample size
};

/// @brief General integration result
struct IntegrationResult {
    double value;       ///< Integration result
    double uncertainty; ///< Integration uncertainty
};

/// @brief Monte Carlo Integration result
struct MCIntegrationResult {
    double value;       ///< Integration result
    double uncertainty; ///< Integration uncertainty
    double nEff;        ///< Effective sample size

    /// @brief `MCIntegrationResult` is convertible to `IntegrationResult`
    operator IntegrationResult() { return {value, uncertainty}; }
};

} // namespace Mustard::Math
