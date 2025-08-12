// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
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

#include "fmt/core.h"

#include <array>
#include <stdexcept>

namespace Mustard::inline Physics::inline Generator {

/// @class VersatileEventGenerator
/// @brief Base class for N-particle event generators with mutable final states.
///
/// @tparam N Number of particles in final state (N ≥ 2)
/// @tparam M Dimension of random state (default=AnyRandomStateDim)
///
/// @requires
/// - N ≥ 2 (minimum two particles)
/// - M ≥ 3N - 4 (sufficient random dimensions for phase space)
template<int N, int M = internal::AnyRandomStateDim>
class VersatileEventGenerator : public EventGenerator<N, M> {
public:
    /// @brief Construct event generator
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    constexpr VersatileEventGenerator(const std::array<int, N>& pdgID, const std::array<double, N>& mass);

    constexpr auto PDGID() const -> const auto& { return fPDGID; }
    constexpr auto PDGID(int i) const -> auto { return fPDGID[i]; }
    constexpr auto Mass() const -> const auto& { return fMass; }
    constexpr auto Mass(int i) const -> auto { return fMass[i]; }

    constexpr auto PDGID(const std::array<int, N>& pdgID) -> void { fPDGID = pdgID; }
    constexpr auto PDGID(int i, int pdgID) -> void { fPDGID[i] = pdgID; }
    constexpr auto Mass(const std::array<double, N>& mass) -> void { fMass = mass; }
    constexpr auto Mass(int i, double mass) -> void { fMass[i] = mass; }

protected:
    /// @brief Check if center-of-mass energy is sufficient
    /// @param cmsE Center-of-mass energy
    /// @exception std::domain_error if center-of-mass energy is insufficient
    MUSTARD_ALWAYS_INLINE auto CheckCMSEnergy(double cmsE) const -> void;

protected:
    std::array<int, N> fPDGID;   ///< Final state PDG IDs
    std::array<double, N> fMass; ///< Final state rest masses
    double fSumMass;             ///< Sum of final state rest masses
};

} // namespace Mustard::inline Physics::inline Generator

#include "Mustard/Physics/Generator/VersatileEventGenerator.inl"
