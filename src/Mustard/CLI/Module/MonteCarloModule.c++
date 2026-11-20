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

#include "Mustard/CLI/CLI.h++"
#include "Mustard/CLI/Module/MonteCarloModule.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Math/Random/Generator/MT1993732.h++"
#include "Mustard/Math/Random/Generator/MT1993764.h++"
#include "Mustard/Math/Random/Generator/RandomDevice.h++"
#include "Mustard/Math/Random/Generator/SplitMix64.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256Plus.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256PlusPlus.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256StarStar.h++"
#include "Mustard/Math/Random/Generator/Xoshiro512Plus.h++"
#include "Mustard/Math/Random/Generator/Xoshiro512PlusPlus.h++"
#include "Mustard/Math/Random/Generator/Xoshiro512StarStar.h++"

#include "fmt/format.h"

#include <stdexcept>
#include <string>

namespace Mustard::CLI::inline Module {

MonteCarloModule::MonteCarloModule(gsl::not_null<CLI<>*> cli) :
    ModuleBase{cli} {
    TheCLI()
        ->add_argument("--rng")
        .help("Set random engine type. Default is xoshiro256**.")
        .choices("xoshiro256**", "xoshiro256++", "xoshiro256+",
                 "xoshiro512**", "xoshiro512++", "xoshiro512+",
                 "mt1993764", "mt1993732", "splitmix64");
    TheCLI()
        ->add_argument("--seed")
        .help("Set random seed. 0 means using random device (non deterministic random seed). Predefined deterministic seed is used by default.")
        .scan<'i', std::int64_t>();
}

auto MonteCarloModule::RandomEngineIfFlagged() const -> std::optional<muc::any_rng> {
    const auto randomEngineName{TheCLI()->present<std::string>("--rng")};
    if (not randomEngineName.has_value()) {
        return {};
    }
    using muc::any_rng;
    if (*randomEngineName == "xoshiro256**") {
        return any_rng{Random::Xoshiro256StarStar{}};
    } else if (*randomEngineName == "xoshiro256++") {
        return any_rng{Random::Xoshiro256PlusPlus{}};
    } else if (*randomEngineName == "xoshiro256+") {
        return any_rng{Random::Xoshiro256Plus{}};
    } else if (*randomEngineName == "xoshiro512**") {
        return any_rng{Random::Xoshiro512StarStar{}};
    } else if (*randomEngineName == "xoshiro512++") {
        return any_rng{Random::Xoshiro512PlusPlus{}};
    } else if (*randomEngineName == "xoshiro512+") {
        return any_rng{Random::Xoshiro512Plus{}};
    } else if (*randomEngineName == "mt1993764") {
        return any_rng{Random::MT1993764{}};
    } else if (*randomEngineName == "mt1993732") {
        return any_rng{Random::MT1993732{}};
    } else if (*randomEngineName == "splitmix64") {
        return any_rng{Random::SplitMix64{}};
    }
    Throw<std::invalid_argument>(fmt::format("Unknown random engine type (got {}).", *randomEngineName));
}

auto MonteCarloModule::NonDeterministicSeed() -> std::uint32_t {
    return static_cast<std::uint32_t>(Random::RandomDevice{}());
}

} // namespace Mustard::CLI::inline Module
