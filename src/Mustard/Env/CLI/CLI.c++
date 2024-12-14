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

#include "Mustard/Env/CLI/CLI.h++"
#include "Mustard/Utility/PrettyLog.h++"
#include "Mustard/Version.h++"

#include "fmt/core.h"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>

namespace Mustard::Env::CLI {

CLI<>::CLI() :
    NonMoveableBase{},
    fArgcArgv{},
    fArgParser{std::make_unique<argparse::ArgumentParser>("", MUSTARD_VERSION_STRING, argparse::default_arguments::none)} {
    if (static bool gInstantiated{}; gInstantiated) {
        Throw<std::logic_error>("Trying to construct CLI twice");
    } else {
        gInstantiated = true;
    }
}

CLI<>::~CLI() = default;

auto CLI<>::ParseArgs(int argc, char* argv[]) -> void {
    if (not Parsed()) {
        try {
            ArgParser().parse_args(argc, argv);
        } catch (const std::runtime_error& exception) {
            fmt::println(stderr, "{}", exception.what());
            fmt::println(stderr, "Try {} --help", argv[0]);
            std::exit(EXIT_FAILURE);
        }
        fArgcArgv = {argc, argv};
    } else {
        ThrowParsed();
    }
}

auto CLI<>::ArgcArgv() const -> ArgcArgvType {
    if (Parsed()) {
        return fArgcArgv.value();
    } else {
        ThrowNotParsed();
    }
}

[[noreturn]] auto CLI<>::ThrowParsed() -> void {
    Throw<std::logic_error>("Command line arguments has been parsed");
}

[[noreturn]] auto CLI<>::ThrowNotParsed() -> void {
    Throw<std::logic_error>("Command line arguments has not been parsed");
}

} // namespace Mustard::Env::CLI
