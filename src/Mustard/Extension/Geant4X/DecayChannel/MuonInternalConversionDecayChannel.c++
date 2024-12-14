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

#include "Mustard/Extension/Geant4X/DecayChannel/MuonInternalConversionDecayChannel.h++"
#include "Mustard/Extension/MPIX/DataType.h++"
#include "Mustard/Extension/MPIX/Execution/Executor.h++"
#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Utility/PhysicalConstant.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "G4DecayProducts.hh"
#include "G4DynamicParticle.hh"
#include "Randomize.hh"

#include "mpi.h"

#include "muc/math"

#include "gsl/gsl"

#include "fmt/format.h"

#include <bit>
#include <limits>
#include <stdexcept>

namespace Mustard::inline Extension::Geant4X::inline DecayChannel {

using namespace PhysicalConstant;

MuonInternalConversionDecayChannel::MuonInternalConversionDecayChannel(const G4String& parentName, G4double br, G4int verbose) : // clang-format off
    G4VDecayChannel{"MuonICDecay", verbose}, // clang-format on
    fMetropolisDelta{0.05},
    fMetropolisDiscard{100},
    fBias{[](auto&&) { return 1; }},
    fRAMBO{muon_mass_c2, {electron_mass_c2, electron_mass_c2, electron_mass_c2, 0, 0}},
    fReady{},
    fRawState{},
    fEvent{},
    fBiasedM2{},
    fXoshiro256Plus{},
    fReseedCounter{},
    fMessengerRegister{this} {
    SetParent(parentName);
    SetBR(br);
    SetNumberOfDaughters(5);
    if (parentName == "mu+") {
        SetDaughter(0, "e+");
        SetDaughter(1, "e-");
        SetDaughter(2, "e+");
        SetDaughter(3, "anti_nu_mu");
        SetDaughter(4, "nu_e");
    } else if (parentName == "mu-") {
        SetDaughter(0, "e-");
        SetDaughter(1, "e+");
        SetDaughter(2, "e-");
        SetDaughter(3, "nu_mu");
        SetDaughter(4, "anti_nu_e");
    } else {
#ifdef G4VERBOSE
        if (GetVerboseLevel() > 0) {
            G4cout << "MuonInternalConversionDecayChannel::(Constructor) says\n"
                      "\tParent particle is not mu+ or mu- but "
                   << parentName << G4endl;
        }
#endif
    }
}

auto MuonInternalConversionDecayChannel::Bias(std::function<auto(const CLHEPX::RAMBO<5>::State&)->double> b) -> void {
    fBias = std::move(b);
    fReady = false;
}

auto MuonInternalConversionDecayChannel::Initialize() -> void {
    if (fReady) { return; }
    // initialize
    while (true) {
        std::ranges::generate(fRawState, [this] { return Math::Random::Uniform<double>{}(fXoshiro256Plus); });
        fEvent = fRAMBO(fRawState);
        if (const auto bias{BiasWithCheck(fEvent.state)};
            bias >= std::numeric_limits<double>::min()) {
            fBiasedM2 = bias * UnbiasedM2(fEvent);
            break;
        }
    }
    // thermalize
    constexpr long double deltaSA0{0.1};
    constexpr auto nSA{100000};
    for (auto deltaSA{deltaSA0}; deltaSA > std::numeric_limits<double>::epsilon(); deltaSA -= deltaSA0 / nSA) {
        UpdateState(deltaSA);
    }
    fReady = true;
}

auto MuonInternalConversionDecayChannel::EstimateBiasScale(unsigned long long n) -> std::tuple<double, double, double> {
    if (n == 0) {
        return {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), 0};
    }

    // store state
    auto originalBias{std::move(fBias)};
    auto originalReady{std::move(fReady)};
    auto originalRawState{std::move(fRawState)};
    auto originalEvent{std::move(fEvent)};
    auto originalBiasedM2{std::move(fBiasedM2)};

    // --- above is protected ---

    Bias([](auto&&) { return 1; }); // to evaluate the bias scale of user-defined bias, temporarily switch to unbiased function
    Initialize();

