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

#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Physics/Generator/EventGenerator.h++"
#include "Mustard/Utility/InlineMacro.h++"

#include "muc/numeric"

#include "fmt/ranges.h"

#include <algorithm>
#include <array>
#include <source_location>
#include <stdexcept>

namespace Mustard::inline Physics::inline Generator {

/// @class VersatileEventGenerator
/// @brief Base class for N-particle event generators with mutable final states.
///
/// @tparam M Number of initial-state particles (M ≥ 1)
/// @tparam N Number of final-state particles (N ≥ 1)
/// @tparam D Dimension of random state (default or D ≥ Dim(phase space))
template<int M, int N, int D = -1>
class VersatileEventGenerator : public EventGenerator<M, N, D> {
public:
    /// @brief Construct event generator
    /// @param pdgID Array of final-state particle PDG IDs (index order preserved)
    /// @param mass Array of final-state particle masses (index order preserved)
    constexpr VersatileEventGenerator(const std::array<int, N>& pdgID, const std::array<double, N>& mass);

    constexpr auto PDGID() const -> const auto& { return fPDGID; }
    constexpr auto PDGID(int i) const -> auto { return fPDGID.at(i); }
    constexpr auto Mass() const -> const auto& { return fMass; }
    constexpr auto Mass(int i) const -> auto { return fMass.at(i); }

    constexpr auto PDGID(const std::array<int, N>& pdgID) -> void { fPDGID = pdgID; }
    constexpr auto PDGID(int i, int pdgID) -> void { fPDGID.at(i) = pdgID; }
    constexpr auto Mass(const std::array<double, N>& mass) -> void;
    constexpr auto Mass(int i, double mass) -> void;

protected:
    /// @brief Check if center-of-mass energy is sufficient
    /// @param cmsE Center-of-mass energy
    /// @exception std::domain_error if center-of-mass energy is insufficient
    MUSTARD_ALWAYS_INLINE auto CheckCMSEnergy(double cmsE, const std::source_location& location = std::source_location::current()) const -> void;

protected:
    std::array<int, N> fPDGID;   ///< Final-state PDG IDs
    std::array<double, N> fMass; ///< Final-state rest masses
    double fSumMass;             ///< Sum of final-state rest masses
};

} // namespace Mustard::inline Physics::inline Generator

#include "Mustard/Physics/Generator/VersatileEventGenerator.inl"
