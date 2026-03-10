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
#include "Mustard/Math/Vector.h++"
#include "Mustard/Physics/QFT/MatrixElement.h++"

#include "fmt/core.h"

#include <array>

namespace Mustard::inline Physics::QFT {

/// @class PolarizedMatrixElement
/// @brief Base for polarized squared matrix element functor
///
/// Extends `MatrixElement` with polarization handling for initial-state particle(s).
/// Provides storage and access methods for initial-state particle polarization vector(s).
///
/// @tparam M Number of initial-state particles (M ≥ 1)
/// @tparam N Number of final-state particles (N ≥ 1)
template<int M, int N>
class PolarizedMatrixElement : public MatrixElement<M, N> {
public:
    /// @brief Initial state polarization vector(s) type
    using typename MatrixElement<M, N>::InitialStatePolarization;

public:
    /// @brief Default constructor (all polarizations zero)
    PolarizedMatrixElement() = default;
    /// @brief Construct with initial-state polarization vectors
    /// @param pol Array of polarization vectors for each initial-state particle (all |p| ≤ 1)
    PolarizedMatrixElement(const InitialStatePolarization& pol);

    /// @brief Get initial-state polarization vector(s)
    /// @return Initial-state polarization vector(s)
    auto Polarization() const -> const auto& { return fPolarization; }
    /// @brief Get polarization vector for single initial-state particle
    /// @param i Particle index (0 ≤ i < M)
    /// @return Polarization vector
    /// @note This overload is only enabled for polarized scattering (M > 1)
    auto Polarization(int i) const -> auto
        requires(M > 1) { return fPolarization.at(i); }

    /// @brief Set initial-state polarization vector(s)
    /// @param pol Initial-state polarization vector(s) (all |p| ≤ 1)
    auto Polarization(const InitialStatePolarization& pol) -> void;
    /// @brief Set polarization for single initial-state particle
    /// @param i Particle index (0 ≤ i < M)
    /// @param pol Polarization vector (|p| ≤ 1)
    /// @note This overload is only enabled for polarized scattering (M > 1)
    auto Polarization(int i, Vector3D pol) -> void
        requires(M > 1);

private:
    InitialStatePolarization fPolarization; ///< Polarization storage
};

} // namespace Mustard::inline Physics::QFT

#include "Mustard/Physics/QFT/PolarizedMatrixElement.inl"
