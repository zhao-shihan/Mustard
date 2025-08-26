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
#include "Mustard/Math/IntegrationResult.h++"
#include "Mustard/Math/MCIntegrationState.h++"
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

#include <array>
#include <cmath>
#include <concepts>
#include <functional>
#include <limits>
#include <typeinfo>
#include <utility>

namespace Mustard::inline Physics::inline Generator {

/// @class MatrixElementBasedGenerator
/// @brief Generator based on a matrix element.
///
/// Generates events distributed according to |M|² × bias, and
/// weight = 1 / bias.
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
    /// @brief User-defined bias function type
    using BiasFunction = std::function<auto(const FinalStateMomenta&)->double>;

public:
    /// @brief Construct event generator
    /// @param cmsE Center-of-mass energy
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    MatrixElementBasedGenerator(double cmsE, const std::array<int, N>& pdgID, const std::array<double, N>& mass);
    /// @brief Construct event generator
    /// @param cmsE Center-of-mass energy
    /// @param polarization Initial-state polarization vector
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    MatrixElementBasedGenerator(double cmsE, CLHEP::Hep3Vector polarization,
                                const std::array<int, N>& pdgID, const std::array<double, N>& mass)
        requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>>;
    /// @brief Construct event generator
    /// @param cmsE Center-of-mass energy
    /// @param polarization Initial-state polarization vectors
    /// @param pdgID Array of particle PDG IDs (index order preserved)
    /// @param mass Array of particle masses (index order preserved)
    /// @note This overload is only enabled for polarized scattering
    MatrixElementBasedGenerator(double cmsE, const std::array<CLHEP::Hep3Vector, M>& polarization,
                                const std::array<int, N>& pdgID, const std::array<double, N>& mass)
        requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1);

    /// @brief Get currently set center-of-mass frame energy
    auto CMSEnergy() const -> auto { return fCMSEnergy; }

    /// @brief |M|² × bias integral divided by |M|² integral
    /// Multiply event weights with the result to normalize weights to the number of generated events.
    /// Essential for calculating total cross-section or decay width when bias function is set
    /// @param executor An executor instance
    /// @param precisionGoal Target relative uncertainty (e.g. 0.01 for 1% rel. unc.)
    /// @param integrationState Integration state for continuing normalization
    /// @param rng Reference to CLHEP random engine
    /// @return Estimated normalization factor and integration state
    auto EstimateNormalizationFactor(Executor<unsigned long long>& executor, double precisionGoal,
                                     std::array<Math::MCIntegrationState, 2> integrationState = {},
                                     CLHEP::HepRandomEngine& rng = *CLHEP::HepRandom::getTheEngine()) -> std::pair<Math::IntegrationResult, std::array<Math::MCIntegrationState, 2>>;

protected:
    /// @brief Set center-of-mass energy
    /// @param cmsE Center-of-mass energy
    auto CMSEnergy(double cmsE) -> void;
    /// @brief Check whether initial state momentum passed to generator matches
    /// currently set CMS energy
    /// @param pI Initial-state 4-momenta passed to generator
    auto CheckCMSEnergyMatch(const InitialStateMomenta& pI) const -> void;

    /// @brief Set particle PDG IDs
    /// @param pdgID Array of particle PDG IDs
    auto PDGID(const std::array<int, N>& pdgID) -> void { fGENBOD.PDGID(pdgID); }
    /// @brief Set particle masses
    /// @param mass Array of particle masses
    auto Mass(const std::array<double, N>& mass) -> void { fGENBOD.Mass(mass); }

    /// @brief Generate an event on phase space
    /// @param rng Reference to CLHEP random engine
    /// @return An event from phase space
    auto PhaseSpace(CLHEP::HepRandomEngine& rng) -> auto { return fGENBOD(rng, {fCMSEnergy, {}}); }

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

    /// @brief Set IR cuts for single final state particle
    /// @param i Particle index (0 ≤ i < N)
    /// @param cut IR cut value
    auto IRCut(int i, double cut) -> void;
    /// @brief Check final-state momenta pass the IR cut
    /// @param pF Final states' 4-momenta
    /// @return true if momenta is IR-safe
    auto IRSafe(const FinalStateMomenta& pF) const -> bool;

    /// @brief Set user-defined bias function in PDF (PDF = |M|² × bias)
    /// @param B User-defined bias
    auto Bias(BiasFunction B) -> void { fBias = std::move(B); }

    /// @brief Get bias with range check
    /// @param pF Final states' 4-momenta
    /// @exception `std::runtime_error` if invalid bias value produced
    /// @return B(p1, ..., pN)
    auto ValidBias(const FinalStateMomenta& pF) const -> double;
    /// @brief Get reweighted PDF value with range check
    /// @param pF Final states' 4-momenta
    /// @param event Final states from phase space
    /// @param bias Bias value at the same phase space point (from BiasWithCheck)
    /// @exception `std::runtime_error` if invalid PDF value produced
    /// @return |M|²(p1, ..., pN) × bias(p1, ..., pN) × |J|(p1, ..., pN)
    auto ValidBiasedMSqDetJ(const FinalStateMomenta& pF, double bias, double detJ) const -> double;

private:
    /// @brief Monte Carlo integration implementation
    auto Integrate(std::regular_invocable<const Event&> auto&& Integrand, double precisionGoal,
                   Math::MCIntegrationState& state, Executor<unsigned long long>& executor, CLHEP::HepRandomEngine& rng) -> Math::IntegrationResult;

protected:
    GENBOD<M, N> fGENBOD; ///< Phase space generator

private:
    double fCMSEnergy;                          ///< Currently set CM energy
    [[no_unique_address]] A fMatrixElement;     ///< Matrix element
    std::vector<std::pair<int, double>> fIRCut; ///< IR cuts
    BiasFunction fBias;                         ///< User bias function
};

} // namespace Mustard::inline Physics::inline Generator

#include "Mustard/Physics/Generator/MatrixElementBasedGenerator.inl"
