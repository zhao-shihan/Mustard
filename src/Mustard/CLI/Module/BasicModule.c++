// -*- C++ -*-
//
// Copyright 2020-2025  The Mustard development team
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

#include "Mustard/CLI/Module/BasicModule.h++"
#include "Mustard/Version.h++"

#include "muc/utility"

#include <algorithm>

namespace Mustard::CLI::inline Module {

BasicModule::BasicModule(argparse::ArgumentParser& argParser) :
    ModuleBase{argParser},
    fVerboseLevelValue{muc::to_underlying(Env::VerboseLevel::Warning)} {
    ArgParser()
        .add_argument("-v", "--verbose")
        .help("Increase verbose level (-2: quiet, -1: error, 0: warning (default), 1: informative, 2: verbose). "
              "This is repeatable (e.g. -v -v or -vv) and can be combined with -q or --quiet (e.g. -vvq (=1) -qv (=0) -qqvqv (=-1)).")
        .flag()
        .append()
        .nargs(0)
        .action([this](auto&&) { ++fVerboseLevelValue; });
    ArgParser()
        .add_argument("-q", "--quiet")
        .help("Decrease verbose level (see previous).")
        .flag()
        .append()
        .nargs(0)
        .action([this](auto&&) { --fVerboseLevelValue; });
    ArgParser()
        .add_argument("--lite")
        .flag()
        .help("Do not show the Mustard banner.");
}

auto BasicModule::VerboseLevel() const -> std::optional<Env::VerboseLevel> {
    if (ArgParser().is_used("-v") or ArgParser().is_used("-q")) {
        return static_cast<Env::VerboseLevel>(std::clamp(fVerboseLevelValue,
                                                         muc::to_underlying(Env::VerboseLevel::Quiet),
                                                         muc::to_underlying(Env::VerboseLevel::Verbose)));
    } else {
        return std::nullopt;
    }
}

} // namespace Mustard::CLI::inline Module
