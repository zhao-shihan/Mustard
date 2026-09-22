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

#pragma once

#include "Mustard/CLI/CLI.h++"
#include "Mustard/CLI/Module/ModuleBase.h++"

#include "muc/random"

#include "gsl/gsl"

#include <cstdint>

namespace Mustard::CLI::inline Module {

/// @brief CLI module for Monte Carlo programs.
///
/// MonteCarloModule registers the --seed flag and provides
/// SeedRandomIfFlagged() to apply the user-specified random seed.
/// After seeding, the caller must propagate the seed to all parallel
/// workers via Parallel::ReseedRandomEngine(engine).
class MonteCarloModule : public ModuleBase {
public:
    /// @brief Construct Monte Carlo CLI module.
    /// @param cli CLI instance to register commands with
    MonteCarloModule(gsl::not_null<CLI<>*> cli);

    /// @brief Apply seed configuration if --seed flag was provided.
    /// @tparam E A random number engine satisfying muc::random_number_engine.
    /// @param engine Engine to seed.
    /// @return true if seeding was performed, false if no --seed flag provided
    /// @note The engine is left at its default state if this returns false.
    /// @warning If this returns true, one MUST call
    ///          Parallel::ReseedRandomEngine(engine) to ensure proper seeding
    ///          in parallel environments.
    template<muc::random_number_engine E>
    auto SeedRandomIfFlagged(E& engine) const -> bool;

private:
    /// @brief Draw a non-deterministic seed from the system random device.
    /// @return A random seed, used when the --seed option is given as 0.
    static auto NonDeterministicSeed() -> std::uint32_t;
};

} // namespace Mustard::CLI::inline Module

#include "Mustard/CLI/Module/MonteCarloModule.inl"
