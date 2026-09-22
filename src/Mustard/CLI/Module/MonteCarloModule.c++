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
#include "Mustard/Math/Random/Generator/RandomDevice.h++"

namespace Mustard::CLI::inline Module {

MonteCarloModule::MonteCarloModule(gsl::not_null<CLI<>*> cli) :
    ModuleBase{cli} {
    TheCLI()
        ->add_argument("--seed")
        .help("Set random seed. 0 means using random device (non deterministic random seed). Predefined deterministic seed is used by default.")
        .scan<'i', std::int64_t>();
}

auto MonteCarloModule::NonDeterministicSeed() -> std::uint32_t {
    return static_cast<std::uint32_t>(Random::RandomDevice{}());
}

} // namespace Mustard::CLI::inline Module
