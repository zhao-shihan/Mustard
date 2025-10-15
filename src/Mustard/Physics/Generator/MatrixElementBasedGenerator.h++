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

#include "Mustard/Execution/Executor.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Math/Estimate.h++"
#include "Mustard/Math/MCIntegrationUtility.h++"
#include "Mustard/Parallel/ReseedRandomEngine.h++"
#include "Mustard/Physics/Generator/EventGenerator.h++"
#include "Mustard/Physics/Generator/GENBOD.h++"
#include "Mustard/Physics/QFT/MatrixElement.h++"
#include "Mustard/Physics/QFT/PolarizedMatrixElement.h++"
#include "Mustard/Utility/VectorArithmeticOperator.h++"

#include "CLHEP/Random/Random.h"
#include "CLHEP/Vector/ThreeVector.h"

#include "mplr/mplr.hpp"

#include "muc/array"
#include "muc/chrono"
#include "muc/math"
#include "muc/numeric"
#include "muc/utility"

#include "gsl/gsl"

#include "fmt/core.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <functional>
#include <limits>
#include <numbers>
#include <stdexcept>
#include <tuple>
#include <typeinfo>
#include <utility>

namespace Mustard::inline Physics::inline Generator {

/// @class MatrixElementBasedGenerator
/// @brief Generator based on a matrix element.
///
/// Generates events distributed according to |M|² × acceptance, and
/// weight = 1 / acceptance.
///
/// @tparam M Number of initial-state particles
/// @tparam N Number of final-state particles
/// @tparam A Matrix element of the process to be generated
template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
class MatrixElementBasedGenerator : public EventGenerator<M, N> {
public:
    /// @brief Initial-state 4-momentum (or container type when M>1)
    using typename EventGenerator<M, N>::InitialStateMomenta;
    /// @brief Final-state 4-momentum container type
    using typename EventGenerator<M, N>::FinalStateMomenta;
    /// @brief Generated event type
    using typename EventGenerator<M, N>::Event;
    /// @brief User-defined acceptance function type
    using AcceptanceFunction = std::function<auto(const FinalStateMomenta&)->double>;

public:
    /// @brief Construct event generator
    /// @param pI initial-state 4-momenta
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    MatrixElementBasedGenerator(const InitialStateMomenta& pI, const std::array<int, N>& pdgID, const std::array<double, N>& mass);
    /// @brief Construct event generator
    /// @param pI initial-state 4-momenta
    /// @param polarization Initial-state polarization vector
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    MatrixElementBasedGenerator(const InitialStateMomenta& pI, CLHEP::Hep3Vector polarization,
                                const std::array<int, N>& pdgID, const std::array<double, N>& mass)
        requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>>;
    /// @brief Construct event generator
    /// @param pI initial-state 4-momenta
    /// @param polarization Initial-state polarization vectors
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    /// @note This overload is only enabled for polarized scattering
    MatrixElementBasedGenerator(const InitialStateMomenta& pI, const std::array<CLHEP::Hep3Vector, M>& polarization,
                                const std::array<int, N>& pdgID, const std::array<double, N>& mass)
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1);

    /// @brief Get currently set initial-state 4-momenta
    auto ISMomenta() const -> const auto& { return fISMomenta; }

    /// @brief Compute |M|² × acceptance integral on phase space by Monte Carlo integration.
    /// Useful for calculating total decay width or cross section
    /// @param executor An executor instance
    /// @param precisionGoal Target relative uncertainty (e.g. 0.01 for 1% rel. unc.)
    /// @param integrationState Integration state for continuing integration
    /// @param rng Reference to CLHEP random engine
    /// @return (1) Monte Carlo integration result of |M|² × acceptance integral on phase space
    ///         (2) Effective sample size
    ///         (3) Current integration state
    auto PhaseSpaceIntegral(Executor<unsigned long long>& executor, double precisionGoal,
                            Math::MCIntegrationState integrationState = {},
                            CLHEP::HepRandomEngine& rng = *CLHEP::HepRandom::getTheEngine()) -> std::tuple<Math::Estimate, double, Math::MCIntegrationState>;

protected:
    /// @brief Set initial-state 4-momenta
    /// @param pI initial-state 4-momenta
    auto ISMomenta(const InitialStateMomenta& pI) -> void;

    /// @brief Set final-state PDG IDs
    /// @param pdgID Array of particle PDG IDs
    auto PDGID(const std::array<int, N>& pdgID) -> void { fGENBOD.PDGID(pdgID); }
    /// @brief Set final-state masses
    /// @param mass Array of particle masses
    auto Mass(const std::array<double, N>& mass) -> void { fGENBOD.Mass(mass); }

