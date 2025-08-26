// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

#include "Mustard/CLI/internal/MakeDefaultArgParser.h++"
#include "Mustard/Version.h++"

#include "fmt/core.h"

#include <cstdlib>

namespace Mustard::CLI::internal {

auto MakeDefaultArgParser() -> std::unique_ptr<argparse::ArgumentParser> {
    auto argParser{std::make_unique<argparse::ArgumentParser>("", MUSTARD_VERSION_STRING, argparse::default_arguments::none)};
    argParser
        ->add_argument("--help")
        .help("Show this help and exit.")
        .nargs(0)
        .action([&parser = *argParser](auto&&) {
            fmt::print("{}", parser.help().view());
            std::exit(EXIT_SUCCESS);
        });
    argParser
        ->add_argument("--mustard-version")
        .help("Show Mustard version information and exit.")
        .nargs(0)
        .action([](auto&&) {
            fmt::print("Mustard v" MUSTARD_VERSION_STRING "\n"
                       "Copyright (C) 2020-2025  The Mustard development team\n"
                       "This program is free software: you can redistribute it and/or modify it under"
                       "the terms of the GNU General Public License as published by the Free Software"
                       "Foundation, either version 3 of the License, or (at your option) any later"
                       "version.\n"
                       "This program is distributed in the hope that it will be useful, but WITHOUT"
                       "ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS"
                       "FOR A PARTICULAR PURPOSE. See the GNU General Public License for more"
                       "details.\n"
                       "You should have received a copy of the GNU General Public License along with"
                       "this program. If not, see <https://www.gnu.org/licenses/>.\n");
            std::exit(EXIT_SUCCESS);
        });
    argParser
        ->add_argument("--mustard-version-short")
        .help("Print Mustard version and exit.")
        .nargs(0)
        .action([](auto&&) {
            fmt::println(MUSTARD_VERSION_STRING);
            std::exit(EXIT_SUCCESS);
        });
    return argParser;
}

} // namespace Mustard::CLI::internal
