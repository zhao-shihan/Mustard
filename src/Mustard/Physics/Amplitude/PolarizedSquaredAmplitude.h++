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
#include "Mustard/Physics/Amplitude/SquaredAmplitude.h++"

#include "CLHEP/Vector/ThreeVector.h"

#include "fmt/core.h"

#include <array>

namespace Mustard::inline Physics::inline Amplitude {

/// @class PolarizedSquaredAmplitude
/// @brief Base for polarized squared matrix element functor
///
/// Extends `SquaredAmplitude` with polarization handling for initial-state particle(s).
/// Provides storage and access methods for initial particle polarization vector(s).
///
/// @tparam M Number of initial-state particles (M ≥ 1)
/// @tparam N Number of final-state particles (N ≥ 1)
template<int M, int N>
class PolarizedSquaredAmplitude : public SquaredAmplitude<M, N> {
public:
    /// @brief Default constructor (all polarizations zero)
    PolarizedSquaredAmplitude() = default;
    /// @brief Construct with initial polarization array
    /// @param polarization Array of polarization vectors for each initial particle (all |p| ≤ 1)
    PolarizedSquaredAmplitude(const std::array<CLHEP::Hep3Vector, M>& polarization);

    /// @brief Get polarization vector for single initial particle
    /// @param i Particle index (0 ≤ i < M)
    auto InitialStatePolarization(int i) const -> auto { return fInitialStatePolarization.at(i); }
    /// @brief Get all polarization vectors
    auto InitialStatePolarization() const -> const auto& { return fInitialStatePolarization; }

    /// @brief Set polarization for single initial particle
    /// @param i Particle index (0 ≤ i < M)
    /// @param polarization Polarization vector (|p| ≤ 1)
    auto InitialStatePolarization(int i, CLHEP::Hep3Vector polarization) -> void;
    /// @brief Set all polarization vectors
    /// @param polarization Array of polarization vectors for each initial particle (all |p| ≤ 1)
    auto InitialStatePolarization(const std::array<CLHEP::Hep3Vector, M>& polarization) -> void;

private:
    std::array<CLHEP::Hep3Vector, M> fInitialStatePolarization; ///< Polarization storage
};

/// @class PolarizedSquaredAmplitude<1, N>
/// @brief Specialization for single initial-state particle
///
/// Simplified interface for decay processes (1→N) with single polarization vector.
///
/// @tparam N Number of final-state particles (N ≥ 1)
template<int N>
class PolarizedSquaredAmplitude<1, N> : public SquaredAmplitude<1, N> {
public:
    /// @brief Default constructor (zero polarization)
    PolarizedSquaredAmplitude() = default;
    /// @brief Construct with polarization vector
    /// @param polarization Polarization vector (|p| ≤ 1)
    PolarizedSquaredAmplitude(CLHEP::Hep3Vector polarization);

    /// @brief Get polarization vector
    auto InitialStatePolarization() const -> auto { return fInitialStatePolarization; }
    /// @brief Set polarization vector
    /// @param p Polarization vector (|p| ≤ 1)
    auto InitialStatePolarization(CLHEP::Hep3Vector polarization) -> void;

private:
    CLHEP::Hep3Vector fInitialStatePolarization;
};

} // namespace Mustard::inline Physics::inline Amplitude

#include "Mustard/Physics/Amplitude/PolarizedSquaredAmplitude.inl"
