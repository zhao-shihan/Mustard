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

#include "Mustard/Env/CLI/Module/Geant4Module.h++"

namespace Mustard::Env::CLI::inline Module {

Geant4Module::Geant4Module(argparse::ArgumentParser& argParser) :
    ModuleBase{argParser} {
    ArgParser()
        .add_argument("macro")
        .help("Run the program in batch session with it. If not provided, run in interactive session with default initialization.")
        .nargs(argparse::nargs_pattern::optional);
    ArgParser()
        .add_argument("-i", "--interactive")
        .flag()
        .help("Run in interactive session despite of a provided macro. The macro will initialize the session.");
}

} // namespace Mustard::Env::CLI::inline Module
