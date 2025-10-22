// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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
#include "Mustard/CLI/Module/BasicModule.h++"
#include "Mustard/Version.h++"

#include "muc/utility"

#include <algorithm>

namespace Mustard::CLI::inline Module {

BasicModule::BasicModule(gsl::not_null<CLI<>*> cli) :
    ModuleBase{cli},
    fVerboseLevelValue{muc::to_underlying(Env::VerboseLevel::Warning)} {
    TheCLI()
        ->add_argument("-v", "--verbose")
        .help("Increase verbose level (-2: quiet, -1: error, 0: warning (default), 1: informative, 2: verbose), e.g. -v (1), -v -v (2), -vv (2).")
        .flag()
        .append()
        .nargs(0)
        .action([this](auto&&) { ++fVerboseLevelValue; });
    TheCLI()
        ->add_argument("-q", "--quiet")
        .help("Decrease verbose level (see previous), e.g. -q, -q -q, -qq. Can be used together with -v, e.g. -q -v, -vvqvq.")
        .flag()
        .append()
        .nargs(0)
        .action([this](auto&&) { --fVerboseLevelValue; });
    TheCLI()
        ->add_argument("--lite")
        .flag()
        .help("Do not show the Mustard banner.");
}

auto BasicModule::VerboseLevel() const -> std::optional<Env::VerboseLevel> {
    if (TheCLI()->is_used("-v") or TheCLI()->is_used("-q")) {
        return static_cast<Env::VerboseLevel>(std::clamp(fVerboseLevelValue,
                                                         muc::to_underlying(Env::VerboseLevel::Quiet),
                                                         muc::to_underlying(Env::VerboseLevel::Verbose)));
    } else {
        return std::nullopt;
    }
}

auto BasicModule::ShowBanner() const -> bool {
    return not TheCLI()->is_used("--lite");
}

} // namespace Mustard::CLI::inline Module
