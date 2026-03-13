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

#include "Mustard/Execution/Executor.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Math/Estimate.h++"
#include "Mustard/Math/MCIntegrationUtility.h++"
#include "Mustard/Math/Vector.h++"
#include "Mustard/Parallel/ReseedRandomEngine.h++"
#include "Mustard/Physics/Generator/EventGenerator.h++"
#include "Mustard/Physics/Generator/GENBOD.h++"
#include "Mustard/Physics/QFT/MatrixElement.h++"
#include "Mustard/Physics/QFT/PolarizedMatrixElement.h++"
#include "Mustard/Utility/VectorArithmeticOperator.h++"

#include "CLHEP/Random/Random.h"
#include "CLHEP/Units/SystemOfUnits.h"

#include "mplr/mplr.hpp"

#include "muc/array"
#include "muc/chrono"
#include "muc/hash_map"
#include "muc/hash_set"
#include "muc/math"
#include "muc/numeric"
#include "muc/utility"

#include "gsl/gsl"

#include "fmt/std.h"

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
/// Generates events distributed according to 1/S × |M|² × acceptance, and
/// weight = 1 / acceptance. Here S is the final state symmetry factor,
/// |M|² is the matrix element (squared amplitude), acceptance is a user-defined
/// function (normally 0 <= acceptance <= 1) to apply importance sampling,
/// and J is the Jacobian of the phase space transformation (e.g. from GENBOD)
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
    /// @brief User-defined acceptance function type (normally 0 <= acceptance <= 1)
    using AcceptanceFunction = std::function<auto(const FinalStateMomenta&)->double>;

public:
    /// @brief Construct event generator
    /// @param pI initial-state 4-momenta
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    MatrixElementBasedGenerator(const InitialStateMomenta& pI, const std::array<int, N>& pdgID, const std::array<double, N>& mass);
    /// @brief Construct event generator
    /// @param pI initial-state 4-momenta
    /// @param polarization Initial-state polarization vector(s)
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    MatrixElementBasedGenerator(const InitialStateMomenta& pI, const typename A::InitialStatePolarization& polarization,
                                const std::array<int, N>& pdgID, const std::array<double, N>& mass)
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>>;

    /// @brief Get currently set initial-state 4-momenta
    auto Momenta() const -> const auto& { return fMomenta; }

    /// @brief Compute 1/S × |M|² × acceptance integral on phase space by Monte Carlo integration.
    /// Useful for calculating total decay width or cross section
    /// @param executor An executor instance
    /// @param precisionGoal Target relative uncertainty (e.g. 0.01 for 1% rel. unc.)
    /// @param integrationState Integration state for continuing integration
    /// @param rng Reference to CLHEP random engine
    /// @return (1) Monte Carlo integration result of 1/S × |M|² × acceptance integral on phase space
    ///         (2) Effective sample size
    ///         (3) Current integration state
    auto PhaseSpaceIntegral(Executor<unsigned long long>& executor, double precisionGoal,
                            MCIntegrationState integrationState = {},
                            CLHEP::HepRandomEngine& rng = *CLHEP::HepRandom::getTheEngine()) -> std::tuple<Estimate, double, MCIntegrationState>;

