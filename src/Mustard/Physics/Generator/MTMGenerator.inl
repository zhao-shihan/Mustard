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
auto MTMGenerator<M, N, A>::operator()(InitialStateMomenta pI, CLHEP::HepRandomEngine& rng) -> Event {
    CheckCMSEnergyUnchanged(pI);
    const auto beta{this->BoostToCMS(pI)};

    this->BurnIn(rng);
    for (int i{}; i < this->fMCMCDiscard; ++i) {
        this->PassEvent(this->fMCMCDelta, rng);
    }
    auto event{this->SampleEvent(this->fMCMCDelta, rng)};

    this->BoostToOriginalFrame(beta, event.p);
    return event;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MTMGenerator<M, N, A>::EstimateWeightNormalizationFactor(unsigned long long n) -> WeightNormalizationFactor {
    auto originalBias{std::move(this->fBias)};
    auto originalBurntIn{std::move(this->fBurntIn)};
    auto originalMarkovChain{std::move(this->fMarkovChain)};
    auto originalEvent{std::move(this->fEvent)};
    auto _{gsl::finally([&] {
        this->fBias = std::move(originalBias);
        this->fBurntIn = std::move(originalBurntIn);
        this->fMarkovChain = std::move(originalMarkovChain);
        this->fEvent = std::move(originalEvent);
    })};

    WeightNormalizationFactor result{.value = std::numeric_limits<double>::quiet_NaN(),
                                     .error = std::numeric_limits<double>::quiet_NaN(),
                                     .nEff = 0};
    if (n == 0) {
        return result;
    }

    this->Bias([](auto&&) { return 1; }); // to calculate the mean of user-defined bias, sample from unbiased |M|²
    auto& rng{*CLHEP::HepRandom::getTheEngine()};
    this->BurnIn(rng);

    using namespace Mustard::VectorArithmeticOperator::Vector2ArithmeticOperator;
    muc::array2d sum{};
    { // Monte Carlo integration here
        muc::array2d compensation{};
        const auto KahanAdd{[&](muc::array2d value) { // improve numeric stability
            const auto correctedValue{value - compensation};
            const auto newSum{sum + correctedValue};
            compensation = (newSum - sum) - correctedValue;
            sum = newSum;
        }};
        Parallel::ReseedRandomEngine(&rng);
        Executor<unsigned long long>{"Estimation", "Sample"}(n, [&](auto) {
            const auto event{this->SampleEvent(rng)};
            const auto bias{originalBias(event.p)};
            KahanAdd({bias, muc::pow(bias, 2)});
        });
    }
    if (mplr::available()) {
        mplr::comm_world().allreduce([](auto a, auto b) { return a + b; }, sum);
    }
    const auto& [sumBias, sumBiasSq]{sum};
    result.value = sumBias / n;
    result.error = std::sqrt(sumBiasSq / n - muc::pow(result.value, 2)) / n;
    result.nEff = muc::pow(result.value / result.error, 2);

    return result;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MTMGenerator<M, N, A>::CheckWeightNormalizationFactor(WeightNormalizationFactor wnf) -> bool {
    const auto [result, error, nEff]{wnf};
    const auto ok{nEff >= 10000};
    MasterPrintLn("Weight normalization factor from user-defined bias:\n"
                  "  {} +/- {}\n"
                  "    rel. err. = {:.2}% ,  N_eff = {:.2f} {}\n",
                  result, error, error / result * 100, nEff, ok ? "(OK)" : "(**INACCURATE**)");
    if (not ok) {
        MasterPrintWarning("N_eff TOO LOW. "
                           "The estimation should be considered inaccurate. "
                           "Try increasing statistics.");
    }
    return ok;
}

template<int M, int N, std::derived_from<SquaredAmplitude<M, N>> A>
auto MTMGenerator<M, N, A>::CheckCMSEnergyUnchanged(const InitialStateMomenta& pI) const -> void {
    if (pI == InitialStateMomenta{}) {
        return;
    }
    const auto cmsE{this->CalculateCMSEnergy(pI)};
    if (not muc::isclose(cmsE, this->fCMSEnergy)) [[unlikely]] {
        PrintWarning(fmt::format("Initial state 4-momenta does not match currently set CMS energy (got {}, expect {})", cmsE, this->fCMSEnergy));
    }
}

} // namespace Mustard::inline Physics::inline Generator
