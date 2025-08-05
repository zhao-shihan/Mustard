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

namespace Mustard::inline Physics::inline Generator {

template<int N>
MetropolisHastingsGenerator<N>::MetropolisHastingsGenerator(double cmsE, const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                                            double delta, int discard) :
    EventGenerator<N>{},
    fCMSEnergy{cmsE},
    fGENBOD{pdgID, mass},
    fMHDelta{delta},
    fMHDiscard{discard},
    fBias{[](auto&&) { return 1; }},
    fBurntIn{},
    fRandomState{},
    fEvent{},
    fBiasedPDF{} {}

template<int N>
auto MetropolisHastingsGenerator<N>::operator()(double, CLHEP::HepRandomEngine& rng) -> Event {
    BurnIn(rng);
    for (int i{}; i < fMHDiscard; ++i) {
        NextEvent(fMHDelta, rng);
    }
    NextEvent(fMHDelta, rng);
    return fEvent;
}

template<int N>
auto MetropolisHastingsGenerator<N>::CMSEnergy(double cmsE) -> void {
    fCMSEnergy = cmsE;
    BurnInRequired();
}

template<int N>
auto MetropolisHastingsGenerator<N>::Bias(std::function<auto(const Momenta&)->double> B) -> void {
    fBias = std::move(B);
    BurnInRequired();
}

template<int N>
auto MetropolisHastingsGenerator<N>::BurnIn(CLHEP::HepRandomEngine& rng) -> void {
    if (fBurntIn) {
        return;
    }
    // find phase space
    while (true) {
        std::ranges::generate(fRandomState, [&rng] { return rng.flat(); });
        fEvent = fGENBOD(fCMSEnergy, fRandomState);
        if (const auto bias{BiasWithCheck(fEvent.p)};
            bias >= std::numeric_limits<double>::min()) {
            fBiasedPDF = bias * fEvent.weight * SquaredAmplitude(fEvent.p);
            break;
        }
    }
    // burining in
    constexpr auto delta0{0.1};
    constexpr auto epsilon{std::numeric_limits<double>::epsilon()};
    constexpr auto nBurnIn{100000.};
    const auto factor{std::pow(epsilon / delta0, 1 / nBurnIn)};
    for (auto delta{delta0}; delta > epsilon; delta *= factor) {
        NextEvent(delta, rng);
    }
    fBurntIn = true;
}

template<int N>
auto MetropolisHastingsGenerator<N>::NextEvent(double delta, CLHEP::HepRandomEngine& rng) -> void {
    typename GENBOD<N>::RandomState proposedRandomState;
    Event proposedEvent;
    while (true) {
        std::ranges::transform(std::as_const(fRandomState), proposedRandomState.begin(),
                               [&](auto u) {
                                   const auto low{std::clamp(u - delta, 0., 1.)};
                                   const auto up{std::clamp(u + delta, 0., 1.)};
                                   return CLHEP::RandFlat::shoot(&rng, low, up);
                               });
        proposedEvent = fGENBOD(fCMSEnergy, proposedRandomState);
        const auto proposedBias{BiasWithCheck(proposedEvent.p)};
        if (proposedBias <= std::numeric_limits<double>::min()) {
            continue;
        }

        const auto proposedBiasedPDF{proposedBias * proposedEvent.weight * SquaredAmplitude(proposedEvent.p)};
        if (proposedBiasedPDF >= fBiasedPDF or
            proposedBiasedPDF >= fBiasedPDF * rng.flat()) {
            fRandomState = proposedRandomState;
            fEvent = proposedEvent;
            fEvent.weight = 1 / proposedBias;
            fBiasedPDF = proposedBiasedPDF;
            return;
        }
    }
}

template<int N>
auto MetropolisHastingsGenerator<N>::BiasWithCheck(const Momenta& momenta) const -> double {
    const auto bias{fBias(momenta)};
    if (bias < 0) {
        std::string where;
        for (auto&& p : momenta) {
            where += fmt::format("[{}, {}, {}; {}]", p.x(), p.y(), p.z(), p.e());
        }
        Throw<std::runtime_error>(fmt::format("Bias should be non-negative (got {} at {})", bias, where));
    }
    return bias;
}

template<int N>
auto MetropolisHastingsGenerator<N>::EstimateWeightNormalizationFactor(unsigned long long n) -> WeightNormalizationFactor {
    auto originalBias{std::move(fBias)};
    auto originalBurntIn{std::move(fBurntIn)};
    auto originalRandomState{std::move(fRandomState)};
    auto originalEvent{std::move(fEvent)};
    auto originalBiasedPDF{std::move(fBiasedPDF)};
    auto _{gsl::finally([&] {
        fBias = std::move(originalBias);
        fBurntIn = std::move(originalBurntIn);
        fRandomState = std::move(originalRandomState);
        fEvent = std::move(originalEvent);
        fBiasedPDF = std::move(originalBiasedPDF);
    })};

    WeightNormalizationFactor result{.factor = std::numeric_limits<double>::quiet_NaN(),
                                     .error = std::numeric_limits<double>::quiet_NaN(),
                                     .nEff = 0};
    if (n == 0) {
        return result;
    }

    Bias([](auto&&) { return 1; }); // to evaluate the weight normalization factor of user-defined bias, temporarily switch to unbiased function
    BurnIn();

    using namespace Mustard::VectorArithmeticOperator::Vector2ArithmeticOperator;
    muc::array2ld sum{};
    const auto nProc{mplr::comm_world()};
    { // Monte Carlo integration here
        const auto rng{CLHEP::HepRandom::getTheEngine()};
        Parallel::ReseedRandomEngine(rng);
        Executor<unsigned long long> executor{"Estimation", "Sample"};
        muc::array2ld partialSum{}; // improve numeric stability
        const auto partialSumThreshold{muc::llround(std::sqrt(n / executor.NProcess()))};
        executor.Execute(n, [&](auto i) {
            (*this)(*rng);
            const auto bias{originalBias(fEvent.p)};
            partialSum += muc::array2ld{bias, muc::pow<2>(bias)};
            if ((i + 1) % partialSumThreshold == 0) {
                sum += partialSum;
                partialSum = {};
            }
        });
        sum += partialSum;
    }
    if (mplr::available()) {
        mplr::comm_world().allreduce([](auto a, auto b) { return a + b; }, sum);
    }
    result.factor = gsl::narrow_cast<double>(sum[0] / n);
    result.error = gsl::narrow_cast<double>(std::sqrt(sum[1]) / n);
    result.nEff = muc::pow<2>(result.factor / result.error);

    return result;
}

template<int N>
auto MetropolisHastingsGenerator<N>::Mass(const std::array<double, N>& mass) -> void {
    fGENBOD.Mass(mass);
    BurnInRequired();
}

template<int N>
auto MetropolisHastingsGenerator<N>::CheckWeightNormalizationFactor(WeightNormalizationFactor wnf) -> bool {
    const auto [result, error, nEff]{wnf};
    const auto ok{nEff >= 10000};
    MasterPrintLn("Weight normalization factor of user-defined bias:\n"
                  "  {} +/- {}\n"
                  "    rel. err. = {:.2}% ,  N_eff = {:.2f} {}\n",
                  result, error, error / result * 100, nEff, ok ? "(OK)" : "(**INACCURATE**)");
    if (not ok) {
        MasterPrintWarning("N_eff TOO LOW. "
                           "This generally means there are a few highly weighted events "
                           "and THEY CAN BIAS THE ESTIMATIONS. "
                           "The estimation should be considered inaccurate.");
    }
    return ok;
}

} // namespace Mustard::inline Physics::inline Generator