protected:
    /// @brief Set initial-state 4-momenta
    /// @param pI initial-state 4-momenta
    auto Momenta(const InitialStateMomenta& pI) -> void;

    /// @brief Get initial-state polarization vector(s)
    /// @note This overload is only enabled for polarized process
    auto Polarization() const -> const typename A::InitialStatePolarization&
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>>;
    /// @brief Get initial-state polarization vector
    /// @param i Particle index (0 ≤ i < M)
    /// @note This overload is only enabled for polarized scattering
    auto Polarization(int i) const -> Vector3D
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1);

    /// @brief Set initial-state polarization vector(s)
    /// @param pol Polarization vector(s) (all |pol| ≤ 1)
    /// @note This overload is only enabled for polarized process
    auto Polarization(const typename A::InitialStatePolarization& pol) -> void
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>>;
    /// @brief Set polarization for single initial-state particle
    /// @param i Particle index (0 ≤ i < M)
    /// @param pol Polarization vector (|pol| ≤ 1)
    /// @note This overload is only enabled for polarized scattering
    auto Polarization(int i, Vector3D pol) -> void
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1);

    /// @brief Set final-state PDG IDs
    /// @param pdgID Array of particle PDG IDs
    auto PDGID(const std::array<int, N>& pdgID) -> void { fGENBOD.PDGID(pdgID); }
    /// @brief Set final-state masses
    /// @param mass Array of particle masses
    auto Mass(const std::array<double, N>& mass) -> void { fGENBOD.Mass(mass); }

    /// @brief Generate an event on phase space
    /// @param rng Reference to CLHEP random engine
    /// @return An event from phase space
    auto PhaseSpace(CLHEP::HepRandomEngine& rng) -> auto { return fGENBOD(rng, fMomenta); }

    /// @brief Add an identical particle index set
    /// @param set A vector of particle indices (0 ≤ index < N)
    auto AddIdenticalSet(std::vector<int> set) -> void;
    /// @brief Get identical particle index sets
    /// @return Identical particle index sets
    auto IdenticalSet() const -> const auto& { return fIdenticalSet; }
    /// @brief Get identical particle index set
    /// @param i Set index (0 ≤ i < number of sets)
    /// @return Identical particle index set
    auto IdenticalSet(int i) const -> const auto& { return fIdenticalSet[i]; }

    /// @brief Set low-energy cutoff for single final-state particle to avoid infrared divergence (if applicable)
    /// @param i Particle index (0 ≤ i < N)
    /// @param cutoff Soft cutoff value (on kinetic energy in the c.m. frame)
    auto SoftCutoff(int i, double cutoff) -> void;
    /// @brief Set collinear cutoff for two particles to avoid infrared divergence (if applicable)
    /// @param pID Pair of particle indices (0 ≤ index < N)
    /// @param cutoff Collinear cutoff value (on angle between the two particles in the c.m. frame, should within (0, π))
    auto CollinearCutoff(std::pair<int, int> pID, double cutoff) -> void;
    /// @brief Check final-state momenta pass the soft cutoff and collinear cutoff (i.e. are IR-safe)
    /// @param pF Final states' 4-momenta
    /// @return true if momenta are IR-safe
    auto InfraredSafe(FinalStateMomenta pF) const -> bool;

    /// @brief Set user-defined acceptance function (normally 0 <= acceptance <= 1)
    /// (PDF = 1/S × |M|² × acceptance, weight = 1 / acceptance)
    /// @param acceptance User-defined acceptance
    auto Acceptance(AcceptanceFunction acceptance) -> void;
    /// @brief Get acceptance with range check
    /// @param pF Final states' 4-momenta
    /// @exception `std::runtime_error` if invalid acceptance value produced
    /// @return acceptance(p1, ..., pN)
    auto Acceptance(const FinalStateMomenta& pF) const -> double;

    /// @brief Get weighted PDF value with range check
    /// @param pF Final states' 4-momenta
    /// @param event Final states from phase space
    /// @param acceptance Acceptance value at the same phase space point (from `Acceptance(const FinalStateMomenta& pF)`)
    /// @exception `std::runtime_error` if invalid PDF value produced
    /// @return 1/S × |M|²(p1, ..., pN) × acceptance(p1, ..., pN) × |J|(p1, ..., pN), where S is
    /// the final state symmetry factor, and J is the Jacobian of the phase space transformation (e.g. from GENBOD)
    auto MSqAcceptanceDetJ(const FinalStateMomenta& pF, double acceptance, double detJ) const -> double;

private:
    /// @brief Monte Carlo integration implementation
    auto Integrate(std::regular_invocable<const Event&> auto&& Integrand, double precisionGoal,
                   MCIntegrationState& state, Executor<unsigned long long>& executor, CLHEP::HepRandomEngine& rng) -> std::pair<Estimate, double>;

protected:
    [[no_unique_address]] A fMatrixElement; ///< Matrix element
    GENBOD<M, N> fGENBOD;                   ///< Phase space generator

private:
    InitialStateMomenta fMomenta;                                     ///< Initial-state 4-momenta
    Vector3D fBoostFromLabToCM;                                       ///< Boost from lab frame to c.m. frame
                                                                      //
    double fFSSymmetryFactor;                                         ///< Final-state identical particle symmetry factor
    std::vector<std::vector<int>> fIdenticalSet;                      ///< Identical particle sets
                                                                      //
    muc::flat_hash_map<int, double> fSoftCutoff;                      ///< Soft cutoffs (on kinetic energies in the c.m. frame)
    muc::flat_hash_map<std::pair<int, int>, double> fCollinearCutoff; ///< Collinear cutoffs (on cosine of angles between particles in the c.m. frame)
    muc::flat_hash_set<int> fInfraredUnsafePID;                       ///< Particle indices involved in IR cutoffs
                                                                      //
    AcceptanceFunction fAcceptance;                                   ///< User acceptance function (normally 0 <= acceptance <= 1)
                                                                      //
    mutable std::int8_t fAcceptanceGt1Counter;                        ///< Counter of acceptance > 1 warning
    mutable std::int8_t fNegativeMSqCounter;                          ///< Counter of negative |M|² warning
};

} // namespace Mustard::inline Physics::inline Generator

#include "Mustard/Physics/Generator/MatrixElementBasedGenerator.inl"
