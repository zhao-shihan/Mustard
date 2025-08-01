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

#include "Mustard/Execution/Executor.h++"
#include "Mustard/Geant4X/DecayChannel/MuonInternalConversionDecayChannel.h++"
#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Utility/PhysicalConstant.h++"
#include "Mustard/Utility/PrettyLog.h++"
#include "Mustard/Utility/VectorArithmeticOperator.h++"

#include "CLHEP/Vector/LorentzVector.h"

#include "G4DecayProducts.hh"
#include "G4DynamicParticle.hh"
#include "Randomize.hh"

#include "mplr/mplr.hpp"

#include "muc/array"
#include "muc/math"
#include "muc/utility"

#include "gsl/gsl"

#include "fmt/format.h"

#include <bit>
#include <limits>
#include <stdexcept>

namespace Mustard::Geant4X::inline DecayChannel {

using namespace PhysicalConstant;

MuonInternalConversionDecayChannel::MuonInternalConversionDecayChannel(const G4String& parentName, G4double br, G4int verbose) : // clang-format off
    G4VDecayChannel{"MuonICDecay", verbose}, // clang-format on
    fMSqVersion{MSqVersion::McMule2020},
    fMetropolisDelta{0.05},
    fMetropolisDiscard{100},
    fBias{[](auto&&) { return 1; }},
    fGENBOD{muon_mass_c2, {electron_mass_c2, electron_mass_c2, electron_mass_c2, 0, 0}},
    fReady{},
    fRandomState{},
    fEvent{},
    fBiasedMSq{},
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

auto MuonInternalConversionDecayChannel::MSqVersion(std::string_view ver) {
    if (ver == "McMule2020") {
        fMSqVersion = MSqVersion::McMule2020;
    } else if (ver == "RR2009PRD") {
        fMSqVersion = MSqVersion::RR2009PRD;
    } else {
        Throw<std::invalid_argument>(fmt::format("No squared amplitude version named '{}'", ver));
    }
}

auto MuonInternalConversionDecayChannel::Bias(std::function<auto(const GENBOD<5>::State&)->double> b) -> void {
    fBias = std::move(b);
    fReady = false;
}

auto MuonInternalConversionDecayChannel::Initialize() -> void {
    if (fReady) {
        return;
    }
    // initialize
    while (true) {
        std::ranges::generate(fRandomState, [this] { return Math::Random::Uniform<double>{}(fXoshiro256Plus); });
        fEvent = fGENBOD(fRandomState);
        if (const auto bias{BiasWithCheck(fEvent.state)};
            bias >= std::numeric_limits<double>::min()) {
            fBiasedMSq = bias * WeightedMSq(fEvent);
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

auto MuonInternalConversionDecayChannel::EstimateWeightNormalizationFactor(unsigned long long n) -> std::tuple<double, double, double> {
    if (n == 0) {
        return {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), 0};
    }

    // store state
    auto originalBias{std::move(fBias)};
    auto originalReady{std::move(fReady)};
    auto originalRandomState{std::move(fRandomState)};
    auto originalEvent{std::move(fEvent)};
    auto originalBiasedMSq{std::move(fBiasedMSq)};

    // --- above is protected ---

    Bias([](auto&&) { return 1; }); // to evaluate the weight normalization factor of user-defined bias, temporarily switch to unbiased function
    Initialize();

    using namespace Mustard::VectorArithmeticOperator::Vector2ArithmeticOperator;
    muc::array2ld sum{};
    const auto worldComm{mplr::comm_world()};
    {                               // Monte Carlo integration here
        muc::array2ld partialSum{}; // improve numeric stability
        Executor<unsigned long long>{"Estimation", "Sample"}
            .Execute(n, [&, partialSumThreshold = muc::llround(std::sqrt(n / worldComm.size()))](auto i) {
                MainSamplingLoop();
                const auto bias{originalBias(fEvent.state)};
                partialSum += muc::array2ld{bias, muc::pow<2>(bias)};
                if ((i + 1) % partialSumThreshold == 0) {
                    sum += partialSum;
                    partialSum = {};
                }
            });
        sum += partialSum;
    }
    worldComm.allreduce([](auto a, auto b) { return a + b; }, sum);
    const auto result{gsl::narrow_cast<double>(sum[0] / n)};
    const auto error{gsl::narrow_cast<double>(std::sqrt(sum[1]) / n)};
    const auto nEff{muc::pow<2>(result / error)};

    // --- below is protected ---

    // restore state
    fBias = std::move(originalBias);
    fReady = std::move(originalReady);
    fRandomState = std::move(originalRandomState);
    fEvent = std::move(originalEvent);
    fBiasedMSq = std::move(originalBiasedMSq);

    return {result, error, nEff};
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

auto MuonInternalConversionDecayChannel::BiasWithCheck(const GENBOD<5>::State& state) const -> double {
    const auto bias{fBias(state)};
    if (bias < 0) {
        Throw<std::runtime_error>("Bias should be non-negative");
    }
    return bias;
}

auto MuonInternalConversionDecayChannel::UpdateState(double delta) -> void {
    GENBOD<5>::RandomState newRandomState;
    GENBOD<5>::Event newEvent;
    while (true) {
        std::ranges::transform(std::as_const(fRandomState), newRandomState.begin(),
                               [&](auto u) {
                                   return Math::Random::Distribution::UniformCompact{
                                       muc::clamp<"()">(u - delta, 0., 1.),
                                       muc::clamp<"()">(u + delta, 0., 1.)}(fXoshiro256Plus);
                               });
        newEvent = fGENBOD(newRandomState);
        const auto bias{BiasWithCheck(newEvent.state)};
        if (bias <= std::numeric_limits<double>::min()) {
            continue;
        }

        const auto newBiasedMSq{bias * WeightedMSq(newEvent)};
        if (newBiasedMSq >= fBiasedMSq or
            newBiasedMSq >= fBiasedMSq * Math::Random::Distribution::Uniform<double>{}(fXoshiro256Plus)) {
            fRandomState = newRandomState;
            fEvent = newEvent;
            fBiasedMSq = newBiasedMSq;
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

auto MuonInternalConversionDecayChannel::WeightedMSq(const GENBOD<5>::Event& event) -> double {
    switch (fMSqVersion) {
    case MSqVersion::McMule2020:
        return event.weight * MSqMcMule2020(event.state);
    case MSqVersion::RR2009PRD:
        return event.weight * MSqRR2009PRD(event.state);
    }
    muc::unreachable();
}

auto MuonInternalConversionDecayChannel::MSqMcMule2020(const GENBOD<5>::State& state) -> double {
    const CLHEP::HepLorentzVector q1{muon_mass_c2};
    const auto& [q2, q6, q5, q4, q3]{state};
    const CLHEP::HepLorentzVector pol1{parent_polarization};

    // Adapt from McMule v0.5.0, mudecrare/mudecrare_pm2ennee.f95, FUNCTION PM2ENNEE
    //
    // Copyright 2020-2024  Yannick Ulrich and others (The McMule development team)
    //

    const auto s12 = q1 * q2;
    const auto s13 = q1 * q3;
    const auto s14 = q1 * q4;
    const auto s15 = q1 * q5;
    const auto s16 = q1 * q6;
    const auto s23 = q2 * q3;
    const auto s24 = q2 * q4;
    const auto s25 = q2 * q5;
    const auto s26 = q2 * q6;
    const auto s34 = q3 * q4;
    const auto s35 = q3 * q5;
    const auto s36 = q3 * q6;
    const auto s45 = q4 * q5;
    const auto s46 = q4 * q6;
    const auto s56 = q5 * q6;

    const auto s2n = q2 * pol1;
    const auto s3n = q3 * pol1;
    const auto s4n = q4 * pol1;
    const auto s5n = q5 * pol1;
    const auto s6n = q6 * pol1;

    const auto M1 = std::sqrt(q1.m2() / 2);
    const auto M2 = std::sqrt(std::abs(q2.m2()) / 2);

    using muc::pow;

    const auto if11 =
        -4 * pow<4>(M2) * s13 * s24 - 4 * pow<2>(M1) * pow<2>(M2) * s13 * s24 -
        2 * pow<2>(M2) * s13 * s15 * s24 - 2 * pow<2>(M2) * s13 * s16 * s24 +
        2 * s13 * s15 * s16 * s24 + 4 * pow<2>(M1) * pow<2>(M2) * s24 * s35 +
        4 * pow<2>(M2) * s15 * s24 * s35 + 2 * pow<2>(M2) * s16 * s24 * s35 -
        s15 * s16 * s24 * s35 + pow<2>(s16) * s24 * s35 +
        4 * pow<2>(M1) * pow<2>(M2) * s24 * s36 + 2 * pow<2>(M2) * s15 * s24 * s36 +
        pow<2>(s15) * s24 * s36 + 4 * pow<2>(M2) * s16 * s24 * s36 -
        s15 * s16 * s24 * s36 + 4 * M1 * pow<4>(M2) * s24 * s3n +
        4 * pow<3>(M1) * pow<2>(M2) * s24 * s3n - 2 * M1 * s15 * s16 * s24 * s3n -
        2 * pow<2>(M1) * s13 * s24 * s56 - 2 * pow<2>(M2) * s13 * s24 * s56 -
        s13 * s15 * s24 * s56 - s13 * s16 * s24 * s56 +
        2 * pow<2>(M1) * s24 * s35 * s56 + s15 * s24 * s35 * s56 +
        2 * pow<2>(M1) * s24 * s36 * s56 + s16 * s24 * s36 * s56 +
        2 * pow<3>(M1) * s24 * s3n * s56 + 2 * M1 * pow<2>(M2) * s24 * s3n * s56 +
        2 * M1 * pow<2>(M2) * s13 * s24 * s5n -
        4 * M1 * pow<2>(M2) * s24 * s35 * s5n -
        2 * M1 * pow<2>(M2) * s24 * s36 * s5n - M1 * s15 * s24 * s36 * s5n +
        M1 * s16 * s24 * s36 * s5n + M1 * s13 * s24 * s56 * s5n -
        M1 * s24 * s35 * s56 * s5n + 2 * M1 * pow<2>(M2) * s13 * s24 * s6n -
        2 * M1 * pow<2>(M2) * s24 * s35 * s6n + M1 * s15 * s24 * s35 * s6n -
        M1 * s16 * s24 * s35 * s6n - 4 * M1 * pow<2>(M2) * s24 * s36 * s6n +
        M1 * s13 * s24 * s56 * s6n - M1 * s24 * s36 * s56 * s6n;
    const auto if22 =
        -8 * pow<4>(M2) * s13 * s24 + 2 * pow<2>(M2) * s13 * s24 * s25 +
        2 * pow<2>(M2) * s13 * s24 * s26 + 2 * s13 * s24 * s25 * s26 +
        8 * M1 * pow<4>(M2) * s24 * s3n - 2 * M1 * pow<2>(M2) * s24 * s25 * s3n -
        2 * M1 * pow<2>(M2) * s24 * s26 * s3n - 2 * M1 * s24 * s25 * s26 * s3n -
        4 * pow<4>(M2) * s13 * s45 + 4 * pow<2>(M2) * s13 * s25 * s45 +
        2 * pow<2>(M2) * s13 * s26 * s45 + s13 * s25 * s26 * s45 -
        s13 * pow<2>(s26) * s45 + 4 * M1 * pow<4>(M2) * s3n * s45 -
        4 * M1 * pow<2>(M2) * s25 * s3n * s45 -
        2 * M1 * pow<2>(M2) * s26 * s3n * s45 - M1 * s25 * s26 * s3n * s45 +
        M1 * pow<2>(s26) * s3n * s45 - 4 * pow<4>(M2) * s13 * s46 +
        2 * pow<2>(M2) * s13 * s25 * s46 - s13 * pow<2>(s25) * s46 +
        4 * pow<2>(M2) * s13 * s26 * s46 + s13 * s25 * s26 * s46 +
        4 * M1 * pow<4>(M2) * s3n * s46 - 2 * M1 * pow<2>(M2) * s25 * s3n * s46 +
        M1 * pow<2>(s25) * s3n * s46 - 4 * M1 * pow<2>(M2) * s26 * s3n * s46 -
        M1 * s25 * s26 * s3n * s46 - 4 * pow<2>(M2) * s13 * s24 * s56 +
        s13 * s24 * s25 * s56 + s13 * s24 * s26 * s56 +
        4 * M1 * pow<2>(M2) * s24 * s3n * s56 - M1 * s24 * s25 * s3n * s56 -
        M1 * s24 * s26 * s3n * s56 - 2 * pow<2>(M2) * s13 * s45 * s56 +
        s13 * s25 * s45 * s56 + 2 * M1 * pow<2>(M2) * s3n * s45 * s56 -
        M1 * s25 * s3n * s45 * s56 - 2 * pow<2>(M2) * s13 * s46 * s56 +
        s13 * s26 * s46 * s56 + 2 * M1 * pow<2>(M2) * s3n * s46 * s56 -
        M1 * s26 * s3n * s46 * s56;
    const auto if33 =
        -4 * pow<4>(M2) * s13 * s45 - 4 * pow<2>(M1) * pow<2>(M2) * s13 * s45 -
        2 * pow<2>(M2) * s12 * s13 * s45 - 2 * pow<2>(M2) * s13 * s16 * s45 +
        2 * s12 * s13 * s16 * s45 + 4 * pow<2>(M1) * pow<2>(M2) * s23 * s45 +
        4 * pow<2>(M2) * s12 * s23 * s45 + 2 * pow<2>(M2) * s16 * s23 * s45 -
        s12 * s16 * s23 * s45 + pow<2>(s16) * s23 * s45 -
        2 * pow<2>(M1) * s13 * s26 * s45 - 2 * pow<2>(M2) * s13 * s26 * s45 -
        s12 * s13 * s26 * s45 - s13 * s16 * s26 * s45 +
        2 * pow<2>(M1) * s23 * s26 * s45 + s12 * s23 * s26 * s45 +
        2 * M1 * pow<2>(M2) * s13 * s2n * s45 -
        4 * M1 * pow<2>(M2) * s23 * s2n * s45 + M1 * s13 * s26 * s2n * s45 -
        M1 * s23 * s26 * s2n * s45 + 4 * pow<2>(M1) * pow<2>(M2) * s36 * s45 +
        2 * pow<2>(M2) * s12 * s36 * s45 + pow<2>(s12) * s36 * s45 +
        4 * pow<2>(M2) * s16 * s36 * s45 - s12 * s16 * s36 * s45 +
        2 * pow<2>(M1) * s26 * s36 * s45 + s16 * s26 * s36 * s45 -
        2 * M1 * pow<2>(M2) * s2n * s36 * s45 - M1 * s12 * s2n * s36 * s45 +
        M1 * s16 * s2n * s36 * s45 + 4 * M1 * pow<4>(M2) * s3n * s45 +
        4 * pow<3>(M1) * pow<2>(M2) * s3n * s45 - 2 * M1 * s12 * s16 * s3n * s45 +
        2 * pow<3>(M1) * s26 * s3n * s45 + 2 * M1 * pow<2>(M2) * s26 * s3n * s45 +
        2 * M1 * pow<2>(M2) * s13 * s45 * s6n -
        2 * M1 * pow<2>(M2) * s23 * s45 * s6n + M1 * s12 * s23 * s45 * s6n -
        M1 * s16 * s23 * s45 * s6n + M1 * s13 * s26 * s45 * s6n -
        4 * M1 * pow<2>(M2) * s36 * s45 * s6n - M1 * s26 * s36 * s45 * s6n;
    const auto if44 =
        -4 * pow<4>(M2) * s13 * s24 + 4 * pow<2>(M2) * s13 * s24 * s25 -
        2 * pow<2>(M2) * s13 * s24 * s26 + s13 * s24 * s25 * s26 +
        4 * M1 * pow<4>(M2) * s24 * s3n - 4 * M1 * pow<2>(M2) * s24 * s25 * s3n +
        2 * M1 * pow<2>(M2) * s24 * s26 * s3n - M1 * s24 * s25 * s26 * s3n -
        8 * pow<4>(M2) * s13 * s45 + 2 * pow<2>(M2) * s13 * s25 * s45 -
        4 * pow<2>(M2) * s13 * s26 * s45 + s13 * s25 * s26 * s45 +
        8 * M1 * pow<4>(M2) * s3n * s45 - 2 * M1 * pow<2>(M2) * s25 * s3n * s45 +
        4 * M1 * pow<2>(M2) * s26 * s3n * s45 - M1 * s25 * s26 * s3n * s45 -
        4 * pow<4>(M2) * s13 * s46 + 2 * pow<2>(M2) * s13 * s25 * s46 -
        s13 * pow<2>(s25) * s46 - 2 * pow<2>(M2) * s13 * s26 * s46 +
        4 * M1 * pow<4>(M2) * s3n * s46 - 2 * M1 * pow<2>(M2) * s25 * s3n * s46 +
        M1 * pow<2>(s25) * s3n * s46 + 2 * M1 * pow<2>(M2) * s26 * s3n * s46 +
        2 * pow<2>(M2) * s13 * s24 * s56 + s13 * s24 * s25 * s56 -
        2 * M1 * pow<2>(M2) * s24 * s3n * s56 - M1 * s24 * s25 * s3n * s56 +
        2 * pow<2>(M2) * s13 * s45 * s56 + 2 * s13 * s25 * s45 * s56 +
        s13 * s26 * s45 * s56 - 2 * M1 * pow<2>(M2) * s3n * s45 * s56 -
        2 * M1 * s25 * s3n * s45 * s56 - M1 * s26 * s3n * s45 * s56 +
        4 * pow<2>(M2) * s13 * s46 * s56 + s13 * s25 * s46 * s56 +
        s13 * s26 * s46 * s56 - 4 * M1 * pow<2>(M2) * s3n * s46 * s56 -
        M1 * s25 * s3n * s46 * s56 - M1 * s26 * s3n * s46 * s56 -
        s13 * s24 * pow<2>(s56) + M1 * s24 * s3n * pow<2>(s56);
    const auto if12 =
        -8 * pow<4>(M2) * s14 * s23 - 4 * pow<2>(M2) * s12 * s13 * s24 -
        2 * pow<2>(M2) * s15 * s23 * s24 - 2 * pow<2>(M2) * s16 * s23 * s24 +
        2 * pow<2>(M2) * s13 * s14 * s25 + 2 * s13 * s16 * s24 * s25 +
        2 * pow<2>(M2) * s13 * s14 * s26 + 2 * s13 * s15 * s24 * s26 +
        8 * pow<4>(M2) * s12 * s34 - 2 * pow<2>(M2) * s15 * s25 * s34 -
        2 * pow<2>(M2) * s16 * s26 * s34 - 8 * M1 * pow<4>(M2) * s2n * s34 +
        2 * pow<2>(M2) * s12 * s24 * s35 + 2 * pow<2>(M2) * s14 * s25 * s35 -
        s16 * s24 * s25 * s35 + s16 * s24 * s26 * s35 -
        2 * M1 * pow<2>(M2) * s24 * s2n * s35 + 2 * pow<2>(M2) * s12 * s24 * s36 +
        s15 * s24 * s25 * s36 + 2 * pow<2>(M2) * s14 * s26 * s36 -
        s15 * s24 * s26 * s36 - 2 * M1 * pow<2>(M2) * s24 * s2n * s36 +
        4 * M1 * pow<2>(M2) * s12 * s24 * s3n -
        2 * M1 * pow<2>(M2) * s14 * s25 * s3n - 2 * M1 * s16 * s24 * s25 * s3n -
        2 * M1 * pow<2>(M2) * s14 * s26 * s3n - 2 * M1 * s15 * s24 * s26 * s3n -
        2 * pow<2>(M2) * s12 * s13 * s45 + 2 * pow<2>(M2) * s15 * s23 * s45 +
        s13 * s15 * s26 * s45 - s13 * s16 * s26 * s45 -
        2 * pow<2>(M2) * s12 * s35 * s45 + 2 * s16 * s26 * s35 * s45 +
        2 * M1 * pow<2>(M2) * s2n * s35 * s45 - 2 * s15 * s26 * s36 * s45 +
        2 * M1 * pow<2>(M2) * s12 * s3n * s45 - M1 * s15 * s26 * s3n * s45 +
        M1 * s16 * s26 * s3n * s45 - 2 * pow<2>(M2) * s12 * s13 * s46 +
        2 * pow<2>(M2) * s16 * s23 * s46 - s13 * s15 * s25 * s46 +
        s13 * s16 * s25 * s46 - 2 * s16 * s25 * s35 * s46 -
        2 * pow<2>(M2) * s12 * s36 * s46 + 2 * s15 * s25 * s36 * s46 +
        2 * M1 * pow<2>(M2) * s2n * s36 * s46 +
        2 * M1 * pow<2>(M2) * s12 * s3n * s46 + M1 * s15 * s25 * s3n * s46 -
        M1 * s16 * s25 * s3n * s46 + 8 * M1 * pow<4>(M2) * s23 * s4n -
        2 * M1 * pow<2>(M2) * s25 * s35 * s4n -
        2 * M1 * pow<2>(M2) * s26 * s36 * s4n - 4 * pow<2>(M2) * s14 * s23 * s56 -
        2 * s12 * s13 * s24 * s56 - s15 * s23 * s24 * s56 - s16 * s23 * s24 * s56 +
        s13 * s14 * s25 * s56 + s13 * s14 * s26 * s56 +
        4 * pow<2>(M2) * s12 * s34 * s56 + s16 * s25 * s34 * s56 +
        s15 * s26 * s34 * s56 - 4 * M1 * pow<2>(M2) * s2n * s34 * s56 +
        s12 * s24 * s35 * s56 - s14 * s26 * s35 * s56 - M1 * s24 * s2n * s35 * s56 +
        s12 * s24 * s36 * s56 - s14 * s25 * s36 * s56 - M1 * s24 * s2n * s36 * s56 +
        2 * M1 * s12 * s24 * s3n * s56 - M1 * s14 * s25 * s3n * s56 -
        M1 * s14 * s26 * s3n * s56 - s12 * s13 * s45 * s56 - s16 * s23 * s45 * s56 +
        s12 * s36 * s45 * s56 - M1 * s2n * s36 * s45 * s56 +
        M1 * s12 * s3n * s45 * s56 - s12 * s13 * s46 * s56 - s15 * s23 * s46 * s56 +
        s12 * s35 * s46 * s56 - M1 * s2n * s35 * s46 * s56 +
        M1 * s12 * s3n * s46 * s56 + 4 * M1 * pow<2>(M2) * s23 * s4n * s56 +
        M1 * s26 * s35 * s4n * s56 + M1 * s25 * s36 * s4n * s56 +
        2 * M1 * pow<2>(M2) * s23 * s24 * s5n +
        2 * M1 * pow<2>(M2) * s25 * s34 * s5n - M1 * s24 * s25 * s36 * s5n +
        M1 * s24 * s26 * s36 * s5n - 2 * M1 * pow<2>(M2) * s23 * s45 * s5n +
        2 * M1 * s26 * s36 * s45 * s5n - 2 * M1 * s25 * s36 * s46 * s5n +
        M1 * s23 * s24 * s56 * s5n - M1 * s26 * s34 * s56 * s5n +
        M1 * s23 * s46 * s56 * s5n + 2 * M1 * pow<2>(M2) * s23 * s24 * s6n +
        2 * M1 * pow<2>(M2) * s26 * s34 * s6n + M1 * s24 * s25 * s35 * s6n -
        M1 * s24 * s26 * s35 * s6n - 2 * M1 * s26 * s35 * s45 * s6n -
        2 * M1 * pow<2>(M2) * s23 * s46 * s6n + 2 * M1 * s25 * s35 * s46 * s6n +
        M1 * s23 * s24 * s56 * s6n - M1 * s25 * s34 * s56 * s6n +
        M1 * s23 * s45 * s56 * s6n;
    const auto if13 =
        4 * pow<4>(M2) * s13 * s14 - 2 * pow<2>(M2) * s13 * s14 * s16 -
        2 * pow<4>(M2) * s14 * s23 - pow<2>(M2) * s14 * s15 * s23 -
        pow<2>(M2) * s14 * s16 * s23 + 2 * pow<4>(M2) * s13 * s24 +
        2 * pow<2>(M1) * pow<2>(M2) * s13 * s24 - pow<2>(M2) * s13 * s15 * s24 +
        pow<2>(M2) * s13 * s16 * s24 - s13 * s15 * s16 * s24 +
        2 * pow<2>(M2) * s13 * s14 * s25 + s13 * s14 * s16 * s25 +
        2 * pow<2>(M2) * s13 * s14 * s26 - 8 * pow<2>(M1) * pow<4>(M2) * s34 -
        2 * pow<4>(M2) * s12 * s34 - 2 * pow<4>(M2) * s15 * s34 +
        2 * pow<2>(M2) * s12 * s15 * s34 - 4 * pow<4>(M2) * s16 * s34 +
        2 * pow<2>(M2) * s12 * s16 * s34 + 2 * pow<2>(M2) * s15 * s16 * s34 -
        4 * pow<2>(M1) * pow<2>(M2) * s25 * s34 + pow<2>(s16) * s25 * s34 -
        4 * pow<2>(M1) * pow<2>(M2) * s26 * s34 - pow<2>(M2) * s15 * s26 * s34 -
        pow<2>(M2) * s16 * s26 * s34 + 2 * M1 * pow<4>(M2) * s2n * s34 -
        M1 * pow<2>(M2) * s15 * s2n * s34 - M1 * pow<2>(M2) * s16 * s2n * s34 -
        2 * pow<4>(M2) * s14 * s35 - pow<2>(M2) * s12 * s14 * s35 -
        pow<2>(M2) * s14 * s16 * s35 + 2 * pow<2>(M1) * pow<2>(M2) * s24 * s35 -
        pow<2>(s16) * s24 * s35 - pow<2>(M2) * s14 * s26 * s35 -
        4 * pow<4>(M2) * s14 * s36 - pow<2>(M2) * s12 * s14 * s36 -
        pow<2>(M2) * s14 * s15 * s36 - 2 * pow<2>(M1) * pow<2>(M2) * s24 * s36 -
        2 * pow<2>(M2) * s16 * s24 * s36 + s15 * s16 * s24 * s36 -
        s14 * s16 * s25 * s36 - pow<2>(M2) * s14 * s26 * s36 +
        2 * M1 * pow<2>(M2) * s14 * s16 * s3n - 2 * M1 * pow<4>(M2) * s24 * s3n -
        2 * pow<3>(M1) * pow<2>(M2) * s24 * s3n + M1 * s15 * s16 * s24 * s3n -
        M1 * s14 * s16 * s25 * s3n + 2 * pow<4>(M2) * s13 * s45 +
        2 * pow<2>(M1) * pow<2>(M2) * s13 * s45 - pow<2>(M2) * s12 * s13 * s45 +
        pow<2>(M2) * s13 * s16 * s45 - s12 * s13 * s16 * s45 +
        2 * pow<2>(M1) * pow<2>(M2) * s23 * s45 - pow<2>(s16) * s23 * s45 +
        pow<2>(M1) * s13 * s26 * s45 + pow<2>(M2) * s13 * s26 * s45 +
        s13 * s16 * s26 * s45 + M1 * pow<2>(M2) * s13 * s2n * s45 -
        2 * pow<2>(M1) * pow<2>(M2) * s36 * s45 - 2 * pow<2>(M2) * s16 * s36 * s45 +
        s12 * s16 * s36 * s45 - 2 * pow<2>(M1) * s26 * s36 * s45 -
        s16 * s26 * s36 * s45 - M1 * s16 * s2n * s36 * s45 -
        2 * M1 * pow<4>(M2) * s3n * s45 - 2 * pow<3>(M1) * pow<2>(M2) * s3n * s45 +
        M1 * s12 * s16 * s3n * s45 - pow<3>(M1) * s26 * s3n * s45 -
        M1 * pow<2>(M2) * s26 * s3n * s45 + 2 * pow<4>(M2) * s13 * s46 +
        2 * pow<2>(M1) * pow<2>(M2) * s13 * s46 - pow<2>(M2) * s12 * s13 * s46 -
        pow<2>(M2) * s13 * s15 * s46 + 2 * pow<2>(M1) * pow<2>(M2) * s23 * s46 +
        pow<2>(M2) * s15 * s23 * s46 + pow<2>(M2) * s16 * s23 * s46 -
        pow<2>(M1) * s13 * s25 * s46 - pow<2>(M2) * s13 * s25 * s46 -
        s13 * s16 * s25 * s46 + M1 * pow<2>(M2) * s13 * s2n * s46 +
        2 * pow<2>(M1) * pow<2>(M2) * s35 * s46 + pow<2>(M2) * s12 * s35 * s46 +
        pow<2>(M2) * s16 * s35 * s46 - M1 * pow<2>(M2) * s2n * s35 * s46 +
        pow<2>(M2) * s12 * s36 * s46 + pow<2>(M2) * s15 * s36 * s46 +
        2 * pow<2>(M1) * s25 * s36 * s46 + s16 * s25 * s36 * s46 -
        M1 * pow<2>(M2) * s2n * s36 * s46 - 2 * M1 * pow<4>(M2) * s3n * s46 -
        2 * pow<3>(M1) * pow<2>(M2) * s3n * s46 + pow<3>(M1) * s25 * s3n * s46 +
        M1 * pow<2>(M2) * s25 * s3n * s46 - 4 * M1 * pow<4>(M2) * s13 * s4n +
        2 * M1 * pow<4>(M2) * s23 * s4n + M1 * pow<2>(M2) * s15 * s23 * s4n +
        M1 * pow<2>(M2) * s16 * s23 * s4n - 2 * M1 * pow<2>(M2) * s13 * s25 * s4n -
        2 * M1 * pow<2>(M2) * s13 * s26 * s4n + 2 * M1 * pow<4>(M2) * s35 * s4n +
        M1 * pow<2>(M2) * s12 * s35 * s4n + M1 * pow<2>(M2) * s16 * s35 * s4n +
        M1 * pow<2>(M2) * s26 * s35 * s4n + 4 * M1 * pow<4>(M2) * s36 * s4n +
        M1 * pow<2>(M2) * s12 * s36 * s4n + M1 * pow<2>(M2) * s15 * s36 * s4n +
        M1 * s16 * s25 * s36 * s4n + M1 * pow<2>(M2) * s26 * s36 * s4n +
        2 * pow<2>(M2) * s13 * s14 * s56 - pow<2>(M2) * s14 * s23 * s56 +
        pow<2>(M1) * s13 * s24 * s56 + pow<2>(M2) * s13 * s24 * s56 +
        s13 * s16 * s24 * s56 - 4 * pow<2>(M1) * pow<2>(M2) * s34 * s56 -
        pow<2>(M2) * s12 * s34 * s56 - pow<2>(M2) * s16 * s34 * s56 +
        M1 * pow<2>(M2) * s2n * s34 * s56 - pow<2>(M2) * s14 * s36 * s56 -
        2 * pow<2>(M1) * s24 * s36 * s56 - s16 * s24 * s36 * s56 -
        pow<3>(M1) * s24 * s3n * s56 - M1 * pow<2>(M2) * s24 * s3n * s56 -
        2 * M1 * pow<2>(M2) * s13 * s4n * s56 + M1 * pow<2>(M2) * s23 * s4n * s56 +
        M1 * pow<2>(M2) * s36 * s4n * s56 + M1 * pow<2>(M2) * s13 * s24 * s5n +
        2 * M1 * pow<4>(M2) * s34 * s5n - M1 * pow<2>(M2) * s12 * s34 * s5n -
        M1 * pow<2>(M2) * s16 * s34 * s5n + M1 * pow<2>(M2) * s26 * s34 * s5n -
        M1 * s16 * s24 * s36 * s5n + M1 * pow<2>(M2) * s13 * s46 * s5n -
        M1 * pow<2>(M2) * s23 * s46 * s5n - M1 * pow<2>(M2) * s36 * s46 * s5n -
        M1 * pow<2>(M2) * s13 * s24 * s6n + 4 * M1 * pow<4>(M2) * s34 * s6n -
        M1 * pow<2>(M2) * s12 * s34 * s6n - M1 * pow<2>(M2) * s15 * s34 * s6n -
        M1 * s16 * s25 * s34 * s6n + M1 * pow<2>(M2) * s26 * s34 * s6n +
        M1 * s16 * s24 * s35 * s6n + 2 * M1 * pow<2>(M2) * s24 * s36 * s6n -
        M1 * pow<2>(M2) * s13 * s45 * s6n + M1 * s16 * s23 * s45 * s6n -
        M1 * s13 * s26 * s45 * s6n + 2 * M1 * pow<2>(M2) * s36 * s45 * s6n +
        M1 * s26 * s36 * s45 * s6n - M1 * pow<2>(M2) * s23 * s46 * s6n +
        M1 * s13 * s25 * s46 * s6n - M1 * pow<2>(M2) * s35 * s46 * s6n -
        M1 * s25 * s36 * s46 * s6n - M1 * s13 * s24 * s56 * s6n +
        M1 * pow<2>(M2) * s34 * s56 * s6n + M1 * s24 * s36 * s56 * s6n;
    const auto if14 =
        -4 * pow<4>(M2) * s14 * s23 - 2 * pow<2>(M2) * s12 * s13 * s24 +
        4 * pow<2>(M2) * s13 * s15 * s24 - 4 * pow<2>(M2) * s13 * s16 * s24 -
        2 * pow<2>(M2) * s16 * s23 * s24 + 2 * pow<2>(M2) * s13 * s14 * s25 +
        s13 * s16 * s24 * s25 + s13 * s15 * s24 * s26 + 4 * pow<4>(M2) * s12 * s34 -
        2 * pow<4>(M2) * s15 * s34 + 2 * pow<4>(M2) * s16 * s34 -
        pow<2>(M2) * s15 * s25 * s34 + pow<2>(M2) * s16 * s25 * s34 -
        pow<2>(M2) * s15 * s26 * s34 - pow<2>(M2) * s16 * s26 * s34 -
        4 * M1 * pow<4>(M2) * s2n * s34 + 2 * pow<4>(M2) * s14 * s35 +
        4 * pow<2>(M2) * s16 * s24 * s35 + pow<2>(M2) * s14 * s25 * s35 +
        pow<2>(M2) * s14 * s26 * s35 + s16 * s24 * s26 * s35 -
        2 * pow<4>(M2) * s14 * s36 + 2 * pow<2>(M2) * s12 * s24 * s36 -
        4 * pow<2>(M2) * s15 * s24 * s36 - pow<2>(M2) * s14 * s25 * s36 +
        pow<2>(M2) * s14 * s26 * s36 - s15 * s24 * s26 * s36 -
        2 * M1 * pow<2>(M2) * s24 * s2n * s36 +
        2 * M1 * pow<2>(M2) * s12 * s24 * s3n -
        4 * M1 * pow<2>(M2) * s15 * s24 * s3n +
        4 * M1 * pow<2>(M2) * s16 * s24 * s3n -
        2 * M1 * pow<2>(M2) * s14 * s25 * s3n - M1 * s16 * s24 * s25 * s3n -
        M1 * s15 * s24 * s26 * s3n - 2 * pow<2>(M2) * s12 * s13 * s45 +
        2 * pow<2>(M2) * s13 * s15 * s45 - 2 * pow<2>(M2) * s13 * s16 * s45 +
        pow<2>(M2) * s15 * s23 * s45 - pow<2>(M2) * s16 * s23 * s45 +
        s13 * s15 * s26 * s45 - pow<2>(M2) * s12 * s35 * s45 +
        pow<2>(M2) * s16 * s35 * s45 + s16 * s26 * s35 * s45 +
        M1 * pow<2>(M2) * s2n * s35 * s45 + pow<2>(M2) * s12 * s36 * s45 -
        pow<2>(M2) * s15 * s36 * s45 - s15 * s26 * s36 * s45 -
        M1 * pow<2>(M2) * s2n * s36 * s45 + 2 * M1 * pow<2>(M2) * s12 * s3n * s45 -
        2 * M1 * pow<2>(M2) * s15 * s3n * s45 +
        2 * M1 * pow<2>(M2) * s16 * s3n * s45 - M1 * s15 * s26 * s3n * s45 +
        2 * pow<2>(M2) * s13 * s15 * s46 - 2 * pow<2>(M2) * s13 * s16 * s46 +
        pow<2>(M2) * s15 * s23 * s46 + pow<2>(M2) * s16 * s23 * s46 -
        s13 * s15 * s25 * s46 - pow<2>(M2) * s12 * s35 * s46 +
        pow<2>(M2) * s16 * s35 * s46 - s16 * s25 * s35 * s46 +
        M1 * pow<2>(M2) * s2n * s35 * s46 - pow<2>(M2) * s12 * s36 * s46 -
        pow<2>(M2) * s15 * s36 * s46 + s15 * s25 * s36 * s46 +
        M1 * pow<2>(M2) * s2n * s36 * s46 - 2 * M1 * pow<2>(M2) * s15 * s3n * s46 +
        2 * M1 * pow<2>(M2) * s16 * s3n * s46 + M1 * s15 * s25 * s3n * s46 +
        4 * M1 * pow<4>(M2) * s23 * s4n - 2 * M1 * pow<4>(M2) * s35 * s4n -
        M1 * pow<2>(M2) * s25 * s35 * s4n - M1 * pow<2>(M2) * s26 * s35 * s4n +
        2 * M1 * pow<4>(M2) * s36 * s4n + M1 * pow<2>(M2) * s25 * s36 * s4n -
        M1 * pow<2>(M2) * s26 * s36 * s4n - 2 * pow<2>(M2) * s14 * s23 * s56 -
        s12 * s13 * s24 * s56 - s16 * s23 * s24 * s56 + s13 * s14 * s25 * s56 +
        2 * pow<2>(M2) * s12 * s34 * s56 - pow<2>(M2) * s15 * s34 * s56 +
        pow<2>(M2) * s16 * s34 * s56 + s16 * s25 * s34 * s56 -
        2 * M1 * pow<2>(M2) * s2n * s34 * s56 + pow<2>(M2) * s14 * s35 * s56 -
        pow<2>(M2) * s14 * s36 * s56 + s12 * s24 * s36 * s56 -
        s14 * s25 * s36 * s56 - M1 * s24 * s2n * s36 * s56 +
        M1 * s12 * s24 * s3n * s56 - M1 * s14 * s25 * s3n * s56 -
        s12 * s13 * s45 * s56 - s16 * s23 * s45 * s56 + s12 * s36 * s45 * s56 -
        M1 * s2n * s36 * s45 * s56 + M1 * s12 * s3n * s45 * s56 +
        2 * M1 * pow<2>(M2) * s23 * s4n * s56 - M1 * pow<2>(M2) * s35 * s4n * s56 +
        M1 * pow<2>(M2) * s36 * s4n * s56 + M1 * s25 * s36 * s4n * s56 +
        2 * M1 * pow<4>(M2) * s34 * s5n + M1 * pow<2>(M2) * s25 * s34 * s5n +
        M1 * pow<2>(M2) * s26 * s34 * s5n + 4 * M1 * pow<2>(M2) * s24 * s36 * s5n +
        M1 * s24 * s26 * s36 * s5n - M1 * pow<2>(M2) * s23 * s45 * s5n +
        M1 * pow<2>(M2) * s36 * s45 * s5n + M1 * s26 * s36 * s45 * s5n -
        M1 * pow<2>(M2) * s23 * s46 * s5n + M1 * pow<2>(M2) * s36 * s46 * s5n -
        M1 * s25 * s36 * s46 * s5n + M1 * pow<2>(M2) * s34 * s56 * s5n +
        2 * M1 * pow<2>(M2) * s23 * s24 * s6n - 2 * M1 * pow<4>(M2) * s34 * s6n -
        M1 * pow<2>(M2) * s25 * s34 * s6n + M1 * pow<2>(M2) * s26 * s34 * s6n -
        4 * M1 * pow<2>(M2) * s24 * s35 * s6n - M1 * s24 * s26 * s35 * s6n +
        M1 * pow<2>(M2) * s23 * s45 * s6n - M1 * pow<2>(M2) * s35 * s45 * s6n -
        M1 * s26 * s35 * s45 * s6n - M1 * pow<2>(M2) * s23 * s46 * s6n -
        M1 * pow<2>(M2) * s35 * s46 * s6n + M1 * s25 * s35 * s46 * s6n +
        M1 * s23 * s24 * s56 * s6n - M1 * pow<2>(M2) * s34 * s56 * s6n -
        M1 * s25 * s34 * s56 * s6n + M1 * s23 * s45 * s56 * s6n;
    const auto if23 =
        2 * pow<4>(M2) * s14 * s23 + 2 * pow<2>(M2) * s12 * s13 * s24 -
        2 * pow<2>(M2) * s13 * s15 * s24 - 2 * pow<2>(M2) * s13 * s16 * s24 -
        pow<2>(M2) * s15 * s23 * s24 + pow<2>(M2) * s16 * s23 * s24 +
        2 * pow<2>(M2) * s13 * s14 * s25 + pow<2>(M2) * s14 * s23 * s25 +
        pow<2>(M2) * s14 * s23 * s26 - s13 * s15 * s24 * s26 +
        s13 * s14 * s25 * s26 - 2 * pow<4>(M2) * s12 * s34 +
        4 * pow<4>(M2) * s15 * s34 + 2 * pow<4>(M2) * s16 * s34 -
        pow<2>(M2) * s12 * s25 * s34 + pow<2>(M2) * s16 * s25 * s34 -
        pow<2>(M2) * s12 * s26 * s34 + 2 * pow<2>(M2) * s15 * s26 * s34 +
        pow<2>(M2) * s16 * s26 * s34 + s16 * s25 * s26 * s34 +
        2 * M1 * pow<4>(M2) * s2n * s34 + M1 * pow<2>(M2) * s25 * s2n * s34 +
        M1 * pow<2>(M2) * s26 * s2n * s34 - 4 * pow<4>(M2) * s14 * s35 +
        pow<2>(M2) * s12 * s24 * s35 - pow<2>(M2) * s16 * s24 * s35 -
        2 * pow<2>(M2) * s14 * s26 * s35 - s16 * s24 * s26 * s35 -
        M1 * pow<2>(M2) * s24 * s2n * s35 - 2 * pow<4>(M2) * s14 * s36 -
        pow<2>(M2) * s12 * s24 * s36 + pow<2>(M2) * s15 * s24 * s36 -
        pow<2>(M2) * s14 * s25 * s36 - pow<2>(M2) * s14 * s26 * s36 +
        s15 * s24 * s26 * s36 - s14 * s25 * s26 * s36 +
        M1 * pow<2>(M2) * s24 * s2n * s36 - 2 * M1 * pow<2>(M2) * s12 * s24 * s3n +
        2 * M1 * pow<2>(M2) * s15 * s24 * s3n +
        2 * M1 * pow<2>(M2) * s16 * s24 * s3n -
        2 * M1 * pow<2>(M2) * s14 * s25 * s3n + M1 * s15 * s24 * s26 * s3n -
        M1 * s14 * s25 * s26 * s3n + 4 * pow<2>(M2) * s12 * s13 * s45 -
        2 * pow<2>(M2) * s13 * s15 * s45 - 4 * pow<2>(M2) * s13 * s16 * s45 +
        4 * pow<2>(M2) * s16 * s23 * s45 + s13 * s16 * s25 * s45 -
        s13 * s15 * s26 * s45 - 2 * pow<2>(M2) * s16 * s35 * s45 -
        s16 * s26 * s35 * s45 - 4 * pow<2>(M2) * s12 * s36 * s45 +
        2 * pow<2>(M2) * s15 * s36 * s45 + s15 * s26 * s36 * s45 +
        4 * M1 * pow<2>(M2) * s2n * s36 * s45 -
        4 * M1 * pow<2>(M2) * s12 * s3n * s45 +
        2 * M1 * pow<2>(M2) * s15 * s3n * s45 +
        4 * M1 * pow<2>(M2) * s16 * s3n * s45 - M1 * s16 * s25 * s3n * s45 +
        M1 * s15 * s26 * s3n * s45 + 2 * pow<2>(M2) * s12 * s13 * s46 -
        2 * pow<2>(M2) * s13 * s16 * s46 - pow<2>(M2) * s15 * s23 * s46 +
        pow<2>(M2) * s16 * s23 * s46 - s12 * s13 * s25 * s46 -
        s16 * s23 * s25 * s46 + pow<2>(M2) * s12 * s35 * s46 +
        pow<2>(M2) * s16 * s35 * s46 - M1 * pow<2>(M2) * s2n * s35 * s46 -
        pow<2>(M2) * s12 * s36 * s46 - pow<2>(M2) * s15 * s36 * s46 +
        s12 * s25 * s36 * s46 + M1 * pow<2>(M2) * s2n * s36 * s46 -
        M1 * s25 * s2n * s36 * s46 - 2 * M1 * pow<2>(M2) * s12 * s3n * s46 +
        2 * M1 * pow<2>(M2) * s16 * s3n * s46 + M1 * s12 * s25 * s3n * s46 -
        2 * M1 * pow<4>(M2) * s23 * s4n - M1 * pow<2>(M2) * s23 * s25 * s4n -
        M1 * pow<2>(M2) * s23 * s26 * s4n + 4 * M1 * pow<4>(M2) * s35 * s4n +
        2 * M1 * pow<2>(M2) * s26 * s35 * s4n + 2 * M1 * pow<4>(M2) * s36 * s4n +
        M1 * pow<2>(M2) * s25 * s36 * s4n + M1 * pow<2>(M2) * s26 * s36 * s4n +
        M1 * s25 * s26 * s36 * s4n + pow<2>(M2) * s14 * s23 * s56 +
        s12 * s13 * s24 * s56 + s16 * s23 * s24 * s56 -
        pow<2>(M2) * s12 * s34 * s56 - pow<2>(M2) * s16 * s34 * s56 +
        M1 * pow<2>(M2) * s2n * s34 * s56 + pow<2>(M2) * s14 * s36 * s56 -
        s12 * s24 * s36 * s56 + M1 * s24 * s2n * s36 * s56 -
        M1 * s12 * s24 * s3n * s56 + s12 * s13 * s45 * s56 + s16 * s23 * s45 * s56 -
        s12 * s36 * s45 * s56 + M1 * s2n * s36 * s45 * s56 -
        M1 * s12 * s3n * s45 * s56 - M1 * pow<2>(M2) * s23 * s4n * s56 -
        M1 * pow<2>(M2) * s36 * s4n * s56 + M1 * pow<2>(M2) * s23 * s24 * s5n -
        4 * M1 * pow<4>(M2) * s34 * s5n - 2 * M1 * pow<2>(M2) * s26 * s34 * s5n -
        M1 * pow<2>(M2) * s24 * s36 * s5n - M1 * s24 * s26 * s36 * s5n -
        2 * M1 * pow<2>(M2) * s36 * s45 * s5n - M1 * s26 * s36 * s45 * s5n +
        M1 * pow<2>(M2) * s23 * s46 * s5n + M1 * pow<2>(M2) * s36 * s46 * s5n -
        M1 * pow<2>(M2) * s23 * s24 * s6n - 2 * M1 * pow<4>(M2) * s34 * s6n -
        M1 * pow<2>(M2) * s25 * s34 * s6n - M1 * pow<2>(M2) * s26 * s34 * s6n -
        M1 * s25 * s26 * s34 * s6n + M1 * pow<2>(M2) * s24 * s35 * s6n +
        M1 * s24 * s26 * s35 * s6n - 4 * M1 * pow<2>(M2) * s23 * s45 * s6n +
        2 * M1 * pow<2>(M2) * s35 * s45 * s6n + M1 * s26 * s35 * s45 * s6n -
        M1 * pow<2>(M2) * s23 * s46 * s6n + M1 * s23 * s25 * s46 * s6n -
        M1 * pow<2>(M2) * s35 * s46 * s6n - M1 * s23 * s24 * s56 * s6n +
        M1 * pow<2>(M2) * s34 * s56 * s6n - M1 * s23 * s45 * s56 * s6n;
    const auto if24 =
        -8 * pow<4>(M2) * s13 * s24 + 8 * pow<2>(M2) * s13 * s24 * s25 -
        4 * pow<2>(M2) * s13 * s24 * s26 + 2 * s13 * s24 * s25 * s26 +
        8 * M1 * pow<4>(M2) * s24 * s3n - 8 * M1 * pow<2>(M2) * s24 * s25 * s3n +
        4 * M1 * pow<2>(M2) * s24 * s26 * s3n - 2 * M1 * s24 * s25 * s26 * s3n -
        8 * pow<4>(M2) * s13 * s45 + 8 * pow<2>(M2) * s13 * s25 * s45 -
        4 * pow<2>(M2) * s13 * s26 * s45 + 2 * s13 * s25 * s26 * s45 +
        8 * M1 * pow<4>(M2) * s3n * s45 - 8 * M1 * pow<2>(M2) * s25 * s3n * s45 +
        4 * M1 * pow<2>(M2) * s26 * s3n * s45 - 2 * M1 * s25 * s26 * s3n * s45 +
        8 * pow<2>(M2) * s13 * s25 * s46 - 2 * s13 * pow<2>(s25) * s46 -
        8 * M1 * pow<2>(M2) * s25 * s3n * s46 + 2 * M1 * pow<2>(s25) * s3n * s46 -
        4 * pow<2>(M2) * s13 * s24 * s56 + 2 * s13 * s24 * s25 * s56 +
        4 * M1 * pow<2>(M2) * s24 * s3n * s56 - 2 * M1 * s24 * s25 * s3n * s56 -
        4 * pow<2>(M2) * s13 * s45 * s56 + 2 * s13 * s25 * s45 * s56 +
        4 * M1 * pow<2>(M2) * s3n * s45 * s56 - 2 * M1 * s25 * s3n * s45 * s56;
    const auto if34 =
        -2 * pow<2>(M2) * s13 * s15 * s24 - 2 * pow<2>(M2) * s15 * s23 * s24 +
        2 * pow<2>(M2) * s13 * s14 * s25 + 2 * pow<2>(M2) * s14 * s23 * s25 -
        s13 * s15 * s24 * s26 + s13 * s14 * s25 * s26 + 8 * pow<4>(M2) * s15 * s34 -
        2 * pow<2>(M2) * s12 * s25 * s34 + 4 * pow<2>(M2) * s15 * s26 * s34 +
        s16 * s25 * s26 * s34 + 2 * M1 * pow<2>(M2) * s25 * s2n * s34 -
        8 * pow<4>(M2) * s14 * s35 + 2 * pow<2>(M2) * s12 * s24 * s35 -
        4 * pow<2>(M2) * s14 * s26 * s35 - s16 * s24 * s26 * s35 -
        2 * M1 * pow<2>(M2) * s24 * s2n * s35 + s15 * s24 * s26 * s36 -
        s14 * s25 * s26 * s36 + 2 * M1 * pow<2>(M2) * s15 * s24 * s3n -
        2 * M1 * pow<2>(M2) * s14 * s25 * s3n + M1 * s15 * s24 * s26 * s3n -
        M1 * s14 * s25 * s26 * s3n - 4 * pow<2>(M2) * s13 * s15 * s45 +
        2 * pow<2>(M2) * s15 * s23 * s45 + 2 * s13 * s16 * s25 * s45 -
        s16 * s23 * s25 * s45 - 2 * s13 * s15 * s26 * s45 + s15 * s23 * s26 * s45 -
        2 * pow<2>(M2) * s12 * s35 * s45 - 2 * pow<2>(M2) * s16 * s35 * s45 -
        s12 * s26 * s35 * s45 - s16 * s26 * s35 * s45 +
        2 * M1 * pow<2>(M2) * s2n * s35 * s45 + M1 * s26 * s2n * s35 * s45 +
        2 * pow<2>(M2) * s15 * s36 * s45 + s12 * s25 * s36 * s45 +
        s15 * s26 * s36 * s45 - M1 * s25 * s2n * s36 * s45 +
        4 * M1 * pow<2>(M2) * s15 * s3n * s45 - 2 * M1 * s16 * s25 * s3n * s45 +
        2 * M1 * s15 * s26 * s3n * s45 - 2 * pow<2>(M2) * s13 * s15 * s46 -
        s12 * s13 * s25 * s46 + s13 * s16 * s25 * s46 - 2 * s16 * s23 * s25 * s46 -
        s13 * s15 * s26 * s46 + s15 * s23 * s26 * s46 +
        2 * pow<2>(M2) * s16 * s35 * s46 - s12 * s26 * s35 * s46 +
        M1 * s26 * s2n * s35 * s46 - 2 * pow<2>(M2) * s15 * s36 * s46 +
        2 * s12 * s25 * s36 * s46 - 2 * M1 * s25 * s2n * s36 * s46 +
        2 * M1 * pow<2>(M2) * s15 * s3n * s46 + M1 * s12 * s25 * s3n * s46 -
        M1 * s16 * s25 * s3n * s46 + M1 * s15 * s26 * s3n * s46 -
        2 * M1 * pow<2>(M2) * s23 * s25 * s4n + 8 * M1 * pow<4>(M2) * s35 * s4n +
        4 * M1 * pow<2>(M2) * s26 * s35 * s4n + M1 * s25 * s26 * s36 * s4n +
        2 * pow<2>(M2) * s13 * s14 * s56 + s12 * s13 * s24 * s56 -
        s13 * s16 * s24 * s56 + 2 * s16 * s23 * s24 * s56 + s13 * s14 * s26 * s56 -
        s14 * s23 * s26 * s56 - 2 * pow<2>(M2) * s16 * s34 * s56 +
        s12 * s26 * s34 * s56 - M1 * s26 * s2n * s34 * s56 +
        2 * pow<2>(M2) * s14 * s36 * s56 - 2 * s12 * s24 * s36 * s56 +
        2 * M1 * s24 * s2n * s36 * s56 - 2 * M1 * pow<2>(M2) * s14 * s3n * s56 -
        M1 * s12 * s24 * s3n * s56 + M1 * s16 * s24 * s3n * s56 -
        M1 * s14 * s26 * s3n * s56 + 2 * s12 * s13 * s45 * s56 +
        s16 * s23 * s45 * s56 - s12 * s36 * s45 * s56 + M1 * s2n * s36 * s45 * s56 -
        2 * M1 * s12 * s3n * s45 * s56 + M1 * s23 * s26 * s4n * s56 -
        2 * M1 * pow<2>(M2) * s36 * s4n * s56 +
        2 * M1 * pow<2>(M2) * s23 * s24 * s5n - 8 * M1 * pow<4>(M2) * s34 * s5n -
        4 * M1 * pow<2>(M2) * s26 * s34 * s5n - M1 * s24 * s26 * s36 * s5n -
        2 * M1 * pow<2>(M2) * s23 * s45 * s5n - M1 * s23 * s26 * s45 * s5n -
        2 * M1 * pow<2>(M2) * s36 * s45 * s5n - M1 * s26 * s36 * s45 * s5n -
        M1 * s23 * s26 * s46 * s5n + 2 * M1 * pow<2>(M2) * s36 * s46 * s5n -
        M1 * s25 * s26 * s34 * s6n + M1 * s24 * s26 * s35 * s6n +
        M1 * s23 * s25 * s45 * s6n + 2 * M1 * pow<2>(M2) * s35 * s45 * s6n +
        M1 * s26 * s35 * s45 * s6n + 2 * M1 * s23 * s25 * s46 * s6n -
        2 * M1 * pow<2>(M2) * s35 * s46 * s6n - 2 * M1 * s23 * s24 * s56 * s6n +
        2 * M1 * pow<2>(M2) * s34 * s56 * s6n - M1 * s23 * s45 * s56 * s6n;

    const auto den1 = (2 * pow<2>(M2) + s56) * (2 * pow<2>(M2) - s15 - s16 + s56);
    const auto den2 = (2 * pow<2>(M2) + s56) * (2 * pow<2>(M2) + s25 + s26 + s56);
    const auto den3 = (2 * pow<2>(M2) + s26) * (2 * pow<2>(M2) - s12 - s16 + s26);
    const auto den4 = (2 * pow<2>(M2) + s26) * (2 * pow<2>(M2) + s25 + s26 + s56);

    return if11 / pow<2>(den1) + if22 / pow<2>(den2) + if33 / pow<2>(den3) +
           if44 / pow<2>(den4) + if12 / (den1 * den2) + if13 / (den1 * den3) +
           if14 / (den1 * den4) + if23 / (den2 * den3) +
           if24 / (den2 * den4) + if34 / (den3 * den4);
}

auto MuonInternalConversionDecayChannel::MSqRR2009PRD(const GENBOD<5>::State& state) -> double {
    // Tree level mu -> eeevv (2 diagrams)
    // Ref: Rashid M. Djilkibaev, and Rostislav V. Konoplich, Rare muon decay mu+->e+e-e+vevmu, Phys. Rev. D 79, 073004 (arXiv:0812.1355)
    // Adapt from mu3e2nu.tex in https://arxiv.org/src/0812.1355

    const auto& [p, p1, p2, k1, k2]{state};

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

    const auto tr11{-(
        qk2 *
        (p2k1 *
             (pp12 - pp1 * (m2 + pp2) + m2 * (m2 + p1p2) - pp2 * (2. * m2 + p1p2)) +
         p1k1 * (m4 - m2 * pp2 + pp22 + m2 * p1p2 - pp1 * (2. * m2 + pp2 + p1p2)) +
         pk1 * ((2. * m2 - pp2) * (m2 + p1p2) - pp1 * (m2 + 2. * pp2 + p1p2))))};
    const auto tr12{
        m2 * pk1 * p1k2 * qp - m2 * p1k1 * p1k2 * qp + m2 * pk1 * p2k2 * qp -
        m2 * p2k1 * p2k2 * qp - 2. * m2 * pk1 * qk2 * qp - m2 * p1k1 * qk2 * qp -
        m2 * p2k1 * qk2 * qp + pk1 * p1k2 * qp * p1p2 + p2k1 * p1k2 * qp * p1p2 +
        pk1 * p2k2 * qp * p1p2 + p1k1 * p2k2 * qp * p1p2 -
        2. * pk1 * qk2 * qp * p1p2 - p1k1 * qk2 * qp * p1p2 -
        p2k1 * qk2 * qp * p1p2 +
        qk1 * (m2 * qk2 * pp1 + m2 * p2k2 * pp2 + m2 * qk2 * pp2 -
               p2k2 * pp1 * p1p2 + qk2 * pp1 * p1p2 + qk2 * pp2 * p1p2 -
               2. * m2 * pk2 * (m2 + p1p2) + p1k2 * (m2 * pp1 - pp2 * p1p2)) -
        m2 * pk1 * pk2 * qp1 + m2 * p1k1 * pk2 * qp1 + pk1 * p2k2 * pp1 * qp1 +
        2. * p2k1 * p2k2 * pp1 * qp1 - p2k1 * qk2 * pp1 * qp1 -
        pk1 * p2k2 * pp2 * qp1 - 2. * p1k1 * p2k2 * pp2 * qp1 +
        2. * pk1 * qk2 * pp2 * qp1 + p1k1 * qk2 * pp2 * qp1 -
        pk1 * pk2 * p1p2 * qp1 - p2k1 * pk2 * p1p2 * qp1 - m2 * pk1 * pk2 * qp2 +
        m2 * p2k1 * pk2 * qp2 - pk1 * p1k2 * pp1 * qp2 -
        2. * p2k1 * p1k2 * pp1 * qp2 + 2. * pk1 * qk2 * pp1 * qp2 +
        p2k1 * qk2 * pp1 * qp2 + pk1 * p1k2 * pp2 * qp2 +
        2. * p1k1 * p1k2 * pp2 * qp2 - p1k1 * qk2 * pp2 * qp2 -
        pk1 * pk2 * p1p2 * qp2 - p1k1 * pk2 * p1p2 * qp2 +
        k1k2 * (2. * m2 * qp * (m2 + p1p2) + pp2 * (p1p2 * qp1 - m2 * qp2) +
                pp1 * (-(m2 * qp1) + p1p2 * qp2))};
    const auto tr13{
        2. * qk2 *
        (p1k1 * pp2 * (-2. * m2 + pp2) +
         pk1 * (pp1 * (m2 - pp2) + m2 * (m2 + p1p2) - pp2 * (2. * m2 + p1p2)) +
         p2k1 * (pp1 * (m2 - pp2) + m2 * (m2 + p1p2) - pp2 * (2. * m2 + p1p2)))};
    const auto tr14{
        (m2 * pk1 * p1k2 * qp + m2 * p1k1 * p1k2 * qp + 4. * m2 * p2k1 * p1k2 * qp -
         m2 * pk1 * p2k2 * qp - m2 * p1k1 * p2k2 * qp - 2. * m2 * pk1 * qk2 * qp -
         2. * m2 * p1k1 * qk2 * qp - 4. * m2 * p2k1 * qk2 * qp -
         2. * p1k1 * p1k2 * pp2 * qp + 2. * p1k1 * qk2 * pp2 * qp +
         2. * pk1 * p1k2 * qp * p1p2 + 2. * p2k1 * p1k2 * qp * p1p2 -
         2. * pk1 * qk2 * qp * p1p2 - 2. * p2k1 * qk2 * qp * p1p2 -
         qk1 * (-2. * (m2 + pp1) * (m2 * p2k2 - qk2 * pp2) -
                p1k2 * (pp1 * (m2 + 2. * pp2) + m2 * (m2 + pp2 - p1p2)) +
                m2 * pk2 * (m2 + pp1 + pp2 + p1p2)) -
         m2 * pk1 * pk2 * qp1 - m2 * p1k1 * pk2 * qp1 - 4. * m2 * p2k1 * pk2 * qp1 +
         m2 * pk1 * p2k2 * qp1 - m2 * p1k1 * p2k2 * qp1 +
         2. * m2 * p2k1 * p2k2 * qp1 + 2. * m2 * pk1 * qk2 * qp1 +
         2. * m2 * p1k1 * qk2 * qp1 + 4. * m2 * p2k1 * qk2 * qp1 +
         2. * pk1 * p2k2 * pp1 * qp1 + 2. * p2k1 * p2k2 * pp1 * qp1 +
         2. * p1k1 * pk2 * pp2 * qp1 - 2. * p2k1 * qk2 * pp2 * qp1 -
         2. * pk1 * pk2 * p1p2 * qp1 - 2. * p2k1 * pk2 * p1p2 * qp1 +
         m2 * pk1 * pk2 * qp2 + m2 * p1k1 * pk2 * qp2 - m2 * pk1 * p1k2 * qp2 +
         m2 * p1k1 * p1k2 * qp2 - 2. * m2 * p2k1 * p1k2 * qp2 +
         2. * m2 * pk1 * qk2 * qp2 + 2. * m2 * p2k1 * qk2 * qp2 -
         2. * pk1 * p1k2 * pp1 * qp2 - 2. * p2k1 * p1k2 * pp1 * qp2 +
         2. * pk1 * qk2 * pp1 * qp2 + 2. * p2k1 * qk2 * pp1 * qp2 +
         k1k2 * (m2 * qp * (m2 + pp1 + pp2 + p1p2) -
                 (pp1 * (m2 + 2. * pp2) + m2 * (m2 + pp2 - p1p2)) * qp1 -
                 2. * m2 * (m2 + pp1) * qp2)) /
        2.0};
    const auto tr22{-(pk1 * (-(p1k2 * (m2 * u2 + p1p2 * (u2 + qp1) +
                                       qp1 * (2. * m2 - qp2) + m2 * qp2 + qp22)) +
                             qk2 * (qp1 * (m2 - 2. * qp2) + m2 * (m2 + u2 + qp2) +
                                    p1p2 * (m2 + u2 + qp1 + qp2)) -
                             p2k2 * (qp12 + qp1 * (m2 - qp2) + p1p2 * (u2 + qp2) +
                                     m2 * (u2 + 2. * qp2))))};
    const auto tr23{
        (-2. * m2 * pk1 * p1k2 * qp + m2 * p1k1 * p1k2 * qp -
         m2 * p2k1 * p1k2 * qp + m2 * p1k1 * p2k2 * qp + m2 * p2k1 * p2k2 * qp +
         2. * m2 * pk1 * qk2 * qp + 2. * m2 * p2k1 * qk2 * qp -
         2. * pk1 * p1k2 * qp * p1p2 - 2. * p2k1 * p1k2 * qp * p1p2 +
         2. * pk1 * qk2 * qp * p1p2 + 2. * p2k1 * qk2 * qp * p1p2 -
         qk1 * (-2. * (m2 * pk2 - qk2 * pp2) * (m2 + p1p2) +
                m2 * p2k2 * (m2 + pp1 + pp2 + p1p2) -
                p1k2 * (m2 * (m2 - pp1 + pp2) + (m2 + 2. * pp2) * p1p2)) +
         2. * m2 * pk1 * pk2 * qp1 - m2 * p1k1 * pk2 * qp1 + m2 * p2k1 * pk2 * qp1 -
         4. * m2 * pk1 * p2k2 * qp1 - m2 * p1k1 * p2k2 * qp1 -
         m2 * p2k1 * p2k2 * qp1 + 4. * m2 * pk1 * qk2 * qp1 +
         2. * m2 * p1k1 * qk2 * qp1 + 2. * m2 * p2k1 * qk2 * qp1 -
         2. * pk1 * p2k2 * pp1 * qp1 - 2. * p2k1 * p2k2 * pp1 * qp1 +
         2. * p1k1 * p2k2 * pp2 * qp1 - 2. * pk1 * qk2 * pp2 * qp1 +
         2. * pk1 * pk2 * p1p2 * qp1 + 2. * p2k1 * pk2 * p1p2 * qp1 -
         m2 * p1k1 * pk2 * qp2 - m2 * p2k1 * pk2 * qp2 +
         4. * m2 * pk1 * p1k2 * qp2 + m2 * p1k1 * p1k2 * qp2 +
         m2 * p2k1 * p1k2 * qp2 - 4. * m2 * pk1 * qk2 * qp2 -
         2. * m2 * p1k1 * qk2 * qp2 - 2. * m2 * p2k1 * qk2 * qp2 +
         2. * pk1 * p1k2 * pp1 * qp2 + 2. * p2k1 * p1k2 * pp1 * qp2 -
         2. * pk1 * qk2 * pp1 * qp2 - 2. * p2k1 * qk2 * pp1 * qp2 -
         2. * p1k1 * p1k2 * pp2 * qp2 + 2. * p1k1 * qk2 * pp2 * qp2 +
         k1k2 * (-2. * m2 * qp * (m2 + p1p2) -
                 (m2 * (m2 - pp1 + pp2) + (m2 + 2. * pp2) * p1p2) * qp1 +
                 m2 * (m2 + pp1 + pp2 + p1p2) * qp2)) /
        2.0};
    const auto tr24{
        (qp1 *
         (-(m2 * p2k1 * pk2) - u2 * p2k1 * pk2 + m2 * qk1 * pk2 + m2 * pk1 * p1k2 +
          m2 * p2k1 * p1k2 - m2 * pk1 * p2k2 - u2 * pk1 * p2k2 + m2 * qk1 * p2k2 -
          m2 * pk1 * qk2 - m2 * p2k1 * qk2 + 2. * p2k1 * p1k2 * pp1 -
          2. * p2k1 * qk2 * pp1 + 2. * qk1 * p1k2 * pp2 - 2. * qk1 * qk2 * pp2 -
          p1k1 * (m2 * pk2 + m2 * p2k2 + 2. * (p1k2 - qk2) * pp2) -
          2. * p2k1 * p1k2 * qp + 2. * p2k1 * qk2 * qp + 2. * pk1 * p1k2 * p1p2 -
          2. * pk1 * qk2 * p1p2 + 2. * p2k1 * pk2 * qp1 + 2. * pk1 * p2k2 * qp1 +
          k1k2 * (m2 * pp1 + pp2 * (m2 + u2 - 2. * qp1) +
                  m2 * (m2 - qp + p1p2 - qp2)) -
          2. * pk1 * p1k2 * qp2 + 2. * pk1 * qk2 * qp2)) /
            2. +
        u2 * ((m2 * pk1 * p1k2 - 2. * m2 * pk1 * p2k2 + m2 * k1k2 * pp1 +
               2. * m2 * k1k2 * pp2 -
               p1k1 * (m2 * pk2 + m2 * p2k2 + 2. * (2. * p1k2 - qk2) * pp2) +
               m2 * k1k2 * p1p2 + 4. * pk1 * p1k2 * p1p2 - 2. * pk1 * qk2 * p1p2 +
               p2k1 * (-2. * qk2 * pp1 + p1k2 * (m2 + 4. * pp1) -
                       2. * pk2 * (m2 - qp1)) +
               2. * pk1 * p2k2 * qp1 - 2. * k1k2 * pp2 * qp1) /
              4.) +
        m2 *
            ((2. * m2 * qk1 * pk2 - u2 * qk1 * pk2 - 2. * u2 * pk1 * p1k2 +
              4. * m2 * qk1 * p1k2 - 2. * u2 * qk1 * p1k2 - 2. * u2 * pk1 * p2k2 +
              2. * m2 * qk1 * p2k2 - u2 * qk1 * p2k2 - 2. * m2 * pk1 * qk2 +
              u2 * pk1 * qk2 - 2. * m2 * p1k1 * qk2 - 4. * m2 * qk1 * qk2 +
              2. * qk1 * p1k2 * pp1 + 2. * qk1 * p2k2 * pp1 - 4. * qk1 * qk2 * pp1 +
              2. * p1k1 * qk2 * pp2 - 4. * qk1 * qk2 * pp2 - 2. * p1k1 * p1k2 * qp +
              2. * qk1 * p1k2 * qp - 2. * p1k1 * p2k2 * qp + 2. * qk1 * p2k2 * qp +
              2. * p1k1 * qk2 * qp + 2. * qk1 * pk2 * p1p2 +
              2. * qk1 * p1k2 * p1p2 - 2. * pk1 * qk2 * p1p2 -
              4. * qk1 * qk2 * p1p2 +
              p2k1 * (qk2 * (-2. * m2 + u2 - 2. * pp1 + 2. * qp) -
                      2. * pk2 * (u2 - qp1) - 2. * p1k2 * (u2 - qp1)) +
              2. * pk1 * p1k2 * qp1 + 2. * pk1 * p2k2 * qp1 + 4. * qk1 * qk2 * qp1 -
              2. * p1k1 * pk2 * qp2 + 2. * qk1 * pk2 * qp2 -
              2. * p1k1 * p1k2 * qp2 + 2. * qk1 * p1k2 * qp2 +
              2. * pk1 * qk2 * qp2 + 2. * p1k1 * qk2 * qp2 +
              k1k2 * (-2. * m2 * u2 + 2. * pp2 * (u2 - qp1) + 2. * m2 * qp1 +
                      qp * (2. * m2 + u2 + 2. * p1p2 - 2. * qp1 - 4. * qp2) +
                      2. * m2 * qp2 + u2 * qp2 + 2. * pp1 * qp2 - 2. * qp1 * qp2)) /
             4.) +
        u2 * m2 *
            ((2. * p2k1 * pk2 + qk1 * pk2 + 3. * pk1 * p1k2 + 3. * p2k1 * p1k2 +
              2. * qk1 * p1k2 + 2. * pk1 * p2k2 + qk1 * p2k2 - 3. * pk1 * qk2 -
              3. * p2k1 * qk2 - p1k1 * (pk2 + p2k2 + 2. * qk2) +
              k1k2 * (6. * m2 + 3. * pp1 - qp + 3. * p1p2 - qp2)) /
             4.)};
    const auto tr33{
        -(qk2 *
          (p1k1 * (m4 + m2 * pp1 - m2 * pp2 + pp22 - (2. * m2 + pp1 + pp2) * p1p2) +
           p2k1 * ((m2 + pp1) * (2. * m2 - pp2) - (m2 + pp1 + 2. * pp2) * p1p2) +
           pk1 * (m2 * (m2 + pp1) - (2. * m2 + pp1) * pp2 - (m2 + pp2) * p1p2 +
                  p1p22)))};
    const auto tr34{
        m2 * pk1 * p2k2 * qp - m2 * p2k1 * p2k2 * qp - p1k1 * p2k2 * pp1 * qp -
        p2k1 * p2k2 * pp1 * qp + 2. * p1k1 * p1k2 * pp2 * qp +
        p2k1 * p1k2 * pp2 * qp - p1k1 * qk2 * pp2 * qp -
        2. * pk1 * p1k2 * qp * p1p2 - p2k1 * p1k2 * qp * p1p2 +
        pk1 * qk2 * qp * p1p2 + 2. * p2k1 * qk2 * qp * p1p2 +
        qk1 * (-2. * m2 * p2k2 * (m2 + pp1) + m2 * pk2 * pp2 + m2 * qk2 * pp2 +
               qk2 * pp1 * pp2 + m2 * qk2 * p1p2 - pk2 * pp1 * p1p2 +
               qk2 * pp1 * p1p2 + p1k2 * (-(pp1 * pp2) + m2 * p1p2)) +
        m2 * p1k1 * p2k2 * qp1 - m2 * p2k1 * p2k2 * qp1 - pk1 * p2k2 * pp1 * qp1 -
        p2k1 * p2k2 * pp1 * qp1 - 2. * p1k1 * pk2 * pp2 * qp1 -
        p2k1 * pk2 * pp2 * qp1 + p1k1 * qk2 * pp2 * qp1 +
        2. * p2k1 * qk2 * pp2 * qp1 + 2. * pk1 * pk2 * p1p2 * qp1 +
        p2k1 * pk2 * p1p2 * qp1 - pk1 * qk2 * p1p2 * qp1 - m2 * pk1 * pk2 * qp2 +
        m2 * p2k1 * pk2 * qp2 - m2 * p1k1 * p1k2 * qp2 + m2 * p2k1 * p1k2 * qp2 -
        m2 * pk1 * qk2 * qp2 - m2 * p1k1 * qk2 * qp2 - 2. * m2 * p2k1 * qk2 * qp2 +
        p1k1 * pk2 * pp1 * qp2 + p2k1 * pk2 * pp1 * qp2 + pk1 * p1k2 * pp1 * qp2 +
        p2k1 * p1k2 * pp1 * qp2 - pk1 * qk2 * pp1 * qp2 - p1k1 * qk2 * pp1 * qp2 -
        2. * p2k1 * qk2 * pp1 * qp2 +
        k1k2 * (p1p2 * (pp1 * qp - m2 * qp1) + pp2 * (-(m2 * qp) + pp1 * qp1) +
                2. * m2 * (m2 + pp1) * qp2)};
    const auto tr44{-(p2k1 * (-(pk2 * (pp1 * (u2 + qp) + m2 * (u2 + 2. * qp) +
                                       (m2 - qp) * qp1 + qp12)) -
                              p1k2 * (m2 * u2 + m2 * qp + qps +
                                      (2. * m2 - qp) * qp1 + pp1 * (u2 + qp1)) +
                              qk2 * (m2 * (m2 + u2 + qp) + (m2 - 2. * qp) * qp1 +
                                     pp1 * (m2 + u2 + qp + qp1))))};

    const auto matr2e{C1 * C1 * D1 * D1 * tr11 - C1 * C1 * D1 * D2 * tr13 +
                      C1 * C1 * D2 * D2 * tr33};
    const auto matr2mu{C2 * C2 * D1 * D1 * tr22 - C2 * C3 * D1 * D2 * tr24 +
                       C3 * C3 * D2 * D2 * tr44};
    const auto matr2emu{C1 * C2 * D1 * D1 * tr12 - C1 * C3 * D1 * D2 * tr14 -
                        C1 * C2 * D1 * D2 * tr23 + C1 * C3 * D2 * D2 * tr34};

    return matr2e + matr2mu + matr2emu;
}

} // namespace Mustard::Geant4X::inline DecayChannel
