// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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
#include "Mustard/Physics/QFT/MatrixElement.h++"

#include "CLHEP/Vector/ThreeVector.h"

#include "fmt/core.h"

#include <array>

namespace Mustard::inline Physics::QFT {

/// @class PolarizedMatrixElement
/// @brief Base for polarized squared matrix element functor
///
/// Extends `MatrixElement` with polarization handling for initial-state particle(s).
/// Provides storage and access methods for initial particle polarization vector(s).
///
/// @tparam M Number of initial-state particles (M ≥ 1)
/// @tparam N Number of final-state particles (N ≥ 1)
template<int M, int N>
class PolarizedMatrixElement : public MatrixElement<M, N> {
public:
    /// @brief Default constructor (all polarizations zero)
    PolarizedMatrixElement() = default;
    /// @brief Construct with initial polarization array
    /// @param pol Array of polarization vectors for each initial particle (all |p| ≤ 1)
    PolarizedMatrixElement(const std::array<CLHEP::Hep3Vector, M>& pol);

    /// @brief Get polarization vector for single initial particle
    /// @param i Particle index (0 ≤ i < M)
    auto InitialStatePolarization(int i) const -> auto { return fInitialStatePolarization.at(i); }
    /// @brief Get all polarization vectors
    auto InitialStatePolarization() const -> const auto& { return fInitialStatePolarization; }

    /// @brief Set polarization for single initial particle
    /// @param i Particle index (0 ≤ i < M)
    /// @param pol Polarization vector (|p| ≤ 1)
    auto InitialStatePolarization(int i, CLHEP::Hep3Vector pol) -> void;
    /// @brief Set all polarization vectors
    /// @param pol Array of polarization vectors for each initial particle (all |p| ≤ 1)
    auto InitialStatePolarization(const std::array<CLHEP::Hep3Vector, M>& pol) -> void;

private:
    std::array<CLHEP::Hep3Vector, M> fInitialStatePolarization; ///< Polarization storage
};

/// @class PolarizedMatrixElement<1, N>
/// @brief Specialization for single initial-state particle
///
/// Simplified interface for decay processes (1→N) with single polarization vector.
///
/// @tparam N Number of final-state particles (N ≥ 1)
template<int N>
class PolarizedMatrixElement<1, N> : public MatrixElement<1, N> {
public:
    /// @brief Default constructor (zero polarization)
    PolarizedMatrixElement() = default;
    /// @brief Construct with polarization vector
    /// @param pol Polarization vector (|p| ≤ 1)
    PolarizedMatrixElement(CLHEP::Hep3Vector pol);

    /// @brief Get polarization vector
    auto InitialStatePolarization() const -> auto { return fInitialStatePolarization; }
    /// @brief Set polarization vector
    /// @param pol Polarization vector (|p| ≤ 1)
    auto InitialStatePolarization(CLHEP::Hep3Vector pol) -> void;

private:
    CLHEP::Hep3Vector fInitialStatePolarization;
};

} // namespace Mustard::inline Physics::QFT

#include "Mustard/Physics/QFT/PolarizedMatrixElement.inl"
