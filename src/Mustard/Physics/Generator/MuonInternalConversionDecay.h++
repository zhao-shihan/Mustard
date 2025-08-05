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

#include "Mustard/Physics/Generator/MetropolisHastingsGenerator.h++"

#include "CLHEP/Vector/ThreeVector.h"

#include <string_view>

namespace Mustard::inline Physics::inline Generator {

/// @class MuonInternalConversionDecay
/// @brief MCMC generator for mu->evvee decays
class MuonInternalConversionDecay : public MetropolisHastingsGenerator<5> {
public:
    /// @brief Squared amplitude options
    enum struct MSqOption {
        McMule2020, ///< Squared amplitude from McMule legacy code (2020)
        RR2009PRD   ///< Unpolarized squared amplitude from Rashid M. Djilkibaev and Rostislav V. Konoplich (2009)
    };

public:
    /// @brief Construct generator for specific parent
    /// @param parent "mu-" or "mu+" (determines PDG IDs in generated event)
    explicit MuonInternalConversionDecay(std::string_view parent);

    /// @brief Set parent particle type
    /// @param parent "mu-" or "mu+"
    /// @exception std::invalid_argument for invalid parent names
    auto Parent(std::string_view parent) -> void;
    /// @brief Select squared amplitude implementation
    auto MSqOption(enum MSqOption option) -> void;
    /// @brief Select squared amplitude implementation
    /// @param option Same as enum MSqOption
    /// @exception std::invalid_argument for invalid option names
    auto MSqOption(std::string_view option) -> void;
    /// @brief Set initial muon polarization vector
    /// @param pol 3-vector polarization in CMS (|pol| ≤ 1)
    auto Polarization(CLHEP::Hep3Vector pol) -> void;

    /// @brief Calculate squared amplitude
    /// @param momenta Final-state particle momenta
    /// @return |M|² value
    virtual auto SquaredAmplitude(const Momenta& momenta) const -> double override;

private:
    auto MSqMcMule2020(const Momenta& momenta) const -> double;
    auto MSqRR2009PRD(const Momenta& momenta) const -> double;

public:
    enum MSqOption fMSqOption;       ///< Selected squared amplitude implementation
    CLHEP::Hep3Vector fPolarization; ///< Muon polarization vector (in CMS)
};

} // namespace Mustard::inline Physics::inline Generator