    /// @brief Generate an event on phase space
    /// @param rng Reference to CLHEP random engine
    /// @return An event from phase space
    auto PhaseSpace(CLHEP::HepRandomEngine& rng) -> auto { return fGENBOD(rng, fISMomenta); }

    /// @brief Get polarization vector
    /// @note This overload is only enabled for polarized decay
    auto InitialStatePolarization() const -> CLHEP::Hep3Vector
        requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>>;
    /// @brief Get polarization vector
    /// @param i Particle index (0 ≤ i < M)
    /// @note This overload is only enabled for polarized scattering
    auto InitialStatePolarization(int i) const -> CLHEP::Hep3Vector
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1);
    /// @brief Get all polarization vectors
    /// @note This overload is only enabled for polarized scattering
    auto InitialStatePolarization() const -> const std::array<CLHEP::Hep3Vector, M>&
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1);

    /// @brief Set polarization vector
    /// @param pol Polarization vector (|pol| ≤ 1)
    /// @note This overload is only enabled for polarized decay
    auto InitialStatePolarization(CLHEP::Hep3Vector pol) -> void
        requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>>;
    /// @brief Set polarization for single initial particle
    /// @param i Particle index (0 ≤ i < M)
    /// @param pol Polarization vector (|pol| ≤ 1)
    /// @note This overload is only enabled for polarized scattering
    auto InitialStatePolarization(int i, CLHEP::Hep3Vector pol) -> void
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1);
    /// @brief Set all polarization vectors
    /// @param pol Array of polarization vectors for each initial particle (all |pol| ≤ 1)
    /// @note This overload is only enabled for polarized scattering
    auto InitialStatePolarization(const std::array<CLHEP::Hep3Vector, M>& pol) -> void
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1);

    /// @brief Set IR cuts for single final-state particle
    /// @param i Particle index (0 ≤ i < N)
    /// @param cut IR cut value
    auto IRCut(int i, double cut) -> void;
    /// @brief Check final-state momenta pass the IR cut
    /// @param pF Final states' 4-momenta
    /// @return true if momenta is IR-safe
    auto IRSafe(const FinalStateMomenta& pF) const -> bool;

    /// @brief Set user-defined acceptance function (0 <= acceptance <= 1 recommended)
    /// (PDF = |M|² × acceptance, weight = 1 / acceptance)
    /// @param B User-defined acceptance
    auto Acceptance(AcceptanceFunction Acceptance) -> void;

    /// @brief Get acceptance with range check
    /// @param pF Final states' 4-momenta
    /// @exception `std::runtime_error` if invalid acceptance value produced
    /// @return B(p1, ..., pN)
    auto ValidAcceptance(const FinalStateMomenta& pF) const -> double;
    /// @brief Get reweighted PDF value with range check
    /// @param pF Final states' 4-momenta
    /// @param event Final states from phase space
    /// @param acceptance Acceptance value at the same phase space point (from ValidAcceptance)
    /// @exception `std::runtime_error` if invalid PDF value produced
    /// @return |M|²(p1, ..., pN) × acceptance(p1, ..., pN) × |J|(p1, ..., pN)
    auto ValidMSqAcceptanceDetJ(const FinalStateMomenta& pF, double acceptance, double detJ) const -> double;

private:
    /// @brief Monte Carlo integration implementation
    auto Integrate(std::regular_invocable<const Event&> auto&& Integrand, double precisionGoal,
                   Math::MCIntegrationState& state, Executor<unsigned long long>& executor, CLHEP::HepRandomEngine& rng) -> std::pair<Math::Estimate, double>;

protected:
    [[no_unique_address]] A fMatrixElement; ///< Matrix element
    GENBOD<M, N> fGENBOD;                   ///< Phase space generator

private:
    InitialStateMomenta fISMomenta;             ///< Initial-state 4-momenta
    CLHEP::Hep3Vector fBoostFromLabToCM;        ///< Boost from lab frame to c.m. frame
    std::vector<std::pair<int, double>> fIRCut; ///< IR cuts
    AcceptanceFunction fAcceptance;             ///< User acceptance function
    mutable std::int8_t fAcceptanceGt1Counter;  ///< Counter of acceptance > 1 warning
    mutable std::int8_t fNegativeMSqCounter;    ///< Counter of negative |M|² warning
};

} // namespace Mustard::inline Physics::inline Generator

#include "Mustard/Physics/Generator/MatrixElementBasedGenerator.inl"
