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

#include "Mustard/Physics/Amplitude/DoubleRadiativeMuonDecayMSqMcMule.h++"
#include "Mustard/Physics/Generator/MTMGenerator.h++"

#include "CLHEP/Vector/ThreeVector.h"

#include <functional>
#include <string_view>

namespace Mustard::inline Physics::inline Generator {

/// @class DoubleRadiativeMuonDecay
/// @brief MCMC generator for mu->enngg decays
/// Kinematics: mu- -> e- nu nu gamma gamma
///             mu+ -> e+ nu nu gamma gamma
/// @warning IR safety is not automatically guaranteed.
/// Always set bias function to ensure IR-safe generation.
class DoubleRadiativeMuonDecay : public MTMGenerator<1, 5, DoubleRadiativeMuonDecayMSqMcMule> {
public:
    /// @brief Construct generator for specific parent
    /// @param parent "mu-" or "mu+" (determines PDG IDs in generated event)
    /// @param polarization Muon polarization vector
    /// @param irCut IR cut for final-state photons
    /// @param delta Step scale along one direction in random state space (0 < delta < 0.5)
    /// @param discard Samples discarded between two events generated in the Markov chain
    DoubleRadiativeMuonDecay(std::string_view parent, CLHEP::Hep3Vector polarization, double irCut,
                             double delta, int discard);

    /// @brief Set parent particle type
    /// @param parent "mu-" or "mu+"
    /// @exception std::invalid_argument for invalid parent names
    auto Parent(std::string_view parent) -> void;
    /// @brief Set IR cut for final-state photons
    /// @param irCut IR cut for final-state photons
    auto IRCut(double irCut) -> void;
};

} // namespace Mustard::inline Physics::inline Generator
