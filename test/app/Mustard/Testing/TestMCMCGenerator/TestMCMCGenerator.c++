// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
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

#include "Mustard/CLI/MonteCarloCLI.h++"
#include "Mustard/Env/MPIMonteCarloEnv.h++"
#include "Mustard/Execution/Executor.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/IO/Print.h++"
#include "Mustard/Physics/Generator/M2ENNEEGenerator.h++"
#include "Mustard/Testing/TestMCMCGenerator/TestMCMCGenerator.h++"

#include "fmt/format.h"

#include <cmath>
#include <cstdlib>
#include <stdexcept>

namespace Mustard::Testing {

TestMCMCGenerator::TestMCMCGenerator() :
    Subprogram{"TestMCMCGenerator", "Test MCMCGenerator initialization and event generation."} {}

auto TestMCMCGenerator::Main(int argc, char* argv[]) const -> int {
    CLI::MonteCarloCLI<> cli;
    Env::MPIMonteCarloEnv env{argc, argv, cli};

    // Keep the default ACF sample size and configure three ACF replicates;
    // initialization raises this to the process count if needed.
    Executor<long long> executor{"MCMC generator test", "Initialization", "replicate"};
    Physics::Generator::M2ENNEEGenerator generator{
        "mu-", {0, 0, 0},
         {0, 0, 0},
         {},
         3
    };
    const auto statistic{generator.MCMCInitialize(executor)};
    if (not generator.MCMCInitialized()) {
        Throw<std::runtime_error>("MCMC generator was not initialized.");
    }
    if (not std::isfinite(statistic.ScalarIntegratedAutocorrelation()) or statistic.ScalarIntegratedAutocorrelation() <= 0) {
        Throw<std::runtime_error>("Initialization returned an invalid scalar integrated autocorrelation time.");
    }
    if (statistic.ScalarIntegratedAutocorrelationStatistic().SampleSize() < 3) {
        Throw<std::runtime_error>("Initialization used fewer than the requested number of ACF replicates.");
    }
    for (int i{}; i < statistic.Dimension(); ++i) {
        if (not std::isfinite(statistic.IntegratedAutocorrelation(i)) or statistic.IntegratedAutocorrelation(i) <= 0) {
            Throw<std::runtime_error>(fmt::format("Initialization returned an invalid IACT for dimension {}.", i));
        }
        if (not std::isfinite(statistic.EffectiveSampleSize(i)) or statistic.EffectiveSampleSize(i) <= 0) {
            Throw<std::runtime_error>(fmt::format("Initialization returned an invalid ESS for dimension {}.", i));
        }
    }

    constexpr auto nEvent{10'000};
    for (int i{}; i < nEvent; ++i) {
        const auto event{generator()};
        if (not std::isfinite(event.weight) or event.weight <= 0) {
            Throw<std::runtime_error>(fmt::format("Generated event {} has an invalid weight ({}).", i, event.weight));
        }
    }
    PrintLn("Generated {} events after MCMC initialization ({} ACF replicates).",
            nEvent, statistic.ScalarIntegratedAutocorrelationStatistic().SampleSize());
    PrintLn("All TestMCMCGenerator tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
