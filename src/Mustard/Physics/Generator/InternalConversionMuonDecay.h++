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

#include "Mustard/Physics/Generator/MultipleTryMetropolisGenerator.h++"
#include "Mustard/Physics/QFT/InternalConversionMuonDecayMSqMcMule.h++"

#include "CLHEP/Vector/ThreeVector.h"

#include <optional>
#include <string_view>

namespace Mustard::inline Physics::inline Generator {

/// @class InternalConversionMuonDecay
/// @brief MCMC generator for mu->ennee decays
/// Kinematics: μ⁻ → e⁻ ν ν e⁺ e⁻
///             μ⁺ → e⁺ ν ν e⁻ e⁺
class InternalConversionMuonDecay : public MultipleTryMetropolisGenerator<1, 5, QFT::InternalConversionMuonDecayMSqMcMule> {
public:
    /// @brief Construct generator for specific parent
    /// @param parent "mu-" or "mu+" (determines PDG IDs in generated event)
    /// @param polarization Muon polarization vector
    /// @param delta Step scale along one direction in random state space (0 < delta < 0.5)
    /// (optional here, but should be set no later than before generation)
    /// @param discard Samples discarded between two events generated from the Markov chain
    /// (optional here, but should be set no later than before generation)
    InternalConversionMuonDecay(std::string_view parent, CLHEP::Hep3Vector polarization,
                                std::optional<double> delta = {}, std::optional<unsigned> discard = {});

    /// @brief Set parent particle
    /// @param parent "mu-" or "mu+"
    /// @exception std::invalid_argument for invalid parent names
    auto Parent(std::string_view parent) -> void;
};

} // namespace Mustard::inline Physics::inline Generator
