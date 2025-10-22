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

#include "Mustard/CLI/Module/Geant4Module.h++"

namespace Mustard::CLI::inline Module {

Geant4Module::Geant4Module(gsl::not_null<CLI<>*> cli) :
    ModuleBase{cli} {
    TheCLI()
        ->add_argument("macro")
        .help("Run the program in batch session with it. If not provided, run in interactive session with default initialization.")
        .nargs(argparse::nargs_pattern::optional);
    TheCLI()
        ->add_argument("-i", "--interactive")
        .flag()
        .help("Run in interactive session despite of a provided macro. The macro will initialize the session.");
}

} // namespace Mustard::CLI::inline Module