    long double biasScale{};
    long double biasScaleError{};
    {
        const auto partialSumThreshold{muc::llround(std::sqrt(n / Env::MPIEnv::Instance().CommWorldSize()))};
        long double biasPartialSum{};  // improve numeric stability
        long double bias2PartialSum{}; // improve numeric stability
        MPIX::Executor<unsigned long long>{"Estimation", "Sample"}
            .Execute(n, [&](auto i) {
                MainSamplingLoop();
                const auto bias{originalBias(fEvent.state)};
                biasPartialSum += bias;
                bias2PartialSum += muc::pow<2>(bias);
                if ((i + 1) % partialSumThreshold == 0) {
                    biasScale += biasPartialSum;
                    biasScaleError += bias2PartialSum;
                    biasPartialSum = 0;
                    bias2PartialSum = 0;
                }
            });
        biasScale += biasPartialSum;
        biasScaleError += bias2PartialSum;
    }
    MPI_Allreduce(MPI_IN_PLACE, &biasScale, 1, MPIX::DataType(biasScale), MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(MPI_IN_PLACE, &biasScaleError, 1, MPIX::DataType(biasScaleError), MPI_SUM, MPI_COMM_WORLD);
    const auto nEff{muc::pow<2>(biasScale) / biasScaleError};
    biasScale /= n;
    biasScaleError = std::sqrt(biasScaleError) / n;

    // --- below is protected ---

    // restore state
    fBias = std::move(originalBias);
    fReady = std::move(originalReady);
    fRawState = std::move(originalRawState);
    fEvent = std::move(originalEvent);
    fBiasedM2 = std::move(originalBiasedM2);

    return {biasScale, biasScaleError, nEff};
}

auto MuonInternalConversionDecayChannel::DecayIt(G4double) -> G4DecayProducts* {
#ifdef G4VERBOSE
    if (GetVerboseLevel() > 1) {
        G4cout << "MuonInternalConversionDecayChannel::DecayIt ";
    }
#endif

    CheckAndFillParent();
    CheckAndFillDaughters();
    MainSamplingLoop();

    // clang-format off
    auto products{new G4DecayProducts{G4DynamicParticle{G4MT_parent, {}, 0}}}; // clang-format on
    for (int i{}; i < 5; ++i) {
        products->PushProducts(new G4DynamicParticle{G4MT_daughters[i], fEvent.state[i]});
    }

#ifdef G4VERBOSE
    if (GetVerboseLevel() > 1) {
        G4cout << "MuonInternalConversionDecayChannel::DecayIt\n"
                  "\tCreate decay products in rest frame."
               << G4endl;
        products->DumpInfo();
    }
#endif
    return products;
}

auto MuonInternalConversionDecayChannel::BiasWithCheck(const CLHEPX::RAMBO<5>::State& state) const -> double {
    const auto bias{fBias(state)};
    if (bias < 0) {
        Throw<std::runtime_error>("Bias should be non-negative");
    }
    return bias;
}

auto MuonInternalConversionDecayChannel::UpdateState(double delta) -> void {
    decltype(fRawState) newRawState;
    decltype(fEvent) newEvent;
    while (true) {
        std::ranges::transform(std::as_const(fRawState), newRawState.begin(),
                               [&](auto u) {
                                   return Math::Random::Distribution::UniformCompact{
                                       muc::clamp<"()">(u - delta, 0., 1.),
                                       muc::clamp<"()">(u + delta, 0., 1.)}(fXoshiro256Plus);
                               });
        newEvent = fRAMBO(newRawState);
        const auto bias{BiasWithCheck(newEvent.state)};
        if (bias <= std::numeric_limits<double>::min()) {
            continue;
        }

        const auto newBiasedM2{bias * UnbiasedM2(newEvent)};
        if (newBiasedM2 >= fBiasedM2 or
            newBiasedM2 >= fBiasedM2 * Math::Random::Distribution::Uniform<double>{}(fXoshiro256Plus)) {
            fRawState = newRawState;
            fEvent = newEvent;
            fBiasedM2 = newBiasedM2;
            fWeight = 1 / bias;
            return;
        }
    }
}

auto MuonInternalConversionDecayChannel::MainSamplingLoop() -> void {
    if (fReseedCounter++ == 0) {
        static_assert(sizeof(Math::Random::SplitMix64::SeedType) % sizeof(unsigned int) == 0);
        std::array<unsigned int, sizeof(Math::Random::SplitMix64::SeedType) / sizeof(unsigned int)> seed;
        std::ranges::generate(seed, [&rng = *G4Random::getTheEngine()] { return rng.operator unsigned int(); });
        fXoshiro256Plus.Seed(std::bit_cast<Math::Random::SplitMix64::SeedType>(seed));
    }
    Initialize();
    for (int i{}; i < fMetropolisDiscard; ++i) {
        UpdateState(fMetropolisDelta);
    }
    UpdateState(fMetropolisDelta);
}

auto MuonInternalConversionDecayChannel::UnbiasedM2(const CLHEPX::RAMBO<5>::Event& event) -> double {
    // Tree level mu -> eeevv (2 diagrams)

    const auto& [p, p1, p2, k1, k2]{event.state};

    constexpr auto u2{muon_mass_c2 * muon_mass_c2};
    constexpr auto m2{electron_mass_c2 * electron_mass_c2};
    constexpr auto m4{m2 * m2};

    const auto qp{muon_mass_c2 * p.e()};
    const auto qp1{muon_mass_c2 * p1.e()};
    const auto qp2{muon_mass_c2 * p2.e()};
    const auto pp1{p * p1};
    const auto pp2{p * p2};
    const auto p1p2{p1 * p2};
    const auto qk1{muon_mass_c2 * k1.e()};
    const auto qk2{muon_mass_c2 * k2.e()};
    const auto p2k1{p2 * k1};
    const auto p1k2{p1 * k2};
    const auto pk1{p * k1};
    const auto pk2{p * k2};
    const auto p1k1{p1 * k1};
    const auto k1k2{k1 * k2};
    const auto p2k2{p2 * k2};

    const auto qps{qp * qp};
    const auto qp12{qp1 * qp1};
    const auto qp22{qp2 * qp2};
    const auto pp12{pp1 * pp1};
    const auto pp22{pp2 * pp2};
    const auto p1p22{p1p2 * p1p2};

    const auto C1{1 / (2 * (m2 + pp1 + pp2 + p1p2))};
    const auto C2{1 / (2 * (m2 - qp1 - qp2 + p1p2))};
    const auto C3{1 / (2 * (m2 - qp - qp1 + pp1))};
    const auto D1{1 / (2 * (m2 + p1p2))};
    const auto D2{1 / (2 * (m2 + pp1))};

    const auto tr11{-(qk2 * (p2k1 * (pp12 - pp1 * (m2 + pp2) + m2 * (m2 + p1p2) -
                                     pp2 * (2. * m2 + p1p2)) +
                             p1k1 * (m4 - m2 * pp2 + pp22 + m2 * p1p2 -
                                     pp1 * (2. * m2 + pp2 + p1p2)) +
                             pk1 * ((2. * m2 - pp2) * (m2 + p1p2) -
                                    pp1 * (m2 + 2. * pp2 + p1p2))))};
    const auto tr12{m2 * pk1 * p1k2 * qp - m2 * p1k1 * p1k2 * qp + m2 * pk1 * p2k2 * qp -
                    m2 * p2k1 * p2k2 * qp - 2. * m2 * pk1 * qk2 * qp - m2 * p1k1 * qk2 * qp -
                    m2 * p2k1 * qk2 * qp + pk1 * p1k2 * qp * p1p2 + p2k1 * p1k2 * qp * p1p2 +
                    pk1 * p2k2 * qp * p1p2 + p1k1 * p2k2 * qp * p1p2 - 2. * pk1 * qk2 * qp * p1p2 -
                    p1k1 * qk2 * qp * p1p2 - p2k1 * qk2 * qp * p1p2 + qk1 * (m2 * qk2 * pp1 + m2 * p2k2 * pp2 + m2 * qk2 * pp2 - p2k2 * pp1 * p1p2 + qk2 * pp1 * p1p2 + qk2 * pp2 * p1p2 - 2. * m2 * pk2 * (m2 + p1p2) + p1k2 * (m2 * pp1 - pp2 * p1p2)) -
                    m2 * pk1 * pk2 * qp1 + m2 * p1k1 * pk2 * qp1 + pk1 * p2k2 * pp1 * qp1 +
                    2. * p2k1 * p2k2 * pp1 * qp1 - p2k1 * qk2 * pp1 * qp1 - pk1 * p2k2 * pp2 * qp1 -
                    2. * p1k1 * p2k2 * pp2 * qp1 + 2. * pk1 * qk2 * pp2 * qp1 + p1k1 * qk2 * pp2 * qp1 -
                    pk1 * pk2 * p1p2 * qp1 - p2k1 * pk2 * p1p2 * qp1 - m2 * pk1 * pk2 * qp2 +
                    m2 * p2k1 * pk2 * qp2 - pk1 * p1k2 * pp1 * qp2 - 2. * p2k1 * p1k2 * pp1 * qp2 +
                    2. * pk1 * qk2 * pp1 * qp2 + p2k1 * qk2 * pp1 * qp2 + pk1 * p1k2 * pp2 * qp2 +
                    2. * p1k1 * p1k2 * pp2 * qp2 - p1k1 * qk2 * pp2 * qp2 - pk1 * pk2 * p1p2 * qp2 -
                    p1k1 * pk2 * p1p2 * qp2 + k1k2 * (2. * m2 * qp * (m2 + p1p2) + pp2 * (p1p2 * qp1 - m2 * qp2) + pp1 * (-(m2 * qp1) + p1p2 * qp2))};
    const auto tr13{2. * qk2 * (p1k1 * pp2 * (-2. * m2 + pp2) + pk1 * (pp1 * (m2 - pp2) + m2 * (m2 + p1p2) - pp2 * (2. * m2 + p1p2)) + p2k1 * (pp1 * (m2 - pp2) + m2 * (m2 + p1p2) - pp2 * (2. * m2 + p1p2)))};
    const auto tr14{(m2 * pk1 * p1k2 * qp + m2 * p1k1 * p1k2 * qp + 4. * m2 * p2k1 * p1k2 * qp -
                     m2 * pk1 * p2k2 * qp - m2 * p1k1 * p2k2 * qp - 2. * m2 * pk1 * qk2 * qp -
                     2. * m2 * p1k1 * qk2 * qp - 4. * m2 * p2k1 * qk2 * qp - 2. * p1k1 * p1k2 * pp2 * qp +
                     2. * p1k1 * qk2 * pp2 * qp + 2. * pk1 * p1k2 * qp * p1p2 + 2. * p2k1 * p1k2 * qp * p1p2 -
                     2. * pk1 * qk2 * qp * p1p2 - 2. * p2k1 * qk2 * qp * p1p2 -
                     qk1 * (-2. * (m2 + pp1) * (m2 * p2k2 - qk2 * pp2) - p1k2 * (pp1 * (m2 + 2. * pp2) + m2 * (m2 + pp2 - p1p2)) + m2 * pk2 * (m2 + pp1 + pp2 + p1p2)) -
                     m2 * pk1 * pk2 * qp1 - m2 * p1k1 * pk2 * qp1 - 4. * m2 * p2k1 * pk2 * qp1 +
                     m2 * pk1 * p2k2 * qp1 - m2 * p1k1 * p2k2 * qp1 + 2. * m2 * p2k1 * p2k2 * qp1 +
                     2. * m2 * pk1 * qk2 * qp1 + 2. * m2 * p1k1 * qk2 * qp1 + 4. * m2 * p2k1 * qk2 * qp1 +
                     2. * pk1 * p2k2 * pp1 * qp1 + 2. * p2k1 * p2k2 * pp1 * qp1 + 2. * p1k1 * pk2 * pp2 * qp1 -
                     2. * p2k1 * qk2 * pp2 * qp1 - 2. * pk1 * pk2 * p1p2 * qp1 - 2. * p2k1 * pk2 * p1p2 * qp1 +
                     m2 * pk1 * pk2 * qp2 + m2 * p1k1 * pk2 * qp2 - m2 * pk1 * p1k2 * qp2 +
                     m2 * p1k1 * p1k2 * qp2 - 2. * m2 * p2k1 * p1k2 * qp2 + 2. * m2 * pk1 * qk2 * qp2 +
                     2. * m2 * p2k1 * qk2 * qp2 - 2. * pk1 * p1k2 * pp1 * qp2 - 2. * p2k1 * p1k2 * pp1 * qp2 +
                     2. * pk1 * qk2 * pp1 * qp2 + 2. * p2k1 * qk2 * pp1 * qp2 +
                     k1k2 * (m2 * qp * (m2 + pp1 + pp2 + p1p2) - (pp1 * (m2 + 2. * pp2) + m2 * (m2 + pp2 - p1p2)) * qp1 - 2. * m2 * (m2 + pp1) * qp2)) /
                    2.0};
    const auto tr22{-(pk1 * (-(p1k2 * (m2 * u2 + p1p2 * (u2 + qp1) + qp1 * (2. * m2 - qp2) +
                                       m2 * qp2 + qp22)) +
                             qk2 * (qp1 * (m2 - 2. * qp2) + m2 * (m2 + u2 + qp2) +
                                    p1p2 * (m2 + u2 + qp1 + qp2)) -
                             p2k2 * (qp12 + qp1 * (m2 - qp2) +
                                     p1p2 * (u2 + qp2) + m2 * (u2 + 2. * qp2))))};
    const auto tr23{(-2. * m2 * pk1 * p1k2 * qp + m2 * p1k1 * p1k2 * qp - m2 * p2k1 * p1k2 * qp +
                     m2 * p1k1 * p2k2 * qp + m2 * p2k1 * p2k2 * qp + 2. * m2 * pk1 * qk2 * qp +
                     2. * m2 * p2k1 * qk2 * qp - 2. * pk1 * p1k2 * qp * p1p2 - 2. * p2k1 * p1k2 * qp * p1p2 +
                     2. * pk1 * qk2 * qp * p1p2 + 2. * p2k1 * qk2 * qp * p1p2 -
                     qk1 * (-2. * (m2 * pk2 - qk2 * pp2) * (m2 + p1p2) + m2 * p2k2 * (m2 + pp1 + pp2 + p1p2) - p1k2 * (m2 * (m2 - pp1 + pp2) + (m2 + 2. * pp2) * p1p2)) +
                     2. * m2 * pk1 * pk2 * qp1 - m2 * p1k1 * pk2 * qp1 + m2 * p2k1 * pk2 * qp1 -
                     4. * m2 * pk1 * p2k2 * qp1 - m2 * p1k1 * p2k2 * qp1 - m2 * p2k1 * p2k2 * qp1 +
                     4. * m2 * pk1 * qk2 * qp1 + 2. * m2 * p1k1 * qk2 * qp1 + 2. * m2 * p2k1 * qk2 * qp1 -
                     2. * pk1 * p2k2 * pp1 * qp1 - 2. * p2k1 * p2k2 * pp1 * qp1 + 2. * p1k1 * p2k2 * pp2 * qp1 -
                     2. * pk1 * qk2 * pp2 * qp1 + 2. * pk1 * pk2 * p1p2 * qp1 + 2. * p2k1 * pk2 * p1p2 * qp1 -
                     m2 * p1k1 * pk2 * qp2 - m2 * p2k1 * pk2 * qp2 + 4. * m2 * pk1 * p1k2 * qp2 +
                     m2 * p1k1 * p1k2 * qp2 + m2 * p2k1 * p1k2 * qp2 - 4. * m2 * pk1 * qk2 * qp2 -
                     2. * m2 * p1k1 * qk2 * qp2 - 2. * m2 * p2k1 * qk2 * qp2 + 2. * pk1 * p1k2 * pp1 * qp2 +
                     2. * p2k1 * p1k2 * pp1 * qp2 - 2. * pk1 * qk2 * pp1 * qp2 - 2. * p2k1 * qk2 * pp1 * qp2 -
                     2. * p1k1 * p1k2 * pp2 * qp2 + 2. * p1k1 * qk2 * pp2 * qp2 +
                     k1k2 * (-2. * m2 * qp * (m2 + p1p2) - (m2 * (m2 - pp1 + pp2) + (m2 + 2. * pp2) * p1p2) * qp1 + m2 * (m2 + pp1 + pp2 + p1p2) * qp2)) /
                    2.0};
    const auto tr24{(qp1 * (-(m2 * p2k1 * pk2) - u2 * p2k1 * pk2 + m2 * qk1 * pk2 + m2 * pk1 * p1k2 +
                            m2 * p2k1 * p1k2 - m2 * pk1 * p2k2 - u2 * pk1 * p2k2 + m2 * qk1 * p2k2 - m2 * pk1 * qk2 -
                            m2 * p2k1 * qk2 + 2. * p2k1 * p1k2 * pp1 - 2. * p2k1 * qk2 * pp1 + 2. * qk1 * p1k2 * pp2 -
                            2. * qk1 * qk2 * pp2 - p1k1 * (m2 * pk2 + m2 * p2k2 + 2. * (p1k2 - qk2) * pp2) -
                            2. * p2k1 * p1k2 * qp + 2. * p2k1 * qk2 * qp + 2. * pk1 * p1k2 * p1p2 - 2. * pk1 * qk2 * p1p2 +
                            2. * p2k1 * pk2 * qp1 + 2. * pk1 * p2k2 * qp1 + k1k2 * (m2 * pp1 + pp2 * (m2 + u2 - 2. * qp1) + m2 * (m2 - qp + p1p2 - qp2)) -
                            2. * pk1 * p1k2 * qp2 + 2. * pk1 * qk2 * qp2)) /
                        2. +
                    u2 * ((m2 * pk1 * p1k2 - 2. * m2 * pk1 * p2k2 + m2 * k1k2 * pp1 + 2. * m2 * k1k2 * pp2 -
                           p1k1 * (m2 * pk2 + m2 * p2k2 + 2. * (2. * p1k2 - qk2) * pp2) + m2 * k1k2 * p1p2 +
                           4. * pk1 * p1k2 * p1p2 - 2. * pk1 * qk2 * p1p2 + p2k1 * (-2. * qk2 * pp1 + p1k2 * (m2 + 4. * pp1) - 2. * pk2 * (m2 - qp1)) + 2. * pk1 * p2k2 * qp1 - 2. * k1k2 * pp2 * qp1) /
                          4.) +
                    m2 * ((2. * m2 * qk1 * pk2 - u2 * qk1 * pk2 - 2. * u2 * pk1 * p1k2 + 4. * m2 * qk1 * p1k2 -
                           2. * u2 * qk1 * p1k2 - 2. * u2 * pk1 * p2k2 + 2. * m2 * qk1 * p2k2 - u2 * qk1 * p2k2 -
                           2. * m2 * pk1 * qk2 + u2 * pk1 * qk2 - 2. * m2 * p1k1 * qk2 - 4. * m2 * qk1 * qk2 +
                           2. * qk1 * p1k2 * pp1 + 2. * qk1 * p2k2 * pp1 - 4. * qk1 * qk2 * pp1 + 2. * p1k1 * qk2 * pp2 -
                           4. * qk1 * qk2 * pp2 - 2. * p1k1 * p1k2 * qp + 2. * qk1 * p1k2 * qp - 2. * p1k1 * p2k2 * qp +
                           2. * qk1 * p2k2 * qp + 2. * p1k1 * qk2 * qp + 2. * qk1 * pk2 * p1p2 + 2. * qk1 * p1k2 * p1p2 -
                           2. * pk1 * qk2 * p1p2 - 4. * qk1 * qk2 * p1p2 + p2k1 * (qk2 * (-2. * m2 + u2 - 2. * pp1 + 2. * qp) - 2. * pk2 * (u2 - qp1) - 2. * p1k2 * (u2 - qp1)) + 2. * pk1 * p1k2 * qp1 + 2. * pk1 * p2k2 * qp1 +
                           4. * qk1 * qk2 * qp1 - 2. * p1k1 * pk2 * qp2 + 2. * qk1 * pk2 * qp2 - 2. * p1k1 * p1k2 * qp2 +
                           2. * qk1 * p1k2 * qp2 + 2. * pk1 * qk2 * qp2 + 2. * p1k1 * qk2 * qp2 + k1k2 * (-2. * m2 * u2 + 2. * pp2 * (u2 - qp1) + 2. * m2 * qp1 + qp * (2. * m2 + u2 + 2. * p1p2 - 2. * qp1 - 4. * qp2) + 2. * m2 * qp2 + u2 * qp2 + 2. * pp1 * qp2 - 2. * qp1 * qp2)) /
                          4.) +
                    u2 * m2 * ((2. * p2k1 * pk2 + qk1 * pk2 + 3. * pk1 * p1k2 + 3. * p2k1 * p1k2 + 2. * qk1 * p1k2 + 2. * pk1 * p2k2 + qk1 * p2k2 - 3. * pk1 * qk2 - 3. * p2k1 * qk2 - p1k1 * (pk2 + p2k2 + 2. * qk2) + k1k2 * (6. * m2 + 3. * pp1 - qp + 3. * p1p2 - qp2)) / 4.)};
    const auto tr33{-(qk2 * (p1k1 * (m4 + m2 * pp1 - m2 * pp2 + pp22 - (2. * m2 + pp1 + pp2) * p1p2) +
                             p2k1 * ((m2 + pp1) * (2. * m2 - pp2) - (m2 + pp1 + 2. * pp2) * p1p2) +
                             pk1 * (m2 * (m2 + pp1) - (2. * m2 + pp1) * pp2 - (m2 + pp2) * p1p2 + p1p22)))};
    const auto tr34{m2 * pk1 * p2k2 * qp - m2 * p2k1 * p2k2 * qp - p1k1 * p2k2 * pp1 * qp -
                    p2k1 * p2k2 * pp1 * qp + 2. * p1k1 * p1k2 * pp2 * qp + p2k1 * p1k2 * pp2 * qp -
                    p1k1 * qk2 * pp2 * qp - 2. * pk1 * p1k2 * qp * p1p2 - p2k1 * p1k2 * qp * p1p2 +
                    pk1 * qk2 * qp * p1p2 + 2. * p2k1 * qk2 * qp * p1p2 +
                    qk1 * (-2. * m2 * p2k2 * (m2 + pp1) + m2 * pk2 * pp2 + m2 * qk2 * pp2 +
                           qk2 * pp1 * pp2 + m2 * qk2 * p1p2 - pk2 * pp1 * p1p2 + qk2 * pp1 * p1p2 +
                           p1k2 * (-(pp1 * pp2) + m2 * p1p2)) +
                    m2 * p1k1 * p2k2 * qp1 -
                    m2 * p2k1 * p2k2 * qp1 - pk1 * p2k2 * pp1 * qp1 - p2k1 * p2k2 * pp1 * qp1 -
                    2. * p1k1 * pk2 * pp2 * qp1 - p2k1 * pk2 * pp2 * qp1 + p1k1 * qk2 * pp2 * qp1 +
                    2. * p2k1 * qk2 * pp2 * qp1 + 2. * pk1 * pk2 * p1p2 * qp1 + p2k1 * pk2 * p1p2 * qp1 -
                    pk1 * qk2 * p1p2 * qp1 - m2 * pk1 * pk2 * qp2 + m2 * p2k1 * pk2 * qp2 -
                    m2 * p1k1 * p1k2 * qp2 + m2 * p2k1 * p1k2 * qp2 - m2 * pk1 * qk2 * qp2 -
                    m2 * p1k1 * qk2 * qp2 - 2. * m2 * p2k1 * qk2 * qp2 + p1k1 * pk2 * pp1 * qp2 +
                    p2k1 * pk2 * pp1 * qp2 + pk1 * p1k2 * pp1 * qp2 + p2k1 * p1k2 * pp1 * qp2 -
                    pk1 * qk2 * pp1 * qp2 - p1k1 * qk2 * pp1 * qp2 - 2. * p2k1 * qk2 * pp1 * qp2 +
                    k1k2 * (p1p2 * (pp1 * qp - m2 * qp1) + pp2 * (-(m2 * qp) + pp1 * qp1) +
                            2. * m2 * (m2 + pp1) * qp2)};
    const auto tr44{-(p2k1 * (-(pk2 * (pp1 * (u2 + qp) + m2 * (u2 + 2. * qp) +
                                       (m2 - qp) * qp1 + qp12)) -
                              p1k2 * (m2 * u2 + m2 * qp + qps +
                                      (2. * m2 - qp) * qp1 + pp1 * (u2 + qp1)) +
                              qk2 * (m2 * (m2 + u2 + qp) +
                                     (m2 - 2. * qp) * qp1 + pp1 * (m2 + u2 + qp + qp1))))};

    const auto matr2e{C1 * C1 * D1 * D1 * tr11 - C1 * C1 * D1 * D2 * tr13 + C1 * C1 * D2 * D2 * tr33};
    const auto matr2mu{C2 * C2 * D1 * D1 * tr22 - C2 * C3 * D1 * D2 * tr24 + C3 * C3 * D2 * D2 * tr44};
    const auto matr2emu{C1 * C2 * D1 * D1 * tr12 - C1 * C3 * D1 * D2 * tr14 - C1 * C2 * D1 * D2 * tr23 + C1 * C3 * D2 * D2 * tr34};

    return event.weight * (matr2e + matr2mu + matr2emu);
}

} // namespace Mustard::inline Extension::Geant4X::inline DecayChannel
