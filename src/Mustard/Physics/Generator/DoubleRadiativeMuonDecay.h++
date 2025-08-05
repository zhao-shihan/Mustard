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

#include <functional>
#include <string_view>

namespace Mustard::inline Physics::inline Generator {

/// @class DoubleRadiativeMuonDecay
/// @brief MCMC generator for mu->evvgg decays
/// @warning IR safety is not automatically guaranteed.
/// Always set bias function to ensure IR-safe generation.
class DoubleRadiativeMuonDecay : public MetropolisHastingsGenerator<5> {
public:
    /// @brief Construct generator for specific parent
    /// @param parent "mu-" or "mu+" (determines PDG IDs in generated event)
    /// @param B User-defined bias (should always include IR cut)
    DoubleRadiativeMuonDecay(std::string_view parent, std::function<auto(const Momenta&)->double> B);

    /// @brief Set parent particle type
    /// @param parent "mu-" or "mu+"
    /// @exception std::invalid_argument for invalid parent names
    auto Parent(std::string_view parent) -> void;
    /// @brief Set initial muon polarization vector
    /// @param pol 3-vector polarization in CMS (|pol| ≤ 1)
    auto Polarization(CLHEP::Hep3Vector pol) -> void;

    /// @brief Calculate squared amplitude
    /// @param momenta Final-state particle momenta
    /// @return |M|² value
    virtual auto SquaredAmplitude(const Momenta& momenta) const -> double override;

private:
    static auto MSqUnpolarized(double mm2, double me2, double s12, double s15, double s16, double s25, double s26, double s56,
                               double den1, double den2, double den3, double den4, double den5, double den6) -> double;
    static auto MSqPolarizedS2n(double mm2, double me2, double s12, double s15, double s16, double s25, double s26, double s56,
                                double den1, double den2, double den3, double den4, double den5, double den6) -> double;
    static auto MSqPolarizedS5n(double mm2, double me2, double s12, double s15, double s16, double s25, double s26, double s56,
                                double den1, double den2, double den3, double den4, double den5, double den6) -> double;
    static auto MSqPolarizedS6n(double mm2, double me2, double s12, double s15, double s16, double s25, double s26, double s56,
                                double den1, double den2, double den3, double den4, double den5, double den6) -> double;

private:
    CLHEP::Hep3Vector fPolarization; ///< Muon polarization vector (in CMS)
};

} // namespace Mustard::inline Physics::inline Generator
