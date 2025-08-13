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

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
MetropolisHastingsGenerator<M, N, A>::MetropolisHastingsGenerator(double cmsE, const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                                                  double delta, int discard) :
    EventGenerator<M, N>{},
    fSquaredAmplitude{},
    fCMSEnergy{cmsE},
    fGENBOD{pdgID, mass},
    fBias{[](auto&&) { return 1; }},
    fMCMCDelta{delta},
    fMCMCDiscard{discard},
    fBurntIn{},
    fMarkovChain{},
    fEvent{} {}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MetropolisHastingsGenerator<M, N, A>::MCMCDelta(double delta) -> void {
    // The step size will increase as the roughly the square root of the number of dimensions
    // so we reduce delta according to state space dimension
    fMCMCDelta = delta / std::sqrt(fgMCStateSpaceDimension);
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MetropolisHastingsGenerator<M, N, A>::CMSEnergy(double cmsE) -> void {
    if (not muc::isclose(cmsE, fCMSEnergy)) {
        BurnInRequired();
    }
    fCMSEnergy = cmsE;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MetropolisHastingsGenerator<M, N, A>::Bias(BiasFunction B) -> void {
    fBias = std::move(B);
    BurnInRequired();
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MetropolisHastingsGenerator<M, N, A>::BurnIn(CLHEP::HepRandomEngine& rng) -> void {
    if (fBurntIn) {
        return;
    }
    // find phase space
    while (true) {
        std::ranges::generate(fMarkovChain.state, [&rng] { return rng.flat(); });
        fEvent = fGENBOD({fCMSEnergy, {}}, fMarkovChain.state);
        if (const auto bias{ValidBias(fEvent.p)};
            bias >= std::numeric_limits<double>::min()) {
            fMarkovChain.acceptance = ValidBiasedPDF(fEvent, bias);
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

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MetropolisHastingsGenerator<M, N, A>::operator()(InitialStateMomenta pI, CLHEP::HepRandomEngine& rng) -> Event {
    CheckCMSEnergyUnchanged(pI);
    const auto beta{this->BoostToCMS(pI)};

    BurnIn(rng);
    for (int i{}; i < fMCMCDiscard; ++i) {
        NextEvent(fMCMCDelta, rng);
    }
    NextEvent(fMCMCDelta, rng);

    this->BoostToOriginalFrame(beta, fEvent.p);
    return fEvent;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MetropolisHastingsGenerator<M, N, A>::CheckCMSEnergyUnchanged(const InitialStateMomenta& pI) const -> void {
    if (pI == InitialStateMomenta{}) {
        return;
    }
    const auto cmsE{this->CalculateCMSEnergy(pI)};
    if (not muc::isclose(cmsE, fCMSEnergy)) {
        PrintWarning(fmt::format("Initial state 4-momenta does not match currently set CMS energy (got {}, expect {})", cmsE, fCMSEnergy));
    }
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MetropolisHastingsGenerator<M, N, A>::NextEvent(double delta, CLHEP::HepRandomEngine& rng) -> void {
    typename GENBOD<M, N>::RandomState proposedRandomState;
    Event proposedEvent;
    while (true) {
        std::ranges::transform(std::as_const(fMarkovChain.state), proposedRandomState.begin(),
                               [&](auto u) {
                                   const auto low{std::clamp(u - delta, 0., 1.)};
                                   const auto up{std::clamp(u + delta, 0., 1.)};
                                   return CLHEP::RandFlat::shoot(&rng, low, up);
                               });
        proposedEvent = fGENBOD({fCMSEnergy, {}}, proposedRandomState);
        const auto proposedBias{ValidBias(proposedEvent.p)};
        if (proposedBias <= std::numeric_limits<double>::min()) {
            continue;
        }

        const auto proposedAcceptance{ValidBiasedPDF(proposedEvent, proposedBias)};
        if (proposedAcceptance >= fMarkovChain.acceptance or
            proposedAcceptance >= fMarkovChain.acceptance * rng.flat()) {
            fMarkovChain.state = proposedRandomState;
            fEvent = proposedEvent;
            fEvent.weight = 1 / proposedBias;
            fMarkovChain.acceptance = proposedAcceptance;
            return;
        }
    }
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MetropolisHastingsGenerator<M, N, A>::ValidBias(const FinalStateMomenta& momenta) const -> double {
    const auto bias{fBias(momenta)};
    constexpr auto Format{[](const FinalStateMomenta& momenta) {
        std::string where;
        for (auto&& p : momenta) {
            where += fmt::format("[{}, {}, {}; {}]", p.x(), p.y(), p.z(), p.e());
        }
        return where;
    }};
    if (not std::isfinite(bias)) {
        Throw<std::runtime_error>(fmt::format("Infinite bias found (got {} at {})", bias, Format(momenta)));
    }
    if (bias < 0) {
        Throw<std::runtime_error>(fmt::format("Negative bias found (got {} at {})", bias, Format(momenta)));
    }
    return bias;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MetropolisHastingsGenerator<M, N, A>::ValidBiasedPDF(const Event& event, double bias) const -> double {
    const auto value{event.weight * fSquaredAmplitude({fCMSEnergy, {}}, event.p) * bias};
    const auto Where{[&] {
        auto where{fmt::format("({})", event.weight)};
        for (auto&& p : event.p) {
            where += fmt::format("[{}, {}, {}; {}]", p.x(), p.y(), p.z(), p.e());
        }
        where += fmt::format(" Bias={}", bias);
        return where;
    }};
    if (not std::isfinite(value)) {
        Throw<std::runtime_error>(fmt::format("Infinite biased PDF found (got {} at {})", value, Where()));
    }
    if (value < 0) {
        Throw<std::runtime_error>(fmt::format("Negative biased PDF found (got {} at {})", value, Where()));
    }
    return value;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MetropolisHastingsGenerator<M, N, A>::EstimateWeightNormalizationFactor(unsigned long long n) -> WeightNormalizationFactor {
    auto originalBias{std::move(fBias)};
    auto originalBurntIn{std::move(fBurntIn)};
    auto originalMarkovChain{std::move(fMarkovChain)};
    auto originalEvent{std::move(fEvent)};
    auto _{gsl::finally([&] {
        fBias = std::move(originalBias);
        fBurntIn = std::move(originalBurntIn);
        fMarkovChain = std::move(originalMarkovChain);
        fEvent = std::move(originalEvent);
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

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MetropolisHastingsGenerator<M, N, A>::CheckWeightNormalizationFactor(WeightNormalizationFactor wnf) -> bool {
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
                           "The estimation should be considered inaccurate. "
                           "Try increasing statistics.");
    }
    return ok;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MetropolisHastingsGenerator<M, N, A>::Mass(const std::array<double, N>& mass) -> void {
    if (not std::ranges::equal(mass, fGENBOD.Mass(),
                               [](auto a, auto b) { return muc::isclose(a, b); })) {
        BurnInRequired();
    }
    fGENBOD.Mass(mass);
}

} // namespace Mustard::inline Physics::inline Generator
