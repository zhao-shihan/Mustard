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

#include "CLHEP/Vector/LorentzVector.h"

#include <array>
#include <type_traits>

namespace Mustard::inline Physics::inline Amplitude {

/// @class SquaredAmplitude
/// @brief Abstract base for squared matrix element functor
///
/// Provides a unified interface for calculating |M|² for scattering/decay processes.
///
/// @tparam M Number of initial-state particles (M ≥ 1)
/// @tparam N Number of final-state particles (N ≥ 1)
template<int M, int N>
    requires(M >= 1 and N >= 1)
class SquaredAmplitude {
public:
    /// @brief Initial state momentum(a) type
    using InitialStateMomenta = std::conditional_t<M == 1, CLHEP::HepLorentzVector,
                                                   std::array<CLHEP::HepLorentzVector, M>>;
    /// @brief Final state momenta type
    using FinalStateMomenta = std::array<CLHEP::HepLorentzVector, N>;

public:
    virtual ~SquaredAmplitude() = default;

    /// @brief Calculate squared matrix element |M|²
    /// @param pI Initial-state 4-momenta
    /// @param pF Final-state 4-momenta
    /// @return |M|² value
    virtual auto operator()(const InitialStateMomenta& pI, const FinalStateMomenta& pF) const -> double = 0;
};

} // namespace Mustard::inline Physics::inline Amplitude
