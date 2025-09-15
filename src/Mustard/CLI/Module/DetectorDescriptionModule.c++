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

#include "Mustard/CLI/Module/DetectorDescriptionModule.h++"

namespace Mustard::CLI::inline Module {

DetectorDescriptionModule<>::DetectorDescriptionModule(gsl::not_null<CLI<>*> cli) :
    ModuleBase{cli} {
    auto& detectorDescriptionCLI{TheCLI()->add_mutually_exclusive_group()};
    detectorDescriptionCLI
        .add_argument("--import-dd")
        .nargs(1)
        .help("Import detector description from file.");
    detectorDescriptionCLI
        .add_argument("--export-dd")
        .nargs(1)
        .help("Export default detector description to file.");
    detectorDescriptionCLI
        .add_argument("--emport-dd")
        .nargs(1)
        .help("Export, import, then export detector description.");
}

} // namespace Mustard::CLI::inline Module
